package Identified;

use strict;
use warnings;

use base qw(Base);

use Base;

use overload (
    "==" => "isEqual",
    "!=" => "isNotEqual",
    );


our $next_id;

BEGIN {
    $next_id = 0;
}


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new();

    $this->{_id} = sprintf("0x%08x", $next_id++);

    return bless($this, $class);
}


sub id($) {
    my $this = shift;

    return $this->{_id};
}


sub isEqual($$) {
    my ($obj1, $obj2) = @_;

    return $obj1->id() eq $obj2->id();
}


sub isNotEqual($$) {
    my ($obj1, $obj2) = @_;

    return !isEqual($obj1, $obj2);
}


1;
