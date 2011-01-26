#! /bin/sh

. tests/lb.sh

#plan 6
plan 4

#for lang in fr ja; do
#    dotest $lang $lang.format --format-func \
#	'case "$1" in
#	sot|sop|sol)
#	    echo -n "$1>"; cat;;
#	eol|eop|eot)
#	    echo "<$1";;
#	*)
#	    cat;;
#	esac'
#done
for lang in fr ko; do
    dotest $lang "$lang.newline" --format-func NEWLINE
done
for lang in fr ko; do
    dotest $lang $lang.newline --format-func TRIM
done

check_result

