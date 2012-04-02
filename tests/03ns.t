#! /bin/sh

. tests/lb.sh

plan 3

dotest ja-k ja-k --colmax 72
dotest ja-k ja-k.ns --lbclass 3083=ID,3063=ID,3087=ID,3085=ID --colmax 72
dotest ja-k ja-k.ns --nonstarter LOOSE --colmax 72

check_result

