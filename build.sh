#!/bin/bash
gcc -fPIC -shared -o cjson.so ../cjson.c ../lib/stack.so

