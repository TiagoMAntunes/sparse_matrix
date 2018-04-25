#!/bin/bash
### use: ./run_all <program.c>

gcc $1 -o proj1
./proj1 < test01.in > test01.mine.out; diff -q test01.out test01.mine.out
./proj1 < test02.in > test02.mine.out; diff -q test02.out test02.mine.out
./proj1 < test03.in > test03.mine.out; diff -q test03.out test03.mine.out
./proj1 < test04.in > test04.mine.out; diff -q test04.out test04.mine.out
./proj1 < test05.in > test05.mine.out; diff -q test05.out test05.mine.out
./proj1 < test06.in > test06.mine.out; diff -q test06.out test06.mine.out
./proj1 < test07.in > test07.mine.out; diff -q test07.out test07.mine.out
./proj1 < test08.in > test08.mine.out; diff -q test08.out test08.mine.out
./proj1 < test09.in > test09.mine.out; diff -q test09.out test09.mine.out
./proj1 < test10.in > test10.mine.out; diff -q test10.out test10.mine.out
./proj1 good.mine.mat < test11.in > test11.mine.out; diff -q test11.out test11.mine.out