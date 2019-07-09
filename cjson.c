#include "./include/cjson.h"

#define JSNULL "null"

static cj_node_t *cj_parse_step(jsonstr_t *str, zjustack_t *stack);
static void handle_number(char *pi, char *pj, cj_node_t *jnode);
static void handle_numarr(cj_node_t *jnode, char *pj, char *pk, int arrlen, int idx);
static int isnull(char *pi);
static void cj_print_step(cj_node_t *jnode, int indent);
static void printarray(cj_node_t *jnode, int indent);
static void cj_free_step(cj_node_t *jnode);

static int TRUE = 1;
static int FALSE = 0;

/**************************************
 *         cj_parse() start         *
 **************************************/
cj_node_t *cj_parse(jsonstr_t *string) {
  if (**string != '{') {
    printf("ERROR:: invalid json string.\n");
    return NULL;
  }

  zjustack_t st;
  stack_init(&st, 128, STACK_CHAR, 0);
  cj_node_t *jnode = cj_parse_step(string, &st);
  stack_free(&st);
  return jnode;
}

static cj_node_t *cj_parse_step(jsonstr_t *pi, zjustack_t *stack) {
  cj_node_t *jnode = (cj_node_t *)malloc(sizeof(cj_node_t));
  char *pj, *pk, *segment;
  int cnt = 0, arrlen = 0, idx = 0, iskey = TRUE, isnum = FALSE, 
      isstr = FALSE, isarr = FALSE, isstrarr = FALSE, isnumarr = FALSE;

  jnode->next = NULL;
  jnode->prev = NULL;
  jnode->key = NULL;
  jnode->type = CJSON_EMPTY;
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
          jnode->type = CJSON_TYPE_OBJARR;
          jnode->value.objarr = (cj_node_t **)calloc(arrlen, sizeof(void *));
        }

        if (**pi == '{') {
          jnode->value.objarr[idx++] = cj_parse(pi);
        } else if (isnull(*pi)) {
          cj_node_t *nullobj = (cj_node_t *)malloc(sizeof(cj_node_t));
          nullobj->key = NULL;
          nullobj->prev = NULL;
          nullobj->next = NULL;
          nullobj->type = CJSON_TYPE_NULL;
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
            jnode->type = CJSON_TYPE_STRARR;
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
          printf("ERROR::cj_parse() error, unexpected ']'.\n");
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
        jnode->type = CJSON_TYPE_OBJECT;
        jnode->value.objval = cj_parse(pi);
        continue;
      }
    } else if (**pi == '[') {
      jnode->type = CJSON_ARRAY;
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
          jnode->type = CJSON_TYPE_STRING;
          jnode->value.stringval = segment;
        }
      }
    } else if (**pi == '}') {
      if (stack_getchar(stack) != '{') {
        printf("ERROR::cj_parse() error, unexpected '}'.\n");
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
      jnode->next = cj_parse_step(pi, stack);
      jnode->next->prev = jnode;
      continue;
    } else if (!isstr && !isnum) {
      if ((**pi >= '0' && **pi <= '9') || **pi == '.'){
        isnum = TRUE;
        pj = *pi;
      } else if (isnull(*pi)) {
        jnode->type = CJSON_TYPE_NULL;
        (*pi)+=4;
        continue;
      }
    }

    (*pi)++;
  } while (stack_count(stack) != 0);
  
  return jnode;
}

static void handle_number(char *pj, char *pk, cj_node_t *jnode) {
  assert(jnode != NULL);
  int cnt = pk - pj;
  char temp[cnt + 1];
  strncpy(temp, pj, cnt);
  temp[cnt] = '\0';

  if (index(temp, '.')) {
    jnode->type = CJSON_TYPE_DOUBLE;
    jnode->value.doubleval = atof(temp);
  } else {
    jnode->type = CJSON_TYPE_INT;
    jnode->value.intval = atoi(temp);
  }
}

