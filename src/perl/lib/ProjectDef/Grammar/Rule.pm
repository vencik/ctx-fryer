package ProjectDef::Grammar::Rule;

use strict;
use warnings;

use base qw(Base);

use Logging qw(:all);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    @_ % 2
    and FATAL("INTERNAL ERROR: invalid arguments for rule constructor: %s", \@_);

    my %this = @_;
    my $this = \%this;

    defined $this->{lhs}    || FATAL("INTERNAL ERROR: Undefined rule LHS");
    defined $this->{rhs}    || FATAL("INTERNAL ERROR: Undefined rule RHS");
    defined $this->{number} || FATAL("INTERNAL ERROR: Undefined rule number");

    return bless($this, $class);
}


sub lhs($) {
    my $this = shift;

    return $this->{lhs};
}


sub rhs($) {
    my $this = shift;

    return @{$this->{rhs}};
}


sub symbols($) {
    my $this = shift;

    return wantarray ? ($this->lhs(), $this->rhs())
                     : 1 + scalar $this->rhs();
}


sub number($) {
    my $this = shift;

    return $this->{number};
}


1;
