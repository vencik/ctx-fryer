#!/bin/sh

# Usage
usage() {
    cat >&2 <<HERE
Usage: `basename $0` <command> [<command-specific options>]

Commands available:
    create_project    Creates project

See the project documentation for more info.

HERE
}

# dirname with symlink dereferencing
dirnameTarget() {
    local file="$1"
    local dir=`dirname "$file"`

    # $file is a symlink
    while test -L $file; do
        file=`readlink $file`

        local target_dir=`dirname "$file"`

        # Symlink with absolute path
        if expr substr "$target_dir" 1 1 >/dev/null; then
            dir="$target_dir"

        # Relative symlink to different directory
        elif test "$target_dir" != "."; then
            dir="$dir/$target_dir"
        fi
    done

    echo "$dir"
}

# Absolute path
this=`which "$0"`
dir=`dirnameTarget "$this"`
expr substr "$dir" 1 1 = '/' >/dev/null || dir="`pwd`/$dir"

# Execute command-specific script with full path and PATH augmented
cmd="$1"

if test -z "$cmd"; then
    usage
    exit 1
fi

shift;

script="undefined"

case "$cmd" in
    create_project)
        script="create_project.sh"
        ;;

    make_project)
        script="make_project.sh"
        ;;

    *)
        echo "Unknown command: $cmd" >&2
        usage
        exit 2
esac

script="$dir/$script"

if test ! -e "$script"; then
    echo "INTERNAL ERROR: $script wasn't found" >&2
    exit 127
fi

export PATH="$PATH:$dir"
sh "$script" $*
