#!/bin/bash
gcc -o test.o test.c ../lib/*.so
./test.o

