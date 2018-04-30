#!/bin/bash
### use: ./run_custom_tests.sh <program.c>

gcc -Wall -ansi $1 -o proj1

for i in {0..2500..1}
   do
     ./proj1 < test$i.in > test$i.mine.out; diff -q test$i.out test$i.mine.out
   done
