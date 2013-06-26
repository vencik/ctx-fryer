package CTXFryer::Math::Function;

use strict;
use warnings;

use base qw(CTXFryer::Math::Relation);

use CTXFryer::Logging qw(:all);


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $arity = shift;

    my $this = $class->SUPER::new($arity + 1);

    bless($this, $class);

    $this->_addItem(@_);

    return $this;
}


sub arity($) {
    my $this = shift;

    return $this->SUPER::arity() - 1;
}


sub _addItem($@) {
    my $this = shift;

    scalar(@_) % 2
    and FATAL("INTERNAL ERROR: Odd number of arguments: %d", scalar(@_));

    my @rel_items;

    while (@_) {
        my $args = shift;
        my $val  = shift;

        my @args_sig = $args->itemSignature();

        $this->{_impl}->values(@args_sig)
        and FATAL("INTERNAL ERROR: Function for arguments with keys %s already defined: %s",
                  \@args_sig, $this);

        push(@rel_items, new CTXFryer::Math::Multiplet($args->item(), $val));
    }

    $this->SUPER::_addItem(@rel_items);
}


sub definitionRange($) {
    my $this = shift;

    my @def_range;

    foreach my $multiplet ($this->items()) {
        my @args = $multiplet->item();

        pop(@args);

        my $args = new CTXFryer::Math::Multiplet(@args);

        push(@def_range, $args);
    }

    return new CTXFryer::Math::Set(@def_range);
}


sub valueRange($) {
    my $this = shift;

    my $arity     = $this->arity();
    my $val_index = $arity - 1;
    my @val_range;

    foreach my $multiplet ($this->items()) {
        my $val = $multiplet->item($val_index);

        push(@val_range, $val);
    }

    return new CTXFryer::Math::Set(@val_range);
}


sub project($$) {
    my ($this, $args) = @_;

    # Sanity checks
    UNIVERSAL::isa($args, "CTXFryer::Math::Multiplet")
    or FATAL("INTERNAL ERROR: %s isn't a multiplet", $args);

    $args->arity() == $this->arity()
    or FATAL("INTERNAL ERROR: %s arity doesn't match %s", $args, $this);

    my $cut = $this->cut($args->item()) || return;

    my $cut_size = $cut->cardinality();

    if (1 == $cut_size) {
        my ($val_1plet) = $cut->items();

        my @val = $val_1plet->item();

        return $val[0];
    }

    FATAL("INTERNAL ERROR: %d values for arguments %s are defined",
          $cut_size, $args);
}


1;
