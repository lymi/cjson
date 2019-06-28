#include <stdlib.h>

typedef struct {
  char *symbolstack;
  int maxlen;
  int top;
} symbolstack_t;

void symbolstack_init(symbolstack_t *st, int maxlen);
int symbolstack_push(symbolstack_t *st, char val);
char symbolstack_get(symbolstack_t *st);
char symbolstack_pop(symbolstack_t *st);
void symbolstack_free(symbolstack_t *st);
int symbolstack_neles(symbolstack_t *st);

