/*
 * Implementation of the word_count interface using Pintos lists.
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
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t* wclist) { /* TODO */
  list_init (wclist);
}

size_t len_words(word_count_list_t* wclist) {
  /* TODO */
  // struct list_elem *e;
  // size_t count = 0;
  // for (e = list_begin(wclist); e != list_end(wclist); e = list_next(e)) {
  //   count++;
  // }
  // return count;
  return list_size(wclist);
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
  /* TODO */
  struct list_elem *e;
  word_count_list_t* iter = wclist;
  for (e = list_begin(iter); e != list_end(iter); e = list_next(e)) {
    word_count_t *item = list_entry (e, word_count_t, elem);
    if (strcmp(item->word, word) == 0) {
      return item;
    }
  }
  return NULL;
}

word_count_t* add_word(word_count_list_t* wclist, char* word) {
  /* TODO */
  word_count_t* wc;
  if (wc = find_word(wclist, word)) {
    wc->count++;
    return wc;
  }
  wc = (word_count_t*)malloc(sizeof(word_count_t));
  wc->count = 1;
  wc->word = strcpy((char *)malloc(strlen(word) + 1), word);
  list_push_front(wclist, &wc->elem);
  
  return wc;
}

void fprint_words(word_count_list_t* wclist, FILE* outfile) { /* TODO */
  struct list_elem *e;
  for (e = list_begin(wclist); e != list_end(wclist); e = list_next(e)) {
    word_count_t *item = list_entry (e, word_count_t, elem);
    fprintf(outfile, "\t%i\t%s\n", item->count, item->word);
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
  list_sort(wclist, list_less, (void *)less);
}
