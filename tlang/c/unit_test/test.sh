#!/bin/sh

# valgrind call
valgrind="valgrind --leak-check=full --show-reachable=yes --trace-children=yes --track-fds=yes"


# Quit
quit () {
    exit_code=$1; test -n "$exit_code" || exit_code=0
    msg="$2"

    if test $exit_code -gt 0; then
        echo "$msg" >&2
    else
        echo "$msg"
    fi

    exit $exit_code
}


# Execute unit test
run_test_impl () {
    id="$1"
    prolog="$2"
    bin="$3"
    opts="$4"
    inf="$5"
    outf="$6"

    test -n "$bin" || quit 127 "INTERNAL ERROR: invalid arguments for run_test_impl: $*"

    log="$bin.log"

    test -n "$inf"  && inf="< $inf"
    test -n "$outf" && outf="| diff - $outf"

    cmd="$prolog ./$bin $opts $inf 2> $log $outf"

    echo "Running test \"$id\": $cmd"

    exec "$cmd"

    cmd_exit_code=$?

    test $cmd_exit_code -gt 0 && quit 1 "Test $id FAILED (exit code $cmd_exit_code)"

    echo "Test \"$id\" PASSED"
}

run_test () {
    run_test_impl "$1" "" "$2" "$3" "$4" "$5"
}

run_test_valgrind () {
    run_test_impl "$1 (under valgrind)" "$valgrind --log-file=$2.%p.valgrind.log" "$2" "$3" "$4" "$5"
}


# Resolve runmode
run_under_valgrind="false"

test -n "$USE_VALGRIND" && run_under_valgrind="true"


# Execute unit tests
if test "$run_under_valgrind" = "false"; then
    run_test "Source file"      test.srcfile          "-l4 test.srcfile.c"
    run_test "Lexical analyser" test.lexical_analyser "-cl4" test.lexical_analyser.input test.lexical_analyser.output
    run_test "Syntax analyser"  test.syntax_analyser  "-l4"  test.syntax_analyser.input  test.syntax_analyser.output
else
    which valgrind >/dev/null || quit 2 "valgrind not found (is it installed?)"

    run_test_valgrind "Source file"      test.srcfile          "-l4 test.srcfile.c"
    run_test_valgrind "Lexical analyser" test.lexical_analyser "-cl4" test.lexical_analyser.input
    run_test_valgrind "Syntax analyser"  test.syntax_analyser  "-l4"  test.syntax_analyser.input
fi

quit 0 "All unit tests PASSED"
