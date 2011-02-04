#! /bin/sh

. tests/lb.sh

plan 6

for lang in fr ja; do
    ./sombok $OPTIONS --format-func '
	( case "$1" in
	sot | sop | sol)
	    echo -n "    $1>"
	    cat
	    ;;
	eol | eop | eot)
	    echo "<$1"
	    ;;
	*)
	    cat
	    ;;
	esac ) | dd 2>/dev/null' \
	-o tmp.out test-data/$lang.in
    cmp tmp.out test-data/$lang.format.out
    if [ "$?" = "0" ]; then
	SUCCESS=`expr $SUCCESS + 1`
    fi
    rm -f tmp.out
done
for lang in fr ko; do
    dotest $lang "$lang.newline" --format-func NEWLINE
done
for lang in fr ko; do
    dotest $lang $lang.newline --format-func TRIM
done

check_result

