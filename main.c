#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 50
#define MAX_WORDS_COUNT 10000

typedef struct {
  char suffix[MAX_WORD_LENGTH];
  size_t left_b;
  size_t right_b;
  bool searching_suffix;
} state;

unsigned long hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

bool hash_map[1000000000];
void hash_map_set(char *str) { hash_map[hash(str) & 0xFFFFFFF] = true; }
bool hash_map_get(char *str) { return hash_map[hash(str) & 0xFFFFFFF]; }

#define STACK_COPACITY 10000

typedef struct {
  state items[STACK_COPACITY];
  size_t count;
} stack;

void stack_push(stack *st, state s);
state stack_pop(stack *st);

stack *create_stack(void) {
  stack *st = malloc(sizeof(stack));
  st->count = 0;
  return st;
}

void dump_stack(stack *st) {
  printf("Stack dump: \n");
  printf("count: %ld\n", st->count);
  for (size_t i = 0; i < st->count; i++) {
    printf("\t [%ld] suff: %s\n", i, st->items[i].suffix);
  }
}

void stack_push(stack *st, state s) {
  if (st->count >= STACK_COPACITY - 1) {
    fprintf(stderr, "Stack overflow\n");
    exit(1);
  }
  st->items[++st->count] = s;
}

state stack_pop(stack *st) {
  if (st->count <= 0) {
    fprintf(stderr, "Stack underflow\n");
    exit(1);
  }
  return st->items[--st->count];
}

char *reverse_str(char *s) {
  char t;
  size_t l, r, len;
  len = strlen(s);
  if (len == 0)
    return s;
  for (l = 0, r = len - 1; l < r; l++, r--) {
    t = s[l];
    s[l] = s[r];
    s[r] = t;
  }
  return s;
}

#define END_OF_UPPERCASE_WORDS 75067

bool search_suffix(char *suffix, FILE *f, char *result) {
  // printf("Searching suffix: %s\n", suffix);
  char word[MAX_WORD_LENGTH];
  fseek(f, END_OF_UPPERCASE_WORDS, SEEK_SET);
  size_t wl, sl = strlen(suffix);
  int w, s;
next_word:
  while (fgets(word, MAX_WORD_LENGTH, f) != NULL) {
    wl = strlen(word);
    if (wl < 2 || wl < sl)
      goto next_word;
    word[wl - 1] = '\0'; // get rid of \n
    if (sl == 0) {
      memcpy(result, word, wl);
      hash_map_set(word);
      return true;
    }
    for (w = wl - 2, s = sl - 1; w >= 0 && s >= 0; w--, s--) {
      if (word[w] != suffix[s]) {
        goto next_word;
      }
    }
    if (hash_map_get(word))
      goto next_word;
    hash_map_set(word);

    memcpy(result, word, wl);
    return true;
  }
  return false;
}

bool search_preffix(char *preffix, FILE *f, char *result) {
  size_t wl, pl;
  pl = strlen(preffix);
  char word[MAX_WORD_LENGTH];
  fseek(f, END_OF_UPPERCASE_WORDS, SEEK_SET);
next_word:
  while (fgets(word, MAX_WORD_LENGTH, f) != NULL) {
    wl = strlen(word);
    if (wl < 2 || wl < pl)
      goto next_word;
    word[wl - 1] = '\0';
    if (pl == 0) {
      memcpy(result, word, wl);
      hash_map_set(word);
      return true;
    }
    if (strncmp(preffix, word, pl) == 0) {

      if (hash_map_get(word))
        goto next_word;
      hash_map_set(word);

      memcpy(result, word, wl);
      return true;
    }
  }
  return false;
}

void insert_str_at(char *dest, char *str, size_t pos) {
  size_t sl = strlen(str);
  memmove(dest + pos + sl, dest + pos, strlen(dest + pos - 1));
  memcpy(dest + pos, str, sl);
}

