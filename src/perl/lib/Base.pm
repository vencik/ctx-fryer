package Base;

use strict;
use warnings;


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {};

    return bless($this, $class);
}


1;
