#! /bin/sh

. tests/lb.sh

plan 2

dotest ko ko.al --hangul-as-al
dotest amitagyong amitagyong

check_result
