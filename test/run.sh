#!/bin/bash
gcc -o test.o test.c ../cjson.c ../lib/stack.so
./test.o

