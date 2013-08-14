#!/bin/sh

cfg2ut=ctx-fryer-cfg2ut
def_file=../../def_file
test_bin=./test
test_args="-l4"


# Unit test can't be done
unavailable() {
    reason="$*"

    cat >&2 <<HERE
!!! WARNING !!!

The unit test can't be executed for the following reason:
${reason}

It isn't absolutely necessary to execute the unit test, however,
it's generally good idea to test code as thoroughly as possible.
Please consider removing the reason why the unit test couldn't
be executed and run it by doing "make check", again.
If you didn't define any unit test in your project definition file,
consider doing so (consult CTX Fryer documentation if you don't
know how).

HERE

    # Impossibility of UT execution is not an error
    exit 0
}


# Failure report
failed() {
    reason="$*"

    cat >&2 <<HERE
!!! FAILED !!!

The unit test failed for the following reason:
${reason}

Please double check the unit test validity; if the UT definition is OK,
please check any code that you might have provided in form of attributes
evaluators and/or destructors.
If you are sure that the problem isn't in your own code, please issue
a bug report.  Don't forget to include the test program log and ideally,
your project definition file (unless it's classified).  Remember that
re-invoking the unit test failure is the best way to find what's wrong.
HERE

    exit 1
}


# Check whether we can run the UT
which ${cfg2ut} >/dev/null || \
    unavailable "${cfg2ut} not found (is ctx-fryer installed?)"

# Sanity checks
test -f ${def_file} || \
    failed "project definition file ${def_file} not found"

test -x ${test_bin} || \
    failed "testing binary ${test_bin} doesn't exist or not executable"

# Execute UT
${cfg2ut} ${def_file} | ${test_bin} ${test_args} || \
    failed "unit test program terminated with exit code $?."

cat >&2 <<HERE
Unit test passed  :-)

All unit test cases defined in your project definition file passed OK.
If you didn't define any, please consider doing so; it's not that difficult
and unit testing code automatically is ALWAYS good idea.
Of course, if you have written your own auto-UTs, you may ignore this.

HERE

exit 0
