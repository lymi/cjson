#include "./include/cjson.h"

#define JSNULL "null"

static json_node_t *json_parse_step(jsonstr_t *str, zju_stack_t *stack);
static void handle_number(char *pi, char *pj, json_node_t *jnode);
static void handle_numarr(json_node_t *jnode, char *pj, char *pk, int arrlen, int idx);
static int isnull(char *pi);
static void json_print_step(json_node_t *jnode, int indent);
static void printarray(json_node_t *jnode, int indent);
static void json_free_step(json_node_t *jnode);

static int TRUE = 1;
static int FALSE = 0;

/**************************************
 *         json_parse() start         *
 **************************************/
json_node_t *json_parse(jsonstr_t *string) {
  if (**string != '{') {
    printf("ERROR:: invalid json string.\n");
    return NULL;
  }

  zju_stack_t st;
  stack_init(&st, 128, STACK_CHAR, 0);
  json_node_t *jnode = json_parse_step(string, &st);
  stack_free(&st);
  return jnode;
}

static json_node_t *json_parse_step(jsonstr_t *pi, zju_stack_t *stack) {
  json_node_t *jnode = (json_node_t *)malloc(sizeof(json_node_t));
  char *pj, *pk, *segment;
  int cnt = 0, arrlen = 0, idx = 0, iskey = TRUE, isnum = FALSE, 
      isstr = FALSE, isarr = FALSE, isstrarr = FALSE, isnumarr = FALSE;

  jnode->next = NULL;
  jnode->prev = NULL;
  jnode->key = NULL;
  jnode->type = JSON_EMPTY;
  jnode->value.objarr = NULL;

  do {
    if (isarr) {
      if (arrlen == 0) {
        char *pt = *pi;
        int depth = 0;

        if (*pt != ']') {
          arrlen = 1;
        }

        while(*pt != ']') {
          if (*pt == '{') {
            depth ++;
          }
          if (*pt == '}') {
            depth --;
          }
          if (*pt == ',' && depth == 0) {
            arrlen ++;
          }
          pt++;
        }

        jnode->arrlen = arrlen;
      }

      if (**pi == '{' || isnull(*pi)) {
        if (jnode->value.objarr == NULL) {
          jnode->type = JSON_ARRAY_OBJECT;
          jnode->value.objarr = (json_node_t **)calloc(arrlen, sizeof(void *));
        }

        if (**pi == '{') {
          jnode->value.objarr[idx++] = json_parse(pi);
        } else if (isnull(*pi)) {
          json_node_t *nullobj = (json_node_t *)malloc(sizeof(json_node_t));
          nullobj->key = NULL;
          nullobj->prev = NULL;
          nullobj->next = NULL;
          nullobj->type = JSON_NULL;
          jnode->value.objarr[idx++] = nullobj;
          (*pi)+=4;
        }
        continue;
      } else if (**pi == '\"') {
        if (stack_getchar(stack) != '\"') {
          stack_pushchar(stack, '\"');
          pj = *pi;
          isstrarr = TRUE;
        } else {
          stack_pop(stack);
          pk = *pi;
          isstrarr = FALSE;
          cnt = pk - pj - 1;
          segment = (char *)malloc(cnt + 1);
          strncpy(segment, pj + 1, cnt);
          segment[cnt] = '\0';

          if (jnode->value.stringarr == NULL) {
            jnode->type = JSON_ARRAY_STRING;
            jnode->value.stringarr = (char **)calloc(arrlen, sizeof(void *));
          }
          jnode->value.stringarr[idx++] = segment;
        }
      } else if (**pi == ',') {
        pk = *pi;
        if (isnumarr) {
          isnumarr = FALSE;
          handle_numarr(jnode, pj, pk, arrlen, idx++);
        }
      } else if (**pi == ']') {
        if (stack_getchar(stack) != '[') {
          printf("ERROR::json_parse() error, unexpected ']'.\n");
          return NULL;
        }
        stack_pop(stack);
        isarr = FALSE;
        pk = *pi;

        if (isnumarr) {
          isnumarr = FALSE;
          handle_numarr(jnode, pj, pk, arrlen, idx++);
        }
      } else if (!isstrarr && !isnumarr) {
        isnumarr = TRUE;
        pj = *pi;
      }

      (*pi)++;
      continue;
    }

    if (**pi == '{') {
      if (iskey) { // a json object begins
        stack_pushchar(stack, '{');
      } else {
        jnode->type = JSON_OBJECT;
        jnode->value.objval = json_parse(pi);
        continue;
      }
    } else if (**pi == '[') {
      jnode->type = JSON_ARRAY;
      stack_pushchar(stack, '[');
      isarr = TRUE;
      idx = 0;
    } else if (**pi == '\"') {
      if (stack_getchar(stack) != '\"') {
        stack_pushchar(stack, '\"');
        pj = *pi;
        isstr = TRUE;
      } else {
        stack_pop(stack);
        isstr = FALSE;
        pk = *pi;
        cnt = pk - pj - 1;

        segment = (char *)malloc(cnt + 1);
        strncpy(segment, pj+1, cnt);
        segment[cnt] = '\0';
        
        if (iskey) {
          jnode->key = segment;
        } else {
          jnode->type = JSON_STRING;
          jnode->value.stringval = segment;
        }
      }
    } else if (**pi == '}') {
      if (stack_getchar(stack) != '{') {
        printf("ERROR::json_parse() error, unexpected '}'.\n");
        return NULL;
      }
      stack_pop(stack);
      pk = *pi;
      if (isnum) {
        isnum = FALSE;
        handle_number(pj, pk, jnode);
      }
    } else if (**pi == ':') {
      iskey = FALSE;
    } else if (**pi == ',') {
      iskey = TRUE;
      pk = *pi;
      if (isnum) {
        isnum = FALSE;
        handle_number(pj, pk, jnode);
      }

      (*pi)++;
      jnode->next = json_parse_step(pi, stack);
      jnode->next->prev = jnode;
      continue;
    } else if (!isstr && !isnum) {
      if ((**pi >= '0' && **pi <= '9') || **pi == '.'){
        isnum = TRUE;
        pj = *pi;
      } else if (isnull(*pi)) {
        jnode->type = JSON_NULL;
        (*pi)+=4;
        continue;
      }
    }

    (*pi)++;
  } while (stack_count(stack) != 0);
  
  return jnode;
}

