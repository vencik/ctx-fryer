#!/bin/sh

echo "Libtoolizing..."
libtoolize --copy || exit $?

echo "Running aclocal..."
aclocal || exit $?

echo "Running autoheader..."
autoheader || exit $?

echo "Running automake..."
automake --add-missing --copy || exit $?

echo "Running autoconf..."
autoconf || exit $?