static void handle_numarr(cj_node_t *jnode, char *pj, char *pk, int arrlen, int idx) {
  assert(jnode != NULL);
  int cnt = pk - pj;
  char temp[cnt + 1];
  strncpy(temp, pj, cnt);
  temp[cnt] = '\0';

  if (index(temp, '.')) {
    if (jnode->value.doublearr == NULL) {
      jnode->type = CJSON_TYPE_DBLARR;
      jnode->value.doublearr = (double *)calloc(arrlen, sizeof(double));
    }
    jnode->value.doublearr[idx] = atof(temp);
  } else {
    if (jnode->value.intarr == NULL) {
      jnode->type = CJSON_TYPE_INTARR;
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
 *         cj_print() start         *
 **************************************/
void cj_print(cj_node_t *jnode) {
  assert(jnode != NULL); 
  cj_print_step(jnode, 0);
  printf("\n");
}

static void cj_print_step(cj_node_t *jnode, int indent) {
  int type = jnode->type;

  if (type == CJSON_TYPE_NULL && jnode->key == NULL) {
    printf("null");
    return;
  }

  if (type == CJSON_EMPTY) {
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
    case CJSON_TYPE_INT:
      printf("%s: %d", jnode->key, jnode->value.intval);
      break;
    case CJSON_TYPE_DOUBLE:
      printf("%s: %f", jnode->key, jnode->value.doubleval);
      break;
    case CJSON_TYPE_STRING:
      printf("%s: %s", jnode->key, jnode->value.stringval);
      break;
    case CJSON_TYPE_OBJECT:
      printf("%s: ", jnode->key);
      cj_print_step(jnode->value.objval, indent + 2);
      break;
    case CJSON_TYPE_NULL:
      printf("%s: null", jnode->key);
      break;
    default: printarray(jnode, indent);
  }

  if (jnode->next == NULL) {
    printf("\n");
    for (int i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("}");
  } else {
    printf(",\n");
    cj_print_step(jnode->next, indent);
  }
}

static void printarray(cj_node_t *jnode, int indent) {
  int type = jnode->type;

  printf("%s: [", jnode->key);

  for (int i = 0; i < jnode->arrlen; i++) {
    switch (type) {
      case CJSON_TYPE_INTARR:
        printf("%d", jnode->value.intarr[i]);
        break;
      case CJSON_TYPE_DBLARR:
        printf("%f", jnode->value.doublearr[i]);
        break;
      case CJSON_TYPE_STRARR:
        printf("%s", jnode->value.stringarr[i]);
        break;
      case CJSON_TYPE_OBJARR:
        cj_print_step(jnode->value.objarr[i], indent + 2);
        break;
    }

    if (i < jnode->arrlen - 1) {
      printf(", ");
    }
  }
  printf("]");
}

/**************************************
 *        cj_create() start         *
 **************************************/
cj_node_t *cj_create(cj_node_t data[], int n) {
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
 *      cj_stringify() start        *
 **************************************/
void cj_stringify(cj_node_t *jnode, char ret[]) {
  assert(jnode != NULL);
  int type = jnode->type;

  if (type == CJSON_TYPE_NULL && jnode->key == NULL) {
    strcat(ret, "null");
    return;
  }

  if (type == CJSON_EMPTY) {
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
    case CJSON_TYPE_INT:
      snprintf(temp, TMPLEN, "%d", jnode->value.intval);
      strcat(ret, temp);
      break;
    case CJSON_TYPE_DOUBLE:
      snprintf(temp, TMPLEN, "%f", jnode->value.doubleval);
      strcat(ret, temp);
      break;
    case CJSON_TYPE_STRING:
      snprintf(temp, TMPLEN, "\"%s\"", jnode->value.stringval);
      strcat(ret, temp);
      break;
    case CJSON_TYPE_OBJECT:
      cj_stringify(jnode->value.objval, ret);
      break;
    case CJSON_TYPE_NULL:
      strcat(ret, "null");
      break;
    default:
      strcat(ret, "[");
      for (int i = 0; i < jnode->arrlen; i++) {
        switch(type) {
          case CJSON_TYPE_INTARR:
            snprintf(temp, TMPLEN, "%d", jnode->value.intarr[i]);
            strcat(ret, temp);
            break;
          case CJSON_TYPE_DBLARR:
            snprintf(temp, TMPLEN, "%f", jnode->value.doublearr[i]);
            strcat(ret, temp);
            break;
          case CJSON_TYPE_STRARR:
            snprintf(temp, TMPLEN, "\"%s\"", jnode->value.stringarr[i]);
            strcat(ret, temp);
            break;
          case CJSON_TYPE_OBJARR:
            cj_stringify(jnode->value.objarr[i], ret);
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
    cj_stringify(jnode->next, ret);
  }
}

/**************************************
 *         cj_free() start          *
 **************************************/
void cj_free(cj_node_t **jnode) {
  assert(jnode != NULL);

  while((*jnode)->next != NULL) {
    *jnode = (*jnode)->next;
  }

  cj_free_step(*jnode);
  *jnode = NULL;
}

static void cj_free_step(cj_node_t *jnode) {
  if (jnode->key != NULL) {
    free(jnode->key);
    jnode->key = NULL;
  }

  int type = jnode->type;

  switch(type) {
    case CJSON_TYPE_STRING:
      free(jnode->value.stringval);
      break;
    case CJSON_TYPE_OBJECT:
      cj_free(&(jnode->value.objval));
      break;
    case CJSON_TYPE_INTARR:
      free(jnode->value.intarr);
      break;
    case CJSON_TYPE_DBLARR:
      free(jnode->value.doublearr);
      break;
    case CJSON_TYPE_STRARR:
      for (int i = 0; i < jnode->arrlen; i++) {
        free(jnode->value.stringarr[i]);
      }
      free(jnode->value.stringarr);
      break;
    case CJSON_TYPE_OBJARR:
      for (int i = 0; i < jnode->arrlen; i++) {
        cj_free(&(jnode->value.objarr[i]));
      }
      free(jnode->value.objarr);
      break;
  }

  if (jnode->prev != NULL) {
    cj_free_step(jnode->prev);
  }

  free(jnode);
}

