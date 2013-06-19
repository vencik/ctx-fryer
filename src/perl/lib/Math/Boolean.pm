package Math::Boolean;

use strict;
use warnings;

use base qw(Math::Set);

use overload (
    bool => 'bool',
    );

use Logging qw(:all);


sub false { bless new Math::Set    }
sub true  { bless new Math::Set(0) }


sub bool($) {
    my $this = shift;

    return !$this->isEmpty();
}


1;
