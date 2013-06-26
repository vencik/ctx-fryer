package CTXFryer::Math::Couple;

use strict;
use warnings;

use base qw(CTXFryer::Math::Multiplet);


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_);

    return bless($this, $class);
}


sub first($) {
    my $this = shift;

    return $this->item(0);
}


sub second($) {
    my $this = shift;

    return $this->item(1);
}


sub left($);  *left  = "first";
sub right($); *right = "second";


sub str($) {
    my $this = shift;

    my $first  = $this->first;
    my $second = $this->second();

    return ref($this) . "($first, $second)";
}


1;
