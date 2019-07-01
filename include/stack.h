#include <stdlib.h>
#include <memory.h>
#include <limits.h>
#include <float.h>

#define STACK_CHAR 1
#define STACK_INT 2
#define STACK_DOUBLE 3
#define STACK_STRING 4
#define STACK_HEAPREF 5

typedef struct {
  int size;
  int top;
  int type;
  int elesize; // if type != 
  
  union {
    char *stack_char;
    int *stack_int;
    double *stack_double;
    char **stack_string;
    void **stack_heapref;
  } stack;
} stack_t;

void stack_init(stack_t *st, int size, int type, int elesize);
/* push */
int stack_pushchar(stack_t *st, char val);
int stack_pushint(stack_t *st, int val);
int stack_pushdouble(stack_t *st, double val);
int stack_pushstring(stack_t *st, char *val);
int stack_pushref(stack_t *st, void *val);
/* get */
char stack_getchar(stack_t *st);
int stack_getint(stack_t *st);
double stack_getdouble(stack_t *st);
char *stack_getstring(stack_t *st);
void *stack_getref(stack_t *st);
void *stack_getref_copy(stack_t *st);

int stack_pop(stack_t *st);
void stack_free(stack_t *st);
int stack_count(stack_t *st);

