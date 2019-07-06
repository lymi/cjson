#!/bin/bash
gcc -o test.o test.c ../cjson.c ../lib/zjustack.so
./test.o

