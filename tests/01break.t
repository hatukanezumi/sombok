#! /bin/sh

. tests/lb.sh

plan 13

for lang in ar el fr he ja ja-a ko ko-decomp ru sa vi vi-decomp zh; do
    dotest $lang $lang
done

check_result

