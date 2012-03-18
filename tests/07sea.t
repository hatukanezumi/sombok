#! /bin/sh

. tests/lb.sh

if [ "`./sombok --sea-support`" = "none" ]; then
    echo "SA word segmentation not supported."
    exit 77
fi
plan 1

dotest th th --complex-breaking

check_result