bool is_string_polindrome(char *s) {
  if (strlen(s) == 0)
    return true;
  size_t left_index = 0, right_index = strlen(s) - 1;
  while (left_index < right_index) {
    if (isalpha(s[left_index]) && isalpha(s[right_index])) {
      if (tolower(s[left_index]) != tolower(s[right_index])) {
        fflush(stdout);
        return false;
      } else {
        left_index++, right_index--;
        continue;
      }
    }
    if (!isalpha(s[left_index])) {
      left_index++;
      continue;
    }
    if (!isalpha(s[right_index])) {
      right_index--;
      continue;
    }
    left_index++, right_index--;
  }
  return true;
}

char *connect_words(char words[MAX_WORDS_COUNT][MAX_WORD_LENGTH], size_t left,
                    size_t right, char *result) {
  size_t l = 0, r = right + 1;
  result[0] = '\0';
  while (l < left) {
    strcat(result, words[l++]);
  }
  while (r < MAX_WORDS_COUNT) {
    strcat(result, words[r++]);
  }
  return result;
}

int main(void) {
  FILE *nouns = fopen("nouns.txt", "r");

  char words[MAX_WORDS_COUNT][MAX_WORD_LENGTH];
  size_t left_b = 0, right_b = MAX_WORDS_COUNT - 1;
  strcpy(words[left_b++], "A man, ");
  strcpy(words[right_b--], "Panama");
  strcpy(words[left_b++], "a plan, ");
  strcpy(words[right_b--], "a canal, ");

  char tmp[MAX_WORD_LENGTH + 5]; // for "a word, "
  char result[MAX_WORD_LENGTH];

  state current_state;
  strcpy(current_state.suffix, "ca");
  current_state.searching_suffix = false;
  current_state.left_b = left_b;
  current_state.right_b = right_b;

  stack *stack = create_stack();
  stack_push(stack, current_state);

  size_t len, suff_len;
  char res[MAX_WORDS_COUNT * MAX_WORD_LENGTH];

  while (current_state.left_b < 200 ||
         !is_string_polindrome(connect_words(words, current_state.left_b,
                                             current_state.right_b, res))) {
    if (current_state.searching_suffix) {
      if (!search_suffix(current_state.suffix, nouns, result)) {
        current_state = stack_pop(stack);
        continue;
      }
    } else {
      if (!search_preffix(current_state.suffix, nouns, result)) {
        current_state = stack_pop(stack);
        continue;
      }
    }

    len = strlen(result);
    suff_len = strlen(current_state.suffix);

    if (current_state.searching_suffix) {
      strncpy(current_state.suffix, result, len - suff_len);
      current_state.suffix[len - suff_len] = '\0';
      reverse_str(current_state.suffix);
      len = strlen(current_state.suffix);
      current_state.suffix[len] = 'a';
      current_state.suffix[len + 1] = '\0';
    } else {
      strcpy(current_state.suffix, result + suff_len);
      len = strlen(current_state.suffix);
      reverse_str(current_state.suffix);
      memmove(current_state.suffix + 1, current_state.suffix, len + 1);
      current_state.suffix[0] = 'a';
    }

    sprintf(tmp, "a %s, ", result);

    if (current_state.searching_suffix) {
      strcpy(words[current_state.right_b--], tmp);
    } else {
      strcpy(words[current_state.left_b++], tmp);
    }

    current_state.searching_suffix = !current_state.searching_suffix;

    connect_words(words, current_state.left_b, current_state.right_b, res);
    printf("\x1B[2J");
    printf("\x1B[H");
    printf("poly: |%s|\n", res);
    printf("left: %ld right: %ld suff: %s\n", current_state.left_b,
           current_state.right_b, current_state.suffix);

    stack_push(stack, current_state);
  }

  connect_words(words, current_state.left_b, current_state.right_b, res);
  printf("res: %s\n", res);

  free(stack);
  return 0;
}
