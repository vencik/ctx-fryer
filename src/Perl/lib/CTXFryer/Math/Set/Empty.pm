package CTXFryer::Math::Set::Empty;

use strict;
use warnings;

use base qw(CTXFryer::Math::Set);

use CTXFryer::Math::Set;


our $empty_set;

BEGIN {
    $empty_set = new CTXFryer::Math::Set;
}


sub new($) {
    my $class = shift; $class = ref $class || $class;

    # Empty set is a singleton
    return $empty_set;
}


1;
