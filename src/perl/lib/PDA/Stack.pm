package PDA::Stack;

use strict;
use warnings;

use base qw(List);

use List;


sub top($) {
    my $this = shift;

    return $this->tail();
}


1;
