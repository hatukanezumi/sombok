#! /bin/sh

. tests/lb.sh

plan 1

dotest th th.al --no-complex-breaking

check_result

