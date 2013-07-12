#!/bin/sh

# Use autotools to prepare build system for development checkouts
if test -d ./.git; then
    ./autogen.sh || exit $?
fi

# Configure build
./configure $* || exit $?

# Make it
make || exit $?

# Installation might need superuser priviledges...
cat <<HERE

------------------------------------------------------------------------------
The package was built successfully
Proceed with installation by running "make install"
You might need to use sudo or switch to root if doing system-wide installation
------------------------------------------------------------------------------

HERE
