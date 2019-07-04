#include "../include/cjson.h"

int main() {
  jsonstr_t jsonstr;
  char ret[1024] = "\0";
  json_node_t *jn;

  printf("/* TEST 1: empty json object */\n");
  jsonstr = "{}";
  jn = json_parse(&jsonstr);
  json_print(jn);

  printf("\n/* TEST 2: normal json string */\n");
  jsonstr = "{\"product_name\":\"P30\",\"price\":6000,\"size\":[\"big\",\"medium\",\"small\"],\"buyers\":[null,{},{\"name\":\"shadiao\",\"age\":100,\"sex\":null},{\"name\":\"shabi\",\"age\":22.5,\"sex\":\"girl\"}],\"version\":null}";
  jn = json_parse(&jsonstr);
  json_print(jn);

  printf("\n/* TEST 3: stringify */\n");
  json_stringify(jn, ret);
  printf("%s\n", ret);
  json_free(&jn);

  printf("\n/* TEST 4: build up a json object */\n");
  json_node_t jnodes1[3];
  json_node_t jnodes2[2];

  int arr[5] = { 111, 222, 333, 444, 555 };

  jnodes2[0].key = "haha";
  jnodes2[0].type = JSON_DOUBLE;
  jnodes2[0].value.doubleval = 1.2345;
  jnodes2[1].key = "wocao";
  jnodes2[1].type = JSON_STRING;
  jnodes2[1].value.stringval = "hello world!";
  jnodes1[0].key = "aa";
  jnodes1[0].type = JSON_INT;
  jnodes1[0].value.intval = 666;
  jnodes1[1].key = "bb";
  jnodes1[1].type = JSON_OBJECT;
  jnodes1[1].value.objval = json_create(jnodes2, 2);
  jnodes1[2].key = "cc";
  jnodes1[2].type = JSON_ARRAY_INT;
  jnodes1[2].arrlen = 5;
  jnodes1[2].value.intarr = arr;

  json_node_t *headnode = json_create(jnodes1, 3);
  json_print(headnode);
  ret[0] = '\0';
  json_stringify(headnode, ret);
  printf("%s\n", ret);

  return 0;
}

