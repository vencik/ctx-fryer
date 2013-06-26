package CTXFryer::Math::Base;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {};

    return bless($this, $class);
}


1;
