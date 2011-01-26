#! /bin/sh

. tests/lb.sh

plan 5

dotest ecclesiazusae ecclesiazusae
dotest ecclesiazusae ecclesiazusae.ColumnsMax --urgent-func FORCE
dotest ecclesiazusae ecclesiazusae.CharactersMax --charmax 79
dotest ecclesiazusae ecclesiazusae.ColumnsMin \
  --colmin 7 --colmax 66 --urgent-func FORCE

dotest negate ecclesiazusae ecclesiazusae --urgent-func ABORT

check_result

