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
} zju_stack_t;

void stack_init(zju_stack_t *st, int size, int type, int elesize);
/* push */
int stack_pushchar(zju_stack_t *st, char val);
int stack_pushint(zju_stack_t *st, int val);
int stack_pushdouble(zju_stack_t *st, double val);
int stack_pushstring(zju_stack_t *st, char *val);
int stack_pushref(zju_stack_t *st, void *val);
/* get */
char stack_getchar(zju_stack_t *st);
int stack_getint(zju_stack_t *st);
double stack_getdouble(zju_stack_t *st);
char *stack_getstring(zju_stack_t *st);
void *stack_getref(zju_stack_t *st);
void *stack_getref_copy(zju_stack_t *st);

int stack_pop(zju_stack_t *st);
void stack_free(zju_stack_t *st);
int stack_count(zju_stack_t *st);

