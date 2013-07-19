#!/bin/sh

log_file="CHANGELOG"


# Change log preamble
cat > ${log_file} <<HERE
CTX Fryer Project Change Log
============================

The change log was generated on `date`.
Below, the committed changes appear in reversed order (most recent first).
Commit hashes are provided to simplify search for the particular commit;
just append the hash to the following URL:
https://github.com/vencik/ctx-fryer/commit/


HERE

# Transform git log into change log
format="%s%x00%H%x00%cN%x00%cE%x00%cd%x00%b%x01"

git log --pretty=format:"${format}" $* | awk '
BEGIN {
    FS = "\0";
    RS = "\1";
}

/^#changelog/ {
    break;
}

{
    ul = "";
    for (i = 0; i < length($1); ++i)
        ul = ul "-";

    print $1 "\n" ul "\n" \
        "Date: " $5 "\n" \
        "By:   " $3 "  <" $4 ">\n" \
        "Hash: " $2 "\n";

    if (length($6)) print $6;
}
' >> ${log_file}

# Commit the new change log
git commit --dry-run -m "#changelog" ${log_file}
