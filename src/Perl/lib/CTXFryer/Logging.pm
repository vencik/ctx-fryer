package CTXFryer::Logging;

use strict;
use warnings;

use vars qw(@ISA @EXPORT_OK %EXPORT_TAGS);

require Exporter;


@ISA = qw(Exporter);

@EXPORT_OK = qw(
    FATAL
    ERROR
    WARN
    INFO
    DEBUG
    DEBUX
);

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
);


our $log_time     = 0;
our $log_process  = 0;
our $log_position = 0;
our $depth        = 0;
our $file_handle  = \*STDERR;
our $threshold    = "ERROR";


sub level2num($) {
    our %level = (
        FATAL  => 0,
        ERROR  => 1,
        WARN   => 2,
        INFO   => 3,
        DEBUG  => 4,
    );

    my $level = shift;

    my $num;

    if ($level =~ /^DEBUG(\d+)$/) {
        $num = $level{DEBUG} + $1;
    }
    elsif ($level =~ /^DEBUX(\d+)$/) {
        $num = $level{DEBUG} + $1;
    }
    else {
        $num = $level{$level};
    }

    defined $num or die "Unrecognised log level \"$level\"";

    return $num;
}


sub _arg2str {
    return map {
        my $str = "<undef>";

        if (defined $_) {
            $str = $_;

            my $ref = ref $_;

            if ($ref) {
                if ($ref eq "ARRAY") {
                    my @array = _arg2str(@$_);

                    local $" = ", ";

                    $str = "(@array)";
                }
                elsif ($ref eq "HASH") {
                    my %hash = _arg2str(%$_);

                    my @key_val;

                    while (my ($key, $val) = each %hash) {
                        push(@key_val, $key . " => " . $val);
                    }

                    local $" = ", ";

                    $str = "(@key_val)";
                }
                else {
                    $str = "$_";
                }
            }
        }

        $str;

    } @_;
}


sub _basename($) {
    my $file = shift;

    defined $file and $file =~ s/.*\///;

    return $file;
}


sub printMsg($$) {
    my ($level, $msg) = @_;

    my $prolog = "";

    $log_time    and $prolog .= localtime()   . " ";
    $log_process and $prolog .= _basename($0) . " ($$) ";

    print $file_handle $prolog . "[$level] $msg\n";

    if ($log_position) {
        my @caller0 = caller($depth);
        my @caller1 = caller($depth + 1);

        my $file = $caller0[1];
        my $line = $caller0[2];
        my $func = $caller1[3] || "main function";

        print $file_handle $prolog . "[$level] ... in $func at $file:$line\n";
    }
}


sub msg($$@) {
    local $depth = $depth + 1;

    my $level  = shift;
    my $format = shift;

    my $msg_lvl_num = level2num($level);
    my $log_lvl_num = level2num($threshold);

    my $msg;

    if (not defined $log_lvl_num) {
        $level = "FATAL";
        $msg   = "INTERNAL ERROR: Logging failed, log level \"$threshold\" unknown";
    }
    elsif (not defined $msg_lvl_num) {
        $level = "FATAL";
        $msg   = "INTERNAL ERROR: Logging failed, message level \"$level\" unknown";
    }
    else {
        $msg_lvl_num <= $log_lvl_num || return;

        my @args = _arg2str(@_);

        $msg = sprintf($format, @args);

        if ($@) {
            $level = 'FATAL';
            $msg   = "INTERNAL ERROR: Logging failed, can't format log message";
        }
    }

    printMsg($level, $msg);
}


sub dumpStack($) {
    local $log_position = 0;
    local $depth = $depth + 1;

    my $level = shift;

    msg($level, "--- Stack trace top ---");

    for (my $i = $depth; ; ++$i) {
        my @caller = caller($i);

        @caller or last;

        my $file = $caller[1];
        my $line = $caller[2];
        my $func = $caller[3];

        msg($level, "$func at $file:$line");
    }

    msg($level, "--- Stack trace bottom ---");
}


sub FATAL($@) {
    my $format = shift;

    local $depth = $depth + 1;

    msg("FATAL", $format, @_);

    dumpStack("FATAL");

    exit 127;
}

sub ERROR($@) {
    my $format = shift;

    local $depth = $depth + 1;

    msg("ERROR", $format, @_);
}


sub WARN($@) {
    my $format = shift;

    local $depth = $depth + 1;

    msg("WARN", $format, @_);
}


sub INFO($@) {
    my $format = shift;

    local $depth = $depth + 1;

    msg("INFO", $format, @_);
}


sub DEBUG($@) {
    my $format = shift;

    local $depth = $depth + 1;

    msg("DEBUG", $format, @_);
}


sub DEBUX($$@) {
    my $number = shift;
    my $format = shift;

    local $depth = $depth + 1;

    msg("DEBUG$number", $format, @_);
}


1;
