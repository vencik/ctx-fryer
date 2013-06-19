package PDA::State;

use strict;
use warnings;

use base qw(Identified Serialised);


sub str($) {
    my $this = shift;

    my $id = $this->id();

    return ref($this) . "($id)";
}


1;
