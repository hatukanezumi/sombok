#! /bin/sh

. tests/lb.sh

#plan 2
plan 1

dotest ja-k ja-k --colmax 72
#XXXdotest ja-k ja-k.ns --tailor-lb KANA_NONSTARTERS=ID --colmax 72

check_result

