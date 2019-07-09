#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zjustack.h"

#define CJSON_TYPE_NULL 0
#define CJSON_TYPE_INT 1
#define CJSON_TYPE_DOUBLE 2
#define CJSON_TYPE_STRING 3
#define CJSON_TYPE_OBJECT 4
#define CJSON_TYPE_INTARR 11
#define CJSON_TYPE_DBLARR 12
#define CJSON_TYPE_STRARR 13
#define CJSON_TYPE_OBJARR 14

#define CJSON_ARRAY 9999
#define CJSON_EMPTY 9998
#define TMPLEN 128
typedef char * jsonstr_t;

/**
 * "jnode->type == CJSON_TYPE_EMPTY" stands for empty js object
 * "jnode->type == CJSON_TYPE_NULL" stands for js null value
 * "jnode->type == CJSON_TYPE_ARRAY" stands for empty js array
 */
typedef struct cjson_node {
  struct cjson_node *prev;
  struct cjson_node *next;
  int type;
  int arrlen;
  char *key;

  union {
    char *stringval;
    int intval;
    double doubleval;
    struct cjson_node *objval;
    int *intarr;
    double *doublearr;
    char **stringarr;
    struct cjson_node **objarr;
  } value;
} cjson_node_t;

cjson_node_t *cjson_parse(char **string);
cjson_node_t *cjson_create(cjson_node_t data[], int n);
void cjson_print(cjson_node_t *jnode);
void cjson_stringify(cjson_node_t *jnode, char ret[]);
void cjson_free(cjson_node_t **jnode); // only for deallocating jnode created by cjson_parse()

