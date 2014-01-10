#!/bin/sh

expressions="
4.3+89.1
1.234*6.54
"

fail() {
    echo $* >&2
    cat >&2 <<HERE

================
Unit test FAILED
================

Please post bugreport

HERE
    exit 1
}

for x in $expressions; do
    res=`echo "$x" | bc -l`

    echo "Checking expression $x == $res"

    echo "$x" | ./calc | xargs expr $res = >/dev/null || fail "Failed to compute $x"
done

cat <<HERE

================
Unit test PASSED
================

HERE
exit 0
