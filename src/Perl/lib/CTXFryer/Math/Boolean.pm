package CTXFryer::Math::Boolean;

use strict;
use warnings;

use base qw(CTXFryer::Math::Set);

use overload (
    bool => 'bool',
    );

use CTXFryer::Logging qw(:all);


sub false { bless new CTXFryer::Math::Set    }
sub true  { bless new CTXFryer::Math::Set(0) }


sub bool($) {
    my $this = shift;

    return !$this->isEmpty();
}


1;
