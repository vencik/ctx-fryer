package LRParser::State;

use strict;
use warnings;

use base qw(PDA::State);

use PDA::State;


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my ($number, $item_set) = @_;

    # Sanity checks
    UNIVERSAL::isa($item_set, "Grammar::ItemSet")
    or FATAL("INTERNAL ERROR: %s isn't an item set", $item_set);

    my $this = $class->SUPER::new();

    $this->{_number}   = $number;
    $this->{_item_set} = $item_set;

    return bless($this, $class);
}


sub number($) {
    my $this = shift;

    return $this->{_number};
}


sub itemSet($) {
    my $this = shift;

    return $this->{_item_set};
}


sub str($) {
    my $this = shift;

    my $number       = $this->number();
    my $item_set_str = $this->itemSet()->str();

    return ref($this) . "($number, item set: $item_set_str)";
}


1;
