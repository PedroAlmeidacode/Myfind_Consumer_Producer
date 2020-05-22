#!/bin/bash

gcc -c main.c               
gcc -c lib_util.c  
gcc -o main main.o lib_util.o  -lpthread

