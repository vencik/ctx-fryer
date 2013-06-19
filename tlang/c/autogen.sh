#!/bin/sh

# GNU autotools pre-build script


info="The $0 script is used to create the GNU autotools based
build configuration system of the package.
If you build from a source package or tarball, you shouldn't
necessarily need to run it.
If you obtained the source from the versioning system, and especially
if you develop it and make non-trivial changes (like adding calls
to previously unused library functions, new dependencies etc) you need
to run it to produce correct configure script for the package.
GNU autotools (aclocal, automake and autoconf) are required for that.
Also note that libtool is used for libraries creation.
With configure script created, you should be able to build the software
by executing the usual ./configure && make && make install triplet.
"


log () {
    echo "$*"
}


quit () {
    exit_code=$1

    test -n "$exit_code" || exit 0

    shift

    if test $exit_code -eq 0; then
        log "$*"
    else
        log "Error:" "$*" >&2
    fi

    exit $exit_code
}


usage_and_quit () {
    exit_code=$1; test -n "$exit_code" || exit_code=0

    msg="$info"

    if test "$exit_code" -gt 0; then
        msg="$2\n\n$msg"
    fi

    quit $exit_code "$msg"
}


# Some preliminary checks
test $# -gt 0 && usage_and_quit

which libtoolize aclocal automake autoconf >/dev/null \
|| usage_and_quit 1 "GNU autotools and libtool are required to run the script"


# Configure build system
run_libtoolize () {
    log "Running libtoolize to prepare libtool usage..."
    libtoolize || quit 2 "libtoolize failed"
}

run_aclocal () {
    log "Running aclocal to create autoconf macra..."
    aclocal || quit 3 "aclocal failed"
}

run_automake () {
    log "Running automake to generate makefile templates for configure script..."
    automake --add-missing || quit 4 "automake failed"
}

run_autoconf () {
    log "Running autoconf to generate configure script..."
    autoconf || quit 5 "autoconf failed"
}


run_libtoolize
run_aclocal
run_automake
run_autoconf

log "Package build configuration system was successfully created"
quit
