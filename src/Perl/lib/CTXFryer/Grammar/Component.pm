package CTXFryer::Grammar::Component;

use strict;
use warnings;


sub grammar($@) {
    my ($this, $grammar) = @_;

    defined $grammar and $this->{_grammar} = $grammar;

    return $this->{_grammar};
}


1;
