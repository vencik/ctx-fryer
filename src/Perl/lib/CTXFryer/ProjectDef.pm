package CTXFryer::ProjectDef;

use strict;
use warnings;

use vars qw(
    @ISA
    @EXPORT_OK
    %EXPORT_TAGS

    %escape_chars

    $skip_metainfo
    $skip_grammar
    $skip_unit_test
    );


# Project definition directive scopes
use constant GLOBAL_SCOPE      => 0;
use constant META_SCOPE        => 1;

use constant TERMINALS_SCOPE   => 2;
use constant RULES_SCOPE       => 3;
use constant ATTRIBUTES_SCOPE  => 4;

use constant UNIT_TEST_SCOPE   => 9;


require Exporter;

@ISA = qw(CTXFryer::Base Exporter);

@EXPORT_OK = qw(
    $skip_metainfo
    $skip_grammar
    $skip_unit_test

    unescape
    str2bool
    str2list
    );

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
    );

use CTXFryer::ProjectDef::MetaInfo;
use CTXFryer::ProjectDef::Grammar::TerminalSymbols;
use CTXFryer::ProjectDef::Grammar::Rules;
use CTXFryer::ProjectDef::Grammar::Attributes;
use CTXFryer::ProjectDef::UnitTest;

use CTXFryer::Logging qw(:all);

use File::Basename;


# Escape sequences
%escape_chars = (
    a => "\a",
    b => "\b",
    n => "\n",
    r => "\r",

    # Backslash itself
    '\\' => '\\'
);


# Unescape string containing escape sequences
sub unescape($) {
    my $str = shift;

    while (my ($ech, $esc) = each %escape_chars) {
        $ech = quotemeta($ech);

        $str =~ s/\\$ech/$esc/g;
    }

    return $str;
}


# Boolean sematics of well-known strings
sub str2bool($) {
    my $str = shift;

    no warnings;

    "no"    eq $str and return 0;
    "false" eq $str and return 0;
    "off"   eq $str and return 0;
    "yes"   eq $str and return 1;
    "true"  eq $str and return 1;
    "on"    eq $str and return 1;

    0 == $str and return 0;
    1 == $str and return 1;

    return 0;
}


# Split string representing a list to items
sub str2list($$) {
    my ($sep, $str) = @_;

    my $sep_re = "[" . quotemeta($sep) . "]+";

    # Strip leading separators
    $str =~ s/^$sep_re//;

    # Strip trailing separators
    $str =~ s/$sep_re$//;

    return split($sep, $str);
}


