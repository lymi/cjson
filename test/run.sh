#!/bin/bash
gcc -o test.o test.c json.c ./symbolstack/symbolstack.c
./test.o

