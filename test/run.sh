#!/bin/bash
gcc -o test.o test.c ../cjson.c ../lib/zju_stack.so
./test.o

