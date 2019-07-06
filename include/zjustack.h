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
} zjustack_t;

void stack_init(zjustack_t *st, int size, int type, int elesize);
/* push */
int stack_pushchar(zjustack_t *st, char val);
int stack_pushint(zjustack_t *st, int val);
int stack_pushdouble(zjustack_t *st, double val);
int stack_pushstring(zjustack_t *st, char *val);
int stack_pushref(zjustack_t *st, void *val);
/* get */
char stack_getchar(zjustack_t *st);
int stack_getint(zjustack_t *st);
double stack_getdouble(zjustack_t *st);
char *stack_getstring(zjustack_t *st);
void *stack_getref(zjustack_t *st);
void *stack_getref_copy(zjustack_t *st);

int stack_pop(zjustack_t *st);
void stack_free(zjustack_t *st);
int stack_count(zjustack_t *st);

