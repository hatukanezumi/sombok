# -*- bash -*-

OPTIONS="\
--charmax 998 \
--colmin 0 \
--colmax 76 \
--format-func SIMPLE \
--no-hangul-as-al \
--legacy-cm \
--newline \\n \
--sizing-func UAX11 \
--urgent-func NONE \
--virama-as-joiner \
"

plan () {
    PLANNED=$1
    SUCCESS=0
}

dotest () {
    in=$1
    shift
    if [ "$in" = "negate" ]; then
	negate=yes
	in=$1
	shift
    fi
    out=$1
    shift

    ./sombok $OPTIONS $* -o tmp.out test-data/$in.in
    rc=$?
    if [ $rc = 0 ]; then
	if [ -e test-data/$out.out ]; then
	    cmp tmp.out test-data/$out.out
	    rc=$?
	else
	    cat tmp.out > test-data/$out.xxx
	    rc=255
	fi
    fi
    rm -f tmp.out

    if [ "$negate" = "yes" ]; then
	if [ $rc = 0 ]; then
	    rc=255
	else
	    rc=0
	fi
    fi
    if [ $rc = 0 ]; then
	SUCCESS=`expr $SUCCESS + 1`
    fi
}

check_result () {
    echo "$SUCCESS of $PLANNED subtests passed."

    if [ "$PLANNED" = "$SUCCESS" ]; then
	exit 0
    else
	exit 1
    fi
}