# Get absolute path to a file
sub file2abspath($) {
    my ($file, $path) = fileparse(shift);

    if ($path eq "." || $path eq "./") {
        $path = $ENV{PWD};
    }
    elsif ($path !~ /^\//) {
        $path = $ENV{PWD} . "/" . $path;
    }

    return $path;
}


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $input_file = shift;

    my $input;

    if ($input_file) {
        open($input, $input_file)
        or FATAL("Can't open file $input_file for reading");
    }
    else {
        $input = \*STDIN;

        $input_file = "standard input";
    }

    INFO("Reading project definition from $input_file");

    my $input_file_path = file2abspath($input_file);

    DEBUG("Project definition file resides in $input_file_path");

    my $metainfo  = ($skip_metainfo  ? undef : new CTXFryer::ProjectDef::MetaInfo($input_file_path));
    my $terminals = ($skip_grammar   ? undef : new CTXFryer::ProjectDef::Grammar::TerminalSymbols);
    my $rules     = ($skip_grammar   ? undef : new CTXFryer::ProjectDef::Grammar::Rules);
    my $attrs     = ($skip_grammar   ? undef : new CTXFryer::ProjectDef::Grammar::Attributes);
    my $unit_test = ($skip_unit_test ? undef : new CTXFryer::ProjectDef::UnitTest);

    my $scope     = GLOBAL_SCOPE;
    my $error_cnt = 0;

    while (<$input>) {
        chomp;

        # Strip comments
        s/#.*$//;

        # Skip empty line
        /^\s*$/ && next;

        my $pos = "line $.";

        # Meta-information start
        if (GLOBAL_SCOPE == $scope && /^\s*Meta\s*$/i) {
            $scope = META_SCOPE;
        }

        # Meta-information end
        elsif (META_SCOPE == $scope && /^\s*MetaEnd\s*$/i) {
            $scope = GLOBAL_SCOPE;
        }

        # Parse metainfo
        elsif (META_SCOPE == $scope) {
            $skip_metainfo or $error_cnt += $metainfo->parse($_, $pos);
        }

        # Terminals definitions start
        elsif (GLOBAL_SCOPE == $scope && /^\s*TerminalSymbols\s*$/i) {
            $scope = TERMINALS_SCOPE;
        }

        # Terminals definitions end
        elsif (TERMINALS_SCOPE == $scope && /^\s*TerminalSymbolsEnd\s*$/i) {
            $scope = GLOBAL_SCOPE;
        }

        # Parse terminals
        elsif (TERMINALS_SCOPE == $scope) {
            $skip_grammar or $error_cnt += $terminals->parse($_, $pos);
        }

        # Rules definitions start
        elsif (GLOBAL_SCOPE == $scope && /^\s*Rules\s*$/i) {
            $scope = RULES_SCOPE;
        }

        # Rules definitions end
        elsif (RULES_SCOPE == $scope && /^\s*RulesEnd\s*$/i) {
            $scope = GLOBAL_SCOPE;
        }

        # Parse rules
        elsif (RULES_SCOPE == $scope) {
            $skip_grammar or $error_cnt += $rules->parse($_, $pos);
        }

        # Attributes definition start
        elsif (GLOBAL_SCOPE == $scope && /^\s*Attributes\s*$/i) {
            $scope = ATTRIBUTES_SCOPE;
        }

        # Attributes definitions end
        elsif (ATTRIBUTES_SCOPE == $scope && /^\s*AttributesEnd\s*$/i) {
            $scope = GLOBAL_SCOPE;
        }

        # Parse attributes
        elsif (ATTRIBUTES_SCOPE == $scope) {
            $skip_grammar or $error_cnt += $attrs->parse($_, $pos);
        }

        # Unit test start
        elsif (GLOBAL_SCOPE == $scope && /^\s*UnitTest\s*$/i) {
            $scope = UNIT_TEST_SCOPE;
        }

        # Unit test end
        elsif (UNIT_TEST_SCOPE == $scope && /^\s*UnitTestEnd\s*$/i) {
            $scope = GLOBAL_SCOPE;
        }

        # Parse unit test
        elsif (UNIT_TEST_SCOPE == $scope) {
            $skip_unit_test or $error_cnt += $unit_test->parse($_, $pos);
        }

        # Parse error
        else {
            ERROR("Parse error on line #%d: \"%s\"", $., $_);

            ++$error_cnt;
        }
    }

    $input_file && close($input);

    $error_cnt
    and FATAL("%d syntax errors spotted in project definition", $error_cnt);

    INFO("Project definition successfully parsed");

    my $this = {
        metainfo  => $metainfo,
        terminals => $terminals,
        rules     => $rules,
        attrs     => $attrs,
        unit_test => $unit_test,
    };

    return bless($this, $class);
}


sub metaInfo($) {
    my $this = shift;

    my $metainfo = $this->{metainfo};

    $metainfo && return $metainfo;

    FATAL("Project meta information collection was disabled");
}


sub terminalSymbols($) {
    my $this = shift;

    my $terminals = $this->{terminals};

    $terminals && return $terminals;

    FATAL("Project grammar definition collection was disabled");
}


sub rules($) {
    my $this = shift;

    my $rules = $this->{rules};

    $rules && return $rules;

    FATAL("Project grammar definition collection was disabled");
}


sub attributes($) {
    my $this = shift;

    my $attrs = $this->{attrs};

    $attrs && return $attrs;

    FATAL("Project grammar definition collection was disabled");
}


sub unitTest($) {
    my $this = shift;

    my $unit_test = $this->{unit_test};

    $unit_test && return $unit_test;

    FATAL("Project unit test definition collection was disabled");
}


1;
