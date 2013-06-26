package CTXFryer::LRParser::Action::Shift;

use strict;
use warnings;

use base qw(CTXFryer::LRParser::Action);

use CTXFryer::LRParser::Action;

use CTXFryer::Logging qw(:all);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $state = shift;

    defined $state
    or FATAL("INTERNAL ERROR: State not defined");

    my $this = $class->SUPER::new();

    $this->{_state} = $state;

    return bless($this, $class);
}


sub state($) {
    my $this = shift;

    return $this->{_state};
}


sub str($) {
    my $this = shift;

    my $state = $this->state();

    return $this->SUPER::str($state);
}


sub xmlElementName { "shift" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = $this->SUPER::xmlElementAttrs();

    $attrs{"state-id"} = $this->state();

    return %attrs;
}


1;
