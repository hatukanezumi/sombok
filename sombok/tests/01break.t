#! /bin/sh

. tests/lb.sh

plan 10

for lang in ar el fr ja ja-a ko ru vi vi-decomp zh; do
    dotest $lang $lang
done

check_result