static void handle_number(char *pj, char *pk, json_node_t *jnode) {
  assert(jnode != NULL);
  int cnt = pk - pj;
  char temp[cnt + 1];
  strncpy(temp, pj, cnt);
  temp[cnt] = '\0';

  if (index(temp, '.')) {
    jnode->type = JSON_DOUBLE;
    jnode->value.doubleval = atof(temp);
  } else {
    jnode->type = JSON_INT;
    jnode->value.intval = atoi(temp);
  }
}

static void handle_numarr(json_node_t *jnode, char *pj, char *pk, int arrlen, int idx) {
  assert(jnode != NULL);
  int cnt = pk - pj;
  char temp[cnt + 1];
  strncpy(temp, pj, cnt);
  temp[cnt] = '\0';

  if (index(temp, '.')) {
    if (jnode->value.doublearr == NULL) {
      jnode->type = JSON_ARRAY_DOUBLE;
      jnode->value.doublearr = (double *)calloc(arrlen, sizeof(double));
    }
    jnode->value.doublearr[idx] = atof(temp);
  } else {
    if (jnode->value.intarr == NULL) {
      jnode->type = JSON_ARRAY_INT;
      jnode->value.intarr = (int *)calloc(arrlen, sizeof(int));
    }
    jnode->value.intarr[idx] = atoi(temp);
  }
}

static int isnull(char *pi) {
  char temp[5];
  strncpy(temp, pi, 4);
  temp[4] = '\0';
  if (strcmp(temp, JSNULL) == 0) {
    return TRUE;
  }
  return FALSE;
}

/**************************************
 *         json_print() start         *
 **************************************/
void json_print(json_node_t *jnode) {
  assert(jnode != NULL); 
  json_print_step(jnode, 0);
  printf("\n");
}

