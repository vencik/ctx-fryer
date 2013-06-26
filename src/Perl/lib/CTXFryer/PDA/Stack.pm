package CTXFryer::PDA::Stack;

use strict;
use warnings;

use base qw(CTXFryer::List);

use CTXFryer::List;


sub top($) {
    my $this = shift;

    return $this->tail();
}


1;
