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

void dump_stack(stack *st) {
  printf("Stack dump: \n");
  printf("count: %ld\n", st->count);
  for (size_t i = 0; i < st->count; i++) {
    printf("\t [%ld] poly: %s\t suff: %s\n", i, st->items[i].poly, st->items[i].suffix);
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

const char init_poly[] = "A man, a plan, a canal, Panama";
const size_t chars_start_lower[26];
const size_t chars_start_upper[26];

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
    if (sl == 0) {
      printf("found: |%s| suff: |%s|\n", word, suffix);
      memcpy(result, word, wl);
      hash_map_set(word);
      return true;
    }
    for (w = wl - 2, s = 0; w >= 0 && s < sl; w--, s++) {
      if (word[w] != suffix[s]) {
        goto next_word;
      }
    }
    if (hash_map_get(word))
      goto next_word;
    hash_map_set(word);

    printf("found: |%s| suff: |%s|\n", word, suffix);
    memcpy(result, word, wl);
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
next_word:
  while (fgets(word, MAX_WORD_LENGTH, f) != NULL) {
    wl = strlen(word);
    if (wl < 2)
      goto next_word;
    word[wl - 1] = '\0';
    if (pl == 0) {
      printf("found: |%s| preff: |%s|\n", word, preffix);
      memcpy(result, word, wl);
      hash_map_set(word);
      return true;
    }
    if (strncmp(preffix, word, pl) == 0) {

      if (hash_map_get(word))
        goto next_word;
      hash_map_set(word);

      printf("found: |%s| preff: |%s|\n", word, preffix);
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

int main(void) {
  FILE *nouns = fopen("nouns.txt", "r");

  char tmp[MAX_WORD_LENGTH];
  char result[MAX_WORD_LENGTH];
  state current_state, prev_state, state_copy;
  strcpy(current_state.poly, "A man, a plan, a canal, Panama");
  strcpy(current_state.suffix, "ca");
  current_state.insert_pos = 15;
  current_state.searching_suffix = false;

  state init_state;
  init_state = current_state;
  stack *stack = create_stack(init_state);
  stack_push(stack, init_state);

  size_t len, suff_len;

  // for (size_t i = 0; i < 50; i++) {
  while (strlen(current_state.poly) < 100) {
    if (current_state.searching_suffix) {
      if (!search_suffix(current_state.suffix, nouns, result)) {
        fprintf(stderr, "Couldn't find suffix %s\n", current_state.suffix);
        dump_stack(stack);
        current_state = stack_pop(stack);
        printf("poly: |%s| suff: %s\n", current_state.poly, current_state.suffix);
        continue;
      }
    } else {
      if (!search_preffix(current_state.suffix, nouns, result)) {
        fprintf(stderr, "Couldn't find preffix %s\n", current_state.suffix);
        dump_stack(stack);
        current_state = stack_pop(stack);
        printf("poly: |%s| suff: %s\n", current_state.poly, current_state.suffix);
        continue;
      }
    }

    len = strlen(result);
    suff_len = strlen(current_state.suffix);
    if (current_state.searching_suffix) {
      strncpy(current_state.suffix, result, len - suff_len);
      current_state.suffix[len-suff_len] = '\0';
      reverse_str(current_state.suffix);
    } else {
      strcpy(current_state.suffix, result + suff_len);
      len = strlen(current_state.suffix);
      // reverse_str(current_state.suffix);
      current_state.suffix[len] = 'a';
      current_state.suffix[len + 1] = '\0';
    }
    printf("new suffix: %s\n", current_state.suffix);
    sprintf(tmp, "a %s, ", result);
    printf("End result: |%s|\n", tmp);

    insert_str_at(current_state.poly, tmp, current_state.insert_pos);
    if (!current_state.searching_suffix) {
      current_state.insert_pos += strlen(tmp);
    }
    current_state.searching_suffix = !current_state.searching_suffix;
    printf("poly: |%s|\n", current_state.poly);
    printf("\n");

    stack_push(stack, current_state);
    dump_stack(stack);
  }

  free(stack);
  return 0;
}
