package CTXFryer::ProjectDef::Grammar::TerminalSymbols;

use strict;
use warnings;

use base qw(CTXFryer::Base);

use CTXFryer::Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {};

    return bless($this, $class);
}


sub identifers($) {
    my $this = shift;

    return keys %$this;
}


sub map($) {
    my $this = shift;

    return %$this;
}


sub regex($$) {
    my ($this, $ident) = @_;

    return $this->{$ident};
}


sub exists($$) {
    my ($this, $ident) = @_;

    exists $this->{$ident};
}


sub parse($$$) {
    my ($this, $line, $pos) = @_;

    my $ret = 0;

    # Terminal definition
    if ($line =~ /^\s*([A-Za-z_]\w*)\s*=\s*(\/.*\/[a-z]*)\s*$/) {
        my $ident = $1;
        my $regex = $2;

        exists $this->{$ident} &&
        WARN("Terminal symbol %s redefinition at %s", $ident, $pos);

        $this->{$ident} = $regex;
    }

    # Parse error
    else {
        ERROR("Terminal symbol definition syntax error at %s", $pos);

        $ret = 1;
    }

    return $ret;
}


1;
