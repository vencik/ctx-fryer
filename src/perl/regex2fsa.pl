#!/usr/bin/perl

use strict;
use warnings;

use CTXFryer;
use Options;

use Logging qw(:all);

use FSA;
use FSA::Regex;


# Default log level
$Logging::threshold = "WARN";

# Usage header
$Options::help_header = <<HERE;
$CTXFryer::header
The script creates union FSA from FSA specifications in the following form
(one per each line):

<language> = /<reg-exp>/

<language> identifies the language accepted by the <reg-exp>.
It must begin with an alphabetic character or underscore and further consist
only of alphanumeric characters and/or underscores.

Lines beginning with hashmark and empty lines are ignored.

Moreover, the script allows for automatic unit testing of the union FSA.
UT cases specifications in the following form (one per each line) may be provided:

<language>* = ["']<word>["']

Language list items are separated by white spaces.
Single or double quotes may be used in the word spec. but they should match.

For each such entry, the script checks if FSA for all of the specified languages
accept the word and of course, if the union FSA accepts/rejects it
(no <language> spec. means that reject is expected).
If the specification does not _exactly_ match FSA result, an error is logged
and the script returns non-zero exit code when finished all the tests.

HERE

# Options
our %opt_tpl = (
    "dumpFSA" => {
        desc     => "Dump sub-FSA to <language>.xml file(s) in optional argument directory",
        args_min => 0,
        args_max => 1,
    },
    "input" => {
        desc     => "Read FSA specifications from file instead of std. input",
        args_min => 1,
        args_max => 1,
    },
    "log-file" => {
        desc     => "Log to file instead of std. error",
        args_min => 1,
        args_max => 1,
    },
    "log-level" => {
        desc     => "Set logging level",
        args_min => 1,
        args_max => 1,
    },
    "log-time" => {
        desc     => "Log time",
        args_min => 0,
        args_max => 0,
    },
    "log-process" => {
        desc     => "Log process",
        args_min => 0,
        args_max => 0,
    },
    "log-position" => {
        desc     => "Log code position",
        args_min => 0,
        args_max => 0,
    },
    "output" => {
        desc     => "Write FSA to file instead of std. output",
        args_min => 1,
        args_max => 1,
    },
);

# Default input
our $input = \*STDIN;


sub dumpFSA($@) {
    my ($fsa, $file_name) = @_;

    $fsa or return;

    my $xml = $fsa->xml();

    if ($file_name) {
        my $fh;

        open($fh, ">$file_name") or FATAL("Failed to open file $file_name for writing");

        print($fh $xml);

        close($fh);
    }
    else {
        print($xml);
    }
}


sub acceptsCmp($$) {
    my ($acc1, $acc2) = @_;

    my %acc1 = map(($_ => 1), @$acc1);
    my %acc2 = map(($_ => 1), @$acc2);

    my @lang1 = sort keys %acc1;
    my @lang2 = sort keys %acc2;

    my $cmp = @lang1 <=> @lang2;

    $cmp && return $cmp;

    for (my $i = 0; $i < @lang1; ++$i) {
        my $lang1 = $lang1[$i];
        my $lang2 = $lang2[$i];

        $cmp = $lang1 cmp $lang2;

        $cmp && last;
    }

    return $cmp;
}


my $exit_code = 0;

# Get command-line options
my %opt = Options::get(%opt_tpl);

if (@ARGV) {
    local $Options::help_exit_code = 1;

    Options::usage(%opt_tpl);
}

# Set log level
if ($opt{"log-level"}) {
    $Logging::threshold = $opt{"log-level"}->{args}->[0];
}

# Open log file
if ($opt{"log-file"}) {
    my $log_file = $opt{"log-file"}->{args}->[0];

    my $fh;

    open($fh, ">>$log_file") or FATAL("Can't log to $log_file");

    $Logging::file_handle = $fh;
}

# Log time
if ($opt{"log-time"}) {
    $Logging::log_time = 1;
}

# Log process
if ($opt{"log-process"}) {
    $Logging::log_process = 1;
}

# Log position
if ($opt{"log-position"}) {
    $Logging::log_position = 1;
}

INFO("Executed");

# Reg. exp. specifications
my %regex_spec;

# Unit tests
my %ut;
my @union_ut;

# Open input file
if ($opt{"input"}) {
    my $input_file = $opt{"input"}->{args}->[0];

    open($input, $input_file)
    or FATAL("Can't open file %s for reading", $input_file);

    INFO("Reading input from %s", $input_file);
}

# Parse input
while (<$input>) {
    /^\s*$/ && next;
    /^\s*#/ && next;

    # FSA definition
    if (/^\s*([A-Za-z_][A-Za-z_0-9]*)\s*=\s*\/(.*)\/([p]*)\s*$/) {
        my $language  = $1;
        my $regex_def = $2;
        my $regex_mod = $3;

        INFO("FSA definition: language: %s, reg. exp: /%s/%s", $language, $regex_def, $regex_mod);

        $regex_spec{$language} = { def => $regex_def, mod => $regex_mod };
    }

    # Unit test definition
    elsif (/^\s*([A-Za-z_][A-Za-z_0-9\s]*)=\s*(["'])(.*)(["'])\s*$/) {
        my @accepts = $1 ? split(/\s+/, $1) : ();
        my $bquote  = $2;
        my $word    = $3;
        my $equote  = $4;

        unless ($bquote eq $equote) {
            ERROR("Syntax error (quotes doesn't match) on UT line $.");

            $exit_code = 1;
        }

        INFO("UT definition: language(s): %s, word: \"%s\"", $word, \@accepts);

        foreach my $language (@accepts) {
            push(@{$ut{$language}}, $word);
        }

        push(@union_ut, { word => $word, accepts => \@accepts });
    }

    # Input parse error
    else {
        ERROR("Syntax error on line $.");

        $exit_code = 1;
    }
}

# Close input file
if ($opt{"input"}) {
    close($input);
}

# Create FSA
my %fsa;

while (my ($language, $regex_spec) = each %regex_spec) {
    my $regex_def = $regex_spec->{def};
    my $regex_mod = $regex_spec->{mod};

    INFO("Creating FSA::Regex for %s specification", $language);

    my $regex = new FSA::Regex($regex_def,
        "-language" => $language,
        "-greedy"   => index($regex_mod, "p") == -1,
    );

    unless ($regex) {
        ERROR("Failed to create FSA::Regex from specification /%s/%s", $regex_def, $regex_mod);

        $exit_code = 2;

        next;
    }

    DEBUG("%s regex. transformed into: %s", $language, $regex);

    INFO("Transforming the regular expression to FSA");

    my $fsa = $regex->fsa();

    unless ($fsa) {
        ERROR("Failed to create FSA from %s", $regex);

        $exit_code = 2;

        next;
    }

    DEBUG("%s-accepting FSA: %s", $language, $fsa);

    INFO("Minimising the FSA");

    $fsa = $fsa->minimise();

    DEBUG("%s-accepting minimal FSA: %s", $language, $fsa);

    $fsa{$language} = $fsa;
}

# Dump FSA
if ($opt{"dumpFSA"}) {
    my $dir = $opt{"dumpFSA"}->{args}->[0] || "";

    $dir and $dir .= "/";

    while (my ($language, $fsa) = each %fsa) {
        my $file = $dir . "$language.xml";

        INFO("Dumping %s-accepting FSA to %s", $language, $file);

        dumpFSA($fsa, $file);
    }
}

# Run FSA unit tests
while (my ($language, $words) = each %ut) {
    my $fsa = $fsa{$language};

    unless ($fsa) {
        ERROR("No FSA defined for language %s", $language);

        $exit_code = 2;

        next;
    }

    foreach my $word (@$words) {
        INFO("Unit-testing word \"%s\"", $word);

        my @accepts = $fsa->accepts($word);

        unless (acceptsCmp(\@accepts, [ $language ]) == 0) {
            ERROR("UT for word \"%s\" failed for language %s", $word, $language);
            ERROR("The language FSA accepts %s", \@accepts);

            $exit_code = 2;
        }
    }
}

# Create union FSA
my $fsa;

if (%fsa) {
    INFO("Creating union FSA (may take some time, be patient)");

    $fsa = FSA::union(values %fsa);

    if ($fsa) {
        INFO("Minimising union FSA (may take even longer, be patient)");

        my $min = $fsa->minimise();

        if ($min) {
            $fsa = $min;

            DEBUG("Result: %s", $fsa);

            INFO("Minimal union FSA sucessfully created");
        }
        else {
            ERROR("Failed to minimise %s", $fsa);

            $exit_code = 2;
        }
    }
    else {
        ERROR("Failed to create union FSA");

        $exit_code = 2;
    }
}

# Run union FSA unit tests
foreach my $union_ut (@union_ut) {
    my $word    = $union_ut->{word};
    my $accepts = $union_ut->{accepts};

    INFO("Union UT: checking word \"%s\"", $word);

    my @union_accepts = $fsa->accepts($word);

    unless (acceptsCmp(\@union_accepts, $accepts) == 0) {
        ERROR("Union UT for word \"%s\") failed", $word);
        ERROR("UT specifies accepts %s", $accepts);
        ERROR("Union FSA accepts %s", \@union_accepts);

        $exit_code = 2;
    }
}

# Output
my $output = $opt{"output"};

if ($output) {
    my $output_file = $output->{args}->[0];

    if ($output_file) {
        dumpFSA($fsa, $output_file);

        INFO("Union FSA dumped to %s", $output_file);
    }
}
else {
    dumpFSA($fsa);
}

# Close log file
if ($opt{"log-file"}) {
    close($Logging::file_handle);
}

INFO("Finished %s", $exit_code ? "with error(s)" : "successfully :-)");

exit $exit_code;
