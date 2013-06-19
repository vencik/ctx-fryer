package Math::Relation::Binary;

use strict;
use warnings;

use base qw(Math::Relation);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(2, @_);

    return bless($this, $class);
}


1;
