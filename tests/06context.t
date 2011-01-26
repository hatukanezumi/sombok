#! /bin/sh

. tests/lb.sh

#plan 2
plan 1

dotest fr fr.ea --context EASTASIAN
#dotest fr fr --context EASTASIAN \
#    --tailor-ea AMBIGUOUS_ALPHABETICS=N

check_result

