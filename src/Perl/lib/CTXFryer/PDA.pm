package CTXFryer::PDA;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);

use CTXFryer::PDA::Input;
use CTXFryer::PDA::State;
use CTXFryer::PDA::Stack;

use CTXFryer::Logging qw(:all);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    # Sanity checks
    foreach my $state (@_) {
        UNIVERSAL::isa($state, "CTXFryer::PDA::State")
        or FATAL("INTERNAL ERROR: %s isn't a PDA state", $state);
    }

    my $this = {
        _states => [ @_ ],
        _stack  => new CTXFryer::PDA::Stack,
    };

    return bless($this, $class);
}


sub _addState($$) {
    my ($this, $state) = @_;

    UNIVERSAL::isa($state, "CTXFryer::PDA::State")
    or FATAL("INTERNAL ERROR: %s isn't a PDA state", $state);

    my $states = $this->{_states};

    push(@$states, $state);
}


sub _states($@) {
    my ($this, $index) = @_;

    my $states = $this->{_states};

    defined $index && return $states->[$index];

    return wantarray ? @$states : $states;
}


sub _stack($) {
    my $this = shift;

    return $this->{_stack};
}


sub _init($) {
    my $this = shift;

    FATAL("INTERNAL ERROR: Class %s doesn't define protected method _init");
}


sub _func($$$$) {
    my ($this, $state_no, $input, $stack) = @_;

    FATAL("INTERNAL ERROR: Class %s doesn't define protected method _func");
}


sub accepts($$) {
    my ($this, $input) = @_;

    my $stack = $this->_stack();

    $stack->clear();

    $this->_init();

    my $state_no = 0;

    while (not $input->isEmpty()) {
        $state_no = $this->_func($state_no, $input, $stack);

        defined $state_no || return;
    }

    return 1;
}


1;
