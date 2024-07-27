#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 30
#define MAX_WORDS_COUNT 50

typedef struct {
  char poly[250];
  char suffix[MAX_WORD_LENGTH];
  size_t insert_pos;
  bool searching_suffix;
} state;

state copy_state(state s) {
  state ns;
  ns.searching_suffix = s.searching_suffix;
  ns.insert_pos = s.insert_pos;
  strcpy(ns.poly, s.poly);
  strcpy(ns.suffix, s.suffix);
  return ns;
}

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

#define STACK_COPACITY 100

typedef struct {
  state items[STACK_COPACITY];
  size_t count;
  state top;
  state zero_val;
} stack;

void stack_push(stack *st, state s);
state stack_pop(stack *st);

stack *create_stack(state zvalue) {
  stack *st = malloc(sizeof(stack));
  st->count = 0;
  st->top = st->items[0];
  st->zero_val = zvalue;
  return st;
}

void free_stack(stack *stack) {
  state s;
  for (size_t i = 0; i < stack->count; i++) {
    s = stack_pop(stack);
    printf("freeing poly: %s\n", s.poly);
  }
}

void dump_stack(stack *st) {
  printf("Stack dump: \n");
  for (size_t i = 0; i < st->count; i++) {
    printf("\t %s\n", st->items[i].poly);
  }
}

void stack_push(stack *st, state s) {
  if (st->count >= STACK_COPACITY) {
    fprintf(stderr, "Stack overflow\n");
    exit(1);
  }
  st->items[st->count++] = s;
}

state stack_pop(stack *st) {
  if (st->count <= 0) {
    return st->zero_val;
  }
  return st->items[--st->count];
}

#define END_OF_UPPERCASE_WORDS 75067

const char init_poly[] = "A man, a plan, a canal, Panama";
const size_t chars_start_lower[26];
const size_t chars_start_upper[26];

char used_words[50][MAX_WORDS_COUNT];
size_t used_words_count = 0;
size_t used_words_index = 0;

bool search_suffix(char *suffix, FILE *f, char *result) {
  printf("Searching suffix: %s\n", suffix);
  char word[MAX_WORD_LENGTH];
  fseek(f, END_OF_UPPERCASE_WORDS, SEEK_SET);
  size_t wl, sl = strlen(suffix);
  int w, s;
next_word:
  while (fgets(word, MAX_WORD_LENGTH, f) != NULL) {
    wl = strlen(word);
    if (wl < 2)
      goto next_word;
    word[wl - 1] = '\0'; // get rid of \n
    for (w = wl - 2, s = 0; w >= 0 && s < sl; w--, s++) {
      if (word[w] != suffix[s]) {
        goto next_word;
      }
    }
    for (size_t i = 0; i < used_words_index; i++) {
      if (strcmp(word, used_words[i]) == 0)
        goto next_word;
    }
    printf("found: |%s| suff: |%s|\n", word, suffix);
    memcpy(result, word, wl);
    memcpy(used_words[used_words_index++], word, wl);
    printf("%s\n", used_words[used_words_index]);
    fflush(stdout);
    return true;
  }
  return false;
}

bool search_preffix(char *preffix, FILE *f, char *result) {
  size_t wl, pl;
  pl = strlen(preffix);
  printf("Searching preffix: |%s|\n", preffix);
  char word[MAX_WORD_LENGTH];
  fseek(f, END_OF_UPPERCASE_WORDS, SEEK_SET);
  printf("current file pointer pos: %ld\n", ftell(f));
next_word:
  while (fgets(word, MAX_WORD_LENGTH, f) != NULL) {
    wl = strlen(word);
    word[wl - 1] = '\0';
    if (strncmp(preffix, word, pl) == 0) {
      for (size_t i = 0; i < used_words_index; i++) {
        if (strcmp(word, used_words[i]) == 0) {
          goto next_word;
        }
      }
      memcpy(result, word, wl);
      memcpy(used_words[used_words_index++], word, wl);
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

int main(void) {
  FILE *nouns = fopen("nouns.txt", "r");

  stack *stack = create_stack();
  char tmp[MAX_WORD_LENGTH];
  char result[MAX_WORD_LENGTH];
  state current_state, prev_state, state_copy;
  strcpy(current_state.poly, "A man, a plan, a canal, Panama");
  strcpy(current_state.suffix, "ca");
  current_state.insert_pos = 15;
  current_state.searching_suffix = false;

  state init_state;
  init_state = current_state;

  size_t len, suff_len;

  for (size_t i = 0; i < 50; i++) {
    if (current_state.searching_suffix) {
      if (!search_suffix(current_state.suffix, nouns, result)) {
        fprintf(stderr, "Couldn't find suffix %s\n", current_state.suffix);
        current_state = stack_pop(stack);
        printf("poly: |%s|\n", current_state.poly);
        continue;
      }
    } else {
      if (!search_preffix(current_state.suffix, nouns, result)) {
        fprintf(stderr, "Couldn't find preffix %s\n", current_state.suffix);
        current_state = stack_pop(stack);
        printf("poly: |%s|\n", current_state.poly);
        continue;
      }
    }
    // TODO:cache all state

    len = strlen(result);
    suff_len = strlen(current_state.suffix);
    printf("result: |%s| len: %ld\n", result, len);
    if (current_state.searching_suffix) {
      strncpy(current_state.suffix, result, len - suff_len);
    } else {

      strcpy(current_state.suffix, result + suff_len);
      current_state.suffix[strlen(current_state.suffix)] = 'a';
      current_state.suffix[strlen(current_state.suffix) + 1] = '\0';
    }
    printf("new suffix: %s\n", current_state.suffix);
    sprintf(tmp, "a %s, ", result);
    printf("result: |%s| len: %ld\n", tmp, len);

    insert_str_at(current_state.poly, tmp, current_state.insert_pos);
    if (current_state.searching_suffix) {
      // insert_pos -= strlen(tmp);
    } else {
      current_state.insert_pos += strlen(tmp);
    }
    current_state.searching_suffix = !current_state.searching_suffix;
    printf("poly: |%s|\n", current_state.poly);
    printf("\n");

    stack_push(stack, current_state);
    dump_stack(stack);
  }

  for (size_t i = 0; i < used_words_index; i++) {
    printf("%s\n", used_words[i]);
  }

  return 0;
}
