#!/bin/bash
gcc -o test.o test.c ../json.c ../modules/symbolstack/symbolstack.c
./test.o

