#! /bin/sh

. tests/lb.sh

plan 8

for lang in ar el fr ja ja-a ko ru zh; do
    dotest $lang $lang
done

check_result

