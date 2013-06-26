package CTXFryer::PDA::State;

use strict;
use warnings;

use base qw(CTXFryer::Identified CTXFryer::Serialised);


sub str($) {
    my $this = shift;

    my $id = $this->id();

    return ref($this) . "($id)";
}


1;
