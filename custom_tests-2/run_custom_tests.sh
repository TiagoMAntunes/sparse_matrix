#!/bin/bash
### use: ./run_custom_tests.sh <program.c>

gcc -Wall -ansi $1 -o proj1

for i in {0..1000}
   do
     ./proj1 < test$i.in > test$i.mine.out; diff -q test$i.out test$i.mine.out
   done
