#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zju_stack.h"

#define JSON_NULL 0
#define JSON_INT 1
#define JSON_DOUBLE 2
#define JSON_STRING 3
#define JSON_OBJECT 4
#define JSON_ARRAY_INT 11
#define JSON_ARRAY_DOUBLE 12
#define JSON_ARRAY_STRING 13
#define JSON_ARRAY_OBJECT 14

#define JSON_ARRAY 9999
#define JSON_EMPTY 9998
#define TMPLEN 128
typedef char * jsonstr_t;

/**
 * "jnode->type == JSON_EMPTY" stands for empty js object
 * "jnode->type == JSON_NULL" stands for js null value
 * "jnode->type == JSON_ARRAY" stands for empty js array
 */
typedef struct json_node {
  struct json_node *prev;
  struct json_node *next;
  int type;
  int arrlen;
  char *key;

  union {
    char *stringval;
    int intval;
    double doubleval;
    struct json_node *objval;
    int *intarr;
    double *doublearr;
    char **stringarr;
    struct json_node **objarr;
  } value;
} json_node_t;

json_node_t *json_parse(char **string);
json_node_t *json_create(json_node_t data[], int n);
void json_print(json_node_t *jnode);
void json_stringify(json_node_t *jnode, char ret[]);
void json_free(json_node_t **jnode); // only for deallocating jnode created by json_parse()

