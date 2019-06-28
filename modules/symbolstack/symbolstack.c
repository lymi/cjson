#include "symbolstack.h"

void symbolstack_init(symbolstack_t *st, int maxlen) {
  st->symbolstack = (char *)calloc(maxlen, 1);
  st->maxlen = maxlen;
  st->top = 0;
}

int symbolstack_push(symbolstack_t *st, char val) {
  if (st->maxlen == st->top) {
    return -1;
  }
  st->symbolstack[(st->top)++] = val;
  return 0;
}

char symbolstack_get(symbolstack_t *st) {
  if (st->top == 0) {
    return '\0';
  }
  return st->symbolstack[st->top - 1];
}

char symbolstack_pop(symbolstack_t *st) {
  if (st->top == 0) {
    return '\0';
  }
  return st->symbolstack[--st->top];
}

void symbolstack_free(symbolstack_t *st) {
  free(st->symbolstack);
  st->symbolstack = NULL;
  st->maxlen = 0;
  st->top = 0;
}

int symbolstack_neles(symbolstack_t *st) {
  return st->top;
}

