#!/bin/sh

this=`basename $0`
date=`date`

log_file="CHANGELOG"
doc_file="doc/changelog.txt"

commit_url_prefix="https://github.com/vencik/ctx-fryer/commit/"


# Change log preamble
cat > ${log_file} <<HERE
CTX Fryer Project Change Log
============================

The change log was generated on ${date}
by the ${this} script.
Below, the committed changes appear in reversed order (most recent first).
Commit hashes are provided to simplify search for the particular commit;
just append the hash to the following URL:
${commit_url_prefix}

HERE

# Change log (for project documentation) preamble
cat > ${doc_file} <<HERE
CTX Fryer Project Change Log
----------------------------

The change log was generated on ${date}.

Below, the committed changes appear in reversed order (most recent first).

HERE


# Transform git log into change log
format="%s%x00%H%x00%cN%x00%cE%x00%cd%x00%b%x01"

git log --pretty=format:"${format}" $* | awk '
BEGIN {
    FS = "\0";
    RS = "\1";
}

/^\n?#changelog/ {
    next;
}

{
    sub("^\n", "");

    # Plain text file output
    ul = "";
    for (i = 0; i < length($1); ++i)
        ul = ul "-";

    print "\n" $1 "\n" ul "\n" \
        "Date: " $5 "\n" \
        "By:   " $3 "  <" $4 ">\n" \
        "Hash: " $2 "\n" \
        >> "'${log_file}'";

    if (length($6)) print $6 >> "'${log_file}'";

    # Project documentation output
    heading = "link:'${commit_url_prefix}'" $2 "[" $1 "]";

    ul = "";
    for (i = 0; i < length(heading); ++i)
        ul = ul "*";

    print "\n." heading "\n*" ul "\n" \
        $5 " by '\''" $3 "'\''\n\n_Hash_: " $2 \
        >> "'${doc_file}'";

    if (length($6)) {
        text = $6;

        sub("\n$", "", text);
        gsub("\n", "\n ", text);

        print "\n " text >> "'${doc_file}'";
    }

    print "*" ul >> "'${doc_file}'";
}
'

# Commit the new change log
git commit -m "#changelog" ${log_file} ${doc_file}
