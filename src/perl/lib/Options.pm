package Options;

use strict;
use warnings;


# Usage header
our $help_header = "";

# Usage exit code
our $help_exit_code = 0;

# Error exit code
our $error_exit_code = 1;


sub usage(@) {
    my %opt_tpl = @_;

    print STDERR $help_header;

    print STDERR "Usage: $0 OPTIONS\n\nOPTIONS:\n";

    foreach my $opt (sort keys %opt_tpl) {
        my $opt_entry = $opt_tpl{$opt};

        my $desc     = $opt_entry->{desc};
        my $args_min = $opt_entry->{args_min};
        my $args_max = $opt_entry->{args_max};

        my $opt_args = "";

        if (not defined $args_max) {
            $opt_args = "<$args_min or more arguments>";
        }
        elsif ($args_max) {
            if ($args_min == $args_max) {
                $opt_args = "<$args_min argument" . ($args_min == 1 ? "" : "s") . ">";
            }
            else {
                $opt_args = "<$args_min up to $args_max arguments>";
            }
        }

        printf(STDERR "    --%s %s\n        %s\n", $opt, $opt_args, $desc);
    }

    print STDERR "    -- alone concludes options part of the command line\n" .
                 "       (may be omitted if and only if nothing follows)\n";

    exit $help_exit_code;
}


sub _unknown($) {
    my $opt = shift;

    print STDERR "Unknown option: --$opt\n" .
                 "Try to run with --help option\n";

    exit $error_exit_code;
}


sub _minArgs($$) {
    my ($opt, $min) = @_;

    print STDERR "Option --$opt requires at least $min argument" .
                 ($min > 1 ? "s" : "") .
                 "\n" .
                 "Try to run with --help option\n";

    exit $error_exit_code;
}


sub _maxArgs($$) {
    my ($opt, $max) = @_;

    print STDERR "Option --$opt " .
                 ($max ? "allows only $max argument"
                       : "does not allow argument") .
                 ($max == 1 ? "" : "s") .
                 "\n" .
                 "Try to run with --help option\n";

    exit $error_exit_code;
}


sub get(@) {
    my %opt_tpl = @_;

    # Set options
    my %opt;

    my $opt;

    while (@ARGV) {
        my $arg = shift(@ARGV);

        # New option or end of options
        if ($arg =~ /^--(.*)$/) {
            length($1) || last;

            $1 eq "help"
            and usage(%opt_tpl);

            exists $opt_tpl{$1}
            or _unknown($1);

            $opt = $opt{$1} = $opt_tpl{$1};

            $opt->{args} = [];
        }

        # Option argument
        elsif ($opt) {
            my $args = $opt->{args};

            push(@$args, $arg);
        }

        # No more options
        else {
            unshift(@ARGV, $arg);

            last;
        }
    }

    # Check min/max amount of option arguments
    my $opt_entry;

    while (($opt, $opt_entry) = each %opt) {
        my $args_min = $opt_entry->{args_min};
        my $args_max = $opt_entry->{args_max};
        my $args     = $opt_entry->{args};

        $args_min <= @$args
        or _minArgs($opt, $args_min);

        @$args <= $args_max
        or _maxArgs($opt, $args_max);
    }

    return %opt;
}


1;
