#!/bin/bash
gcc -o test.o test.c ../build/cjson.so ../lib/stack.so
./test.o

