package Grammar::Terminal;

use strict;
use warnings;

use base qw(Grammar::Symbol);

use Grammar::Symbol;


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my $ident = shift;
    my $regex = shift;

    my $this = $class->SUPER::new($ident);

    $this->{_regex} = $regex;

    bless($this, $class);

    return $this;
}


sub regex($) {
    my $this = shift;

    return $this->{_regex};
}


sub xmlElementName { "terminal-symbol" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = $this->SUPER::xmlElementAttrs();

    $attrs{regex} = $this->regex();

    return %attrs;
}


1;
