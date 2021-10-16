#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(unused struct tokens* tokens);
int cmd_cd(struct tokens* tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print the current working directory"},
    {cmd_cd, "cd", "change the current working directory to given directory"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) { exit(0); }

/* Prints the current working directory */
int cmd_pwd(unused struct tokens* tokens) {
  static char cwd[4096];
  getcwd(cwd, 4096);
  fprintf(stdout, "%s\n", cwd);
  return 1;
}

/* Changes the current working directory to given directory*/
int cmd_cd(struct tokens* tokens) {
  size_t words = tokens_get_length(tokens);
  if (words > 2) {
    fprintf(stdout, "cd: too many arguments\n");
    return 0;
  }
  chdir(tokens_get_token(tokens, 1));
  return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char* argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* wait status for child */
  int wstatus;

  /* Set up path for Path Resolution */
  char* PATH = "/usr/local/sbin/ /usr/local/bin/ /usr/sbin/ /usr/bin/ /sbin/ /bin/ /usr/games/ /usr/local/games/ /snap/bin/ /usr/local/go/bin/ /home/vagrant/.bin/ /home/vagrant/.fzf/bin/";
  struct tokens* path_tks = tokenize(PATH);

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens* tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      pid_t child_pid = fork();

      // Child process: execv the program
      if (child_pid == 0) {
        /* Set up funcs and argvs for execv */
        char* cmd_fn = tokens_get_token(tokens, 0);
        size_t tokens_nums = tokens_get_length(tokens);
        /* Huge mistake here!!!: debug me for 30 min but just find out a
         * simple mistake: malloc -> calloc, the latter one gives the default NULL
         * value  */
        char** cmd_argv = (char **) calloc(tokens_nums, sizeof(char *));

        /* Redirection index */
        int input_red_ind = -1;
        int output_red_ind = -1;
        /* Redirection file descriptor */
        int in_rediction = -1;
        int out_rediction = -1;

        int arg_index = 0;
        // Copy the argvs in tokens into cmd_argv
        for (int i = 0; i < tokens_nums; i++) {
          char* item = tokens_get_token(tokens, i);
          /* Check redirection */
          if (strcmp(item, "<") == 0) {
            input_red_ind = i;
          } else if (strcmp(item, ">") == 0) {
            output_red_ind = i;
          }
           else if (input_red_ind != -1 && i == (input_red_ind + 1)) {
            in_rediction = open(item, O_RDONLY);
            dup2(in_rediction, 0);
            close(in_rediction);
          } else if (output_red_ind != -1 && i == (output_red_ind + 1)) {
            /* Create the file desc fisrt if not exist and then open. 
             * Reference: https://stackoverflow.com/questions/9874002/how-to-create-a-file-only-if-it-doesnt-exist
             */

            //out_rediction = open(item, O_WRONLY);
            out_rediction = open(item, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            dup2(out_rediction, 1);
            close(out_rediction);
          } 
            else {
            cmd_argv[arg_index++] = item;
          }
        }

        // Try the full pathname
        if (execv(cmd_fn, cmd_argv) != -1) {
          /* Clean up memory */
          tokens_destroy(tokens);
          tokens_destroy(path_tks);
          free(cmd_argv);
          exit(0);
        }
        
        // Try the default path with prefix 
        for (int i = 0; i < tokens_get_length(path_tks); i++) {
          char* path = tokens_get_token(path_tks, i);

          // append cmd_fn to each path in $PATH 
          strcat(path, cmd_fn);

          // update cmd_argv and then execv
          cmd_argv[0] = path;
          if (execv(path, cmd_argv) != -1) {
            /* Clean up memory */
            tokens_destroy(tokens);
            tokens_destroy(path_tks);
            free(cmd_argv);
            exit(0);
          }
        }
        fprintf(stdout, "%s: No such file or directory\n", cmd_fn);
      } else {
        // Questionmark: busy waiting?
        while ((waitpid(-1, &wstatus, 0)) > 0);
        }
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  tokens_destroy(path_tks);
  return 0;
}
