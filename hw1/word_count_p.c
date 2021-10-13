/*
 * Implementation of the word_count interface using Pintos lists and //pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t* wclist) { /* TODO */
  list_init(&wclist->lst);
  pthread_mutex_init(&wclist->lock, NULL);
}

size_t len_words(word_count_list_t* wclist) {
  size_t len;
  //pthread_mutex_lock(&wclist->lock);
  len = list_size(&wclist->lst);
  //pthread_mutex_unlock(&wclist->lock);
  return len;
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
  struct list_elem *e;
  word_count_t *target = NULL;
  //pthread_mutex_lock(&wclist->lock);
  for (e = list_begin(&wclist->lst); e != list_end(&wclist->lst); e = list_next(e)) {
    word_count_t *item = list_entry (e, word_count_t, elem);
    if (strcmp(item->word, word) == 0) {
      target = item;
      break;
    }
  }
  //pthread_mutex_unlock(&wclist->lock);
  return target;
}

word_count_t* add_word(word_count_list_t* wclist, char* word) {
  word_count_t* wc;
  if (wc = find_word(wclist, word)) {
    /*  take notice of the position of lock statement:
        if lock before find_word: there will be double lock instuction,
        which cause the program to halt.

        second remark: find_word maybe needn't be synchronized.
    */
    pthread_mutex_lock(&wclist->lock);
    wc->count++;
    pthread_mutex_unlock(&wclist->lock);
    return wc;
  }
  pthread_mutex_lock(&wclist->lock);
  wc = (word_count_t*)malloc(sizeof(word_count_t));
  wc->count = 1;
  wc->word = strcpy(malloc(strlen(word) + 1), word);
  list_push_front(&wclist->lst, &wc->elem);
  pthread_mutex_unlock(&wclist->lock);
  
  return wc;
}

void fprint_words(word_count_list_t* wclist, FILE* outfile) { /* TODO */
  struct list_elem *e;
  for (e = list_begin(&wclist->lst); e != list_end(&wclist->lst); e = list_next(e)) {
    word_count_t *item = list_entry (e, word_count_t, elem);
    fprintf(outfile, "%8i\t%s\n", item->count, item->word);
  }
}

bool list_less(const struct list_elem* a, const struct list_elem* b, void* aux) {
  bool (*less)(const word_count_t*, const word_count_t*) = aux;
  word_count_t *item1 = list_entry (a, word_count_t, elem);
  word_count_t *item2 = list_entry (b, word_count_t, elem);
  return less(item1, item2);
}

void wordcount_sort(word_count_list_t* wclist,
                    bool less(const word_count_t*, const word_count_t*)) {
  /* TODO */
  list_sort(&wclist->lst, list_less, (void *)less);
}
