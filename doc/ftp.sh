#!/bin/sh

host=""
username=""
passwd=""
root=""


usage() {
    cat <<HERE
Usage: $0 [OPTIONS] [<path>]*

Recursively uploads files under the specified path to an FTP server.

OPTIONS:
    -h or --help                       Display this help and exit
    -H or --host   <hostname>          Set FTP host
    -u or --user   <username>          Set username for the host
    -p or --passwd <password>          Set password for the user
    -r or --root   <path>              Remote tree root

Unless the credentials are specified, the script will ask for them.
Host specification may be enough if there's matching entry in your ~/.netrc
ftp configuration file (see man netrc).
If the remote tree root is specified, the script changes to it before doing
anything else.

HERE
}


usage_and_exit() {
    usage
    exit $1
}


# Parse & unify command line arguments
options=`getopt -o "h,H:,u:,p:,r:" -l "help,host:,user:,passwd:,root:" -- $*`

test $? = 0 || usage_and_exit 1 >&2

# Process options
set -- $options

while test "$1" != "--"; do
    case "$1" in
    -h|--help)
        # Print usage
        usage_and_exit 0
        ;;

    -H|--host)
        # Host
        shift; eval host="$1"
        ;;

    -u|--user)
        # Username
        shift; eval username="$1"
        ;;

    -p|--passwd)
        # Password
        shift; eval passwd="$1"
        ;;

    -r|--root)
        # Remote tree root
        shift; eval root="$1"
        ;;
    esac

    # Next option
    shift
done

# Shift the final "--"
shift

# Read FTP host unless provided on cmd-line
if test -z "$host"; then
    echo -n "Host: "
    read host
fi

# Set awk routines to parse username and password from .netrc
awk_init="
BEGIN {
    out = 0;
}

/^[ \\t]*machine[ \\t]*$host[ \\t]*\$/ {
    out = 1;
    next;
}
"'
/^[ \t]*machine[ \t]*/ {
    out = 0;
}
'

awk_get_username='
/^[ \t]*login[ \t]*/ {
    if (out)
        print "username=\"" $2 "\";";
}
'

awk_get_passwd='
/^[ \t]*password[ \t]*/ {
    if (out)
        print "passwd=\"" $2 "\";";
}
'


# Try to get credentials from ~/.netrc if exists
if test -z "$username" -a -f ~/.netrc; then
    eval `awk "$awk_init $awk_get_username" ~/.netrc`
fi
if test -z "$passwd" -a -f ~/.netrc; then
    eval `awk "$awk_init $awk_get_passwd" ~/.netrc`
fi

# Read credentials unless provided on cmd-line
if test -z "$username"; then
    echo -n "Username for ftp://$host: "
    read username
fi
if test -z "$passwd"; then
    echo -n "Password for ftp://$username@$host: "
    tty_modes=`stty -g`
    stty -echo
    read passwd
    stty $tty_modes
    echo
fi

# Final credentials checks
ok=true

if test -z "$host"; then
    echo "Host not specified" >&2
    ok=false
fi
if test -z "$passwd"; then
    echo "Username not specified" >&2
    ok=false
fi
if test -z "$username"; then
    echo "Password not specified" >&2
    ok=false
fi


# Error
if test "$ok" = "false"; then
    usage_and_exit 2 >&2
fi


# Create FTP batch
put_all_under() {
    dir="$1"

    dir_list=""

    for f in `ls -d "$dir"/*`; do
        if test -d "$f"; then
            dir_list="${dir_list} $f"
        else
            echo "put `basename $f`"
        fi
    done

    for d in $dir_list; do
        bn_d=`basename $d`
        echo "mkdir $bn_d\ncd $bn_d\nlcd $bn_d"
        put_all_under "$d"
        echo "cd ..\nlcd .."
    done
}


batch="bin\n"

for dir in `echo $root | sed -e 's/\// /g'`; do
    batch="${batch}mkdir $dir\ncd $dir\n"
done

while test $# -gt 0; do
    dir=`eval echo $1`
    tree=`eval put_all_under $dir`
    batch="${batch}lcd $dir\n${tree}"
    shift
done

batch="${batch}\nbye"

# Run FTP batch (from stdin, echoing commands)
(
    echo "quote USER $username"
    echo "quote PASS $passwd"
    echo "$batch" | awk '{ print "!echo \"" $0 "\""; print $0 }'
) | ftp -nip "$host"
