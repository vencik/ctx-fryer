package FSA::Branch;

use strict;
use warnings;

use base qw(Identified Serialised);

use FSA::Symbol::Set;

use Logging qw(:all);



sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new();

    $this->{_symbol} = new FSA::Symbol::Set();
    $this->{_target} = undef;

    return bless($this, $class);
}


sub copy($) {
    my $orig = shift;

    my $class = ref $orig;

    my $this = $class->SUPER::new();

    $this->{_symbol} = FSA::Symbol::Set::copy($orig->symbol());
    $this->{_target} = $orig->target();

    return bless($this, $class);
}


sub symbol($@) {
    my ($this, $symbol) = @_;

    if ($symbol) {
        $this->{_symbol} = $symbol;
    }

    return $this->{_symbol};
}


sub target($@) {
    my ($this, $state) = @_;

    if ($state) {
        $this->{_target} = $state;
    }

    return $this->{_target};
}


sub str($) {
    my $this = shift;

    my $id     = $this->id();
    my $symbol = $this->symbol();
    my $target = $this->target();

    my $target_id = defined $target ? $target->id : "<!undef!>";

    return ref($this) . "($id, symbol set: $symbol, target: $target_id)";
}


sub xmlElementName { "fsa-branch" }

sub xmlElementAttrs($) {
    my $this = shift;

    my $target = $this->target();

    defined $target
    or FATAL("INTERNAL ERROR: %s target isn't defined", $this);

    return (
        "id"        => $this->id(),
        "target-id" => $target->id(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    return $this->symbol();
}


1;
