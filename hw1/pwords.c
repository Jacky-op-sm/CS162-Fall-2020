/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

typedef struct thread_args {
  FILE* file;
  word_count_list_t* word_counts;
} thread_args_t;

void* count_words_one_arg(void *arg) {
  thread_args_t *args = arg;
  count_words(args->word_counts, args->file);
  pthread_exit(0);
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    /* Test for the single file: */
    // FILE* file = fopen(argv[1], "r");
    // count_words(&word_counts, file);
    // fclose(file);
    
    /* TODO */
    int num_file = argc - 1;
    pthread_t threads[num_file];
    FILE* files[num_file];
    /*  it's really important here to make an array of agrs_t:
        otherwise, if each loop creates an arg and pass it to thread routine[line 78]
        the program won't work well, I guess the reason is that each loop will replace
        the old arg and corrupt the program. 
    */
    thread_args_t args_t[num_file];
    for (int i = 0; i < num_file; i++) {
      char* file_path = argv[i + 1];
      files[i] = fopen(file_path, "r");
      thread_args_t arg;
      arg.file = files[i];
      arg.word_counts = &word_counts;
      args_t[i] = arg;
      pthread_create(&threads[i], NULL, &count_words_one_arg, &args_t[i]);
    } 

    for (int i = 0; i < num_file; i++) {
      pthread_join(threads[i], NULL);
      fclose(files[i]);
    }  
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
