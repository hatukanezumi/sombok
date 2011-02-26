#! /bin/sh

. tests/lb.sh

plan 2

dotest fr fr.ea --context EASTASIAN
dotest fr fr --context EASTASIAN \
    --eawidth \
    C6=N,D0=N,D8=N,DE-E1=N,E6=N,E8-EA=N,EC-ED=N,F0=N,F2-F3=N,F8-FA=N,FC=N,FE=N

check_result

