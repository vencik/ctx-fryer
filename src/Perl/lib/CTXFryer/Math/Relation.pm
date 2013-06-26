package CTXFryer::Math::Relation;

use strict;
use warnings;

use base qw(CTXFryer::Math::Set);

use CTXFryer::Math::Multiplet;

use CTXFryer::Logging qw(:all);


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $arity = shift
    or FATAL("INTERNAL ERROR: Nular relations aren't supported");

    my $this = $class->SUPER::new();

    $this->{_arity} = $arity;

    bless($this, $class);

    $this->_addItem(@_);

    return $this;
}


sub arity($) {
    my $this = shift;

    return $this->{_arity};
}


sub _addItem($@) {
    my $this = shift;

    my $arity = arity($this);

    foreach my $item (@_) {
        UNIVERSAL::isa($item, "CTXFryer::Math::Multiplet")
        or FATAL("INTERNAL ERROR: %s isn't a multiplet", $item);

        $item->arity() == $arity
        or FATAL("INTERNAL ERROR: %s arity isn't equal to %d", $item, $arity);
    }

    return $this->SUPER::_addItem(@_);
}


sub relates($@) {
    my $this = shift;

    arity($this) == @_
    or FATAL("INTERNAL ERROR: Invalid argument count (%d) for %s", scalar(@_), $this);

    return $this->contains(new CTXFryer::Math::Multiplet(@_));
}


sub _cut($@) {
    my $this = shift;

    my $arity = arity($this);

    DEBUX(1, "Cutting %d-ary relation using keys %s", $arity, \@_);

    my @cut_items;

    foreach my $multiplet ($this->items()) {
        my @cut_multiplet;

        my $i = 0;

        # Check if fixed multiplet items match
        for (; $i < $arity; ++$i) {
            my $cut_key = $_[$i];

            if (defined $cut_key) {
                $multiplet->itemSignature($i) eq $cut_key || last;
            }
            else {
                push(@cut_multiplet, $multiplet->item($i));
            }
        }

        # All fixed multiplet items match
        DEBUX(2, "%s does%s match %s", $multiplet, ($i == $arity ? "" : "n't"), \@_);

        if ($i == $arity) {
            push(@cut_items, new CTXFryer::Math::Multiplet(@cut_multiplet));
        }
    }

    @cut_items || return;

    $arity = $cut_items[0]->arity();

    return new CTXFryer::Math::Relation($arity, @cut_items);
}


sub cut($@) {
    my $this = shift;

    my $cut = $this->_cut(map(CTXFryer::Math::Set::itemKey($_), @_));

    DEBUG("Result of %s|%s: %s", $this, \@_, $cut);

    return $cut;
}


1;
