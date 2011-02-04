#! /bin/sh

. tests/lb.sh

plan 2

dotest uri uri.break --colmax 1 --prep-func BREAKURI
dotest uri uri.nonbreak --colmax 1 --prep-func NONBREAKURI

check_result