static void json_print_step(json_node_t *jnode, int indent) {
  int type = jnode->type;

  if (type == JSON_NULL && jnode->key == NULL) {
    printf("null");
    return;
  }

  if (type == JSON_EMPTY) {
    printf("{}");
    return;
  }

  if (jnode->prev == NULL) {
    printf("{\n");
  }

  for (int i = 0; i < indent + 2; i++) {
    printf(" ");
  }

  switch (type) {
    case JSON_INT:
      printf("%s: %d", jnode->key, jnode->value.intval);
      break;
    case JSON_DOUBLE:
      printf("%s: %f", jnode->key, jnode->value.doubleval);
      break;
    case JSON_STRING:
      printf("%s: %s", jnode->key, jnode->value.stringval);
      break;
    case JSON_OBJECT:
      printf("%s: ", jnode->key);
      json_print_step(jnode->value.objval, indent + 2);
      break;
    case JSON_NULL:
      printf("%s: null", jnode->key);
      break;
    default:
      printarray(jnode, indent);
  }

  if (jnode->next == NULL) {
    printf("\n");
    for (int i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("}");
  } else {
    printf(",\n");
    json_print_step(jnode->next, indent);
  }
}

static void printarray(json_node_t *jnode, int indent) {
  int type = jnode->type;

  printf("%s: [", jnode->key);

  for (int i = 0; i < jnode->arrlen; i++) {
    switch (type) {
      case JSON_ARRAY_INT:
        printf("%d", jnode->value.intarr[i]);
        break;
      case JSON_ARRAY_DOUBLE:
        printf("%f", jnode->value.doublearr[i]);
        break;
      case JSON_ARRAY_STRING:
        printf("%s", jnode->value.stringarr[i]);
        break;
      case JSON_ARRAY_OBJECT:
        json_print_step(jnode->value.objarr[i], indent + 2);
        break;
    }

    if (i < jnode->arrlen - 1) {
      printf(", ");
    }
  }
  printf("]");
}

/**************************************
 *        json_create() start         *
 **************************************/
json_node_t *json_create(json_node_t data[], int n) {
  for (int i = 0; i < n; i++) {
    if (i == 0) {
      data[i].prev = NULL;
    } else {
      data[i].prev = &data[i-1];
    }

    if (i == n - 1) {
      data[i].next = NULL;
    } else {
      data[i].next = &data[i+1];
    }
  }

  return &data[0];
}

/**************************************
 *      json_stringify() start        *
 **************************************/
void json_stringify(json_node_t *jnode, char ret[]) {
  assert(jnode != NULL);
  int type = jnode->type;

  if (type == JSON_NULL && jnode->key == NULL) {
    strcat(ret, "null");
    return;
  }

  if (type == JSON_EMPTY) {
    strcat(ret, "{}");
    return;
  }

  if (jnode->prev == NULL) {
    strcat(ret, "{");
  }

  char temp[TMPLEN]; 
  sprintf(temp, "\"%s\":", jnode->key);
  strcat(ret, temp);
  switch(type) {
    case JSON_INT:
      snprintf(temp, TMPLEN, "%d", jnode->value.intval);
      strcat(ret, temp);
      break;
    case JSON_DOUBLE:
      snprintf(temp, TMPLEN, "%f", jnode->value.doubleval);
      strcat(ret, temp);
      break;
    case JSON_STRING:
      snprintf(temp, TMPLEN, "\"%s\"", jnode->value.stringval);
      strcat(ret, temp);
      break;
    case JSON_OBJECT:
      json_stringify(jnode->value.objval, ret);
      break;
    case JSON_NULL:
      strcat(ret, "null");
      break;
    default:
      strcat(ret, "[");
      for (int i = 0; i < jnode->arrlen; i++) {
        switch(type) {
          case JSON_ARRAY_INT:
            snprintf(temp, TMPLEN, "%d", jnode->value.intarr[i]);
            strcat(ret, temp);
            break;
          case JSON_ARRAY_DOUBLE:
            snprintf(temp, TMPLEN, "%f", jnode->value.doublearr[i]);
            strcat(ret, temp);
            break;
          case JSON_ARRAY_STRING:
            snprintf(temp, TMPLEN, "\"%s\"", jnode->value.stringarr[i]);
            strcat(ret, temp);
            break;
          case JSON_ARRAY_OBJECT:
            json_stringify(jnode->value.objarr[i], ret);
            break;
        }

        if (i < jnode->arrlen - 1) {
          strcat(ret, ",");
        }
      }
      strcat(ret, "]");
  }

  if (jnode->next == NULL) {
    strcat(ret, "}");
  } else {
    strcat(ret, ",");
    json_stringify(jnode->next, ret);
  }
}

/**************************************
 *         json_free() start          *
 **************************************/
void json_free(json_node_t **jnode) {
  assert(jnode != NULL);

  while((*jnode)->next != NULL) {
    *jnode = (*jnode)->next;
  }

  json_free_step(*jnode);
  *jnode = NULL;
}

static void json_free_step(json_node_t *jnode) {
  if (jnode->key != NULL) {
    free(jnode->key);
    jnode->key = NULL;
  }

  int type = jnode->type;

  switch(type) {
    case JSON_STRING:
      free(jnode->value.stringval);
      break;
    case JSON_OBJECT:
      json_free(&(jnode->value.objval));
      break;
    case JSON_ARRAY_INT:
      free(jnode->value.intarr);
      break;
    case JSON_ARRAY_DOUBLE:
      free(jnode->value.doublearr);
      break;
    case JSON_ARRAY_STRING:
      for (int i = 0; i < jnode->arrlen; i++) {
        free(jnode->value.stringarr[i]);
      }
      free(jnode->value.stringarr);
      break;
    case JSON_ARRAY_OBJECT:
      for (int i = 0; i < jnode->arrlen; i++) {
        json_free(&(jnode->value.objarr[i]));
      }
      free(jnode->value.objarr);
      break;
  }

  if (jnode->prev != NULL) {
    json_free_step(jnode->prev);
  }

  free(jnode);
}

