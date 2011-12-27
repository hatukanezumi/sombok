#! /bin/sh

. tests/lb.sh

plan 2

dotest ko ko.al --hangul-as-al
dotest amitagyong amitagyong --eawidth 302E=Z,302F=Z

check_result
