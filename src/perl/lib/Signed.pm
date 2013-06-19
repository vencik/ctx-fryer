package Signed;

use strict;
use warnings;

use base qw(Serialised);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $sig = shift;

    my $this = {
        _sig => $sig,
    };

    return bless($this, $class);
}


sub signature($) {
    my $this = shift;

    return $this->{_sig};
}


sub str($) {
    my $this = shift;

    my $sig = $this->signature();

    return ref($this) . "(\"$sig\")";
}


1;
