#include <assert.h>
#include <stdio.h>
#include "stack.h"

int main() {
  stack_t st;

  stack_init(&st, 5);
  stack_push(&st, 'a');
  stack_push(&st, 'b');
  stack_push(&st, 'c');
  stack_push(&st, 'd');
  stack_push(&st, 'e');
  stack_push(&st, 'f');

  assert(stack_get(&st) == 'e');
  assert(stack_pop(&st) == 'e');
  assert(stack_pop(&st) == 'd');
  assert(stack_pop(&st) == 'c');
  assert(stack_pop(&st) == 'b');
  assert(stack_pop(&st) == 'a');
  assert(stack_pop(&st) == '\0');
  assert(stack_get(&st) == '\0');
  stack_del(&st);

  printf("ALL TEST PASSED!\n");

  return 0;
}

