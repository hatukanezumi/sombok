#! /bin/sh

. tests/lb.sh

plan 12

for lang in ar el fr he ja ja-a ko ru sa vi vi-decomp zh; do
    dotest $lang $lang
done

check_result

