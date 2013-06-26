package CTXFryer::Math::Set;

use strict;
use warnings;

use base qw(CTXFryer::Math::Base CTXFryer::Signed);

use CTXFryer::Table;
use CTXFryer::Table::Iterator;

use CTXFryer::Logging qw(:all);

use CTXFryer::Serialised qw(:xml);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new();

    $this->{_impl} = new CTXFryer::Table;

    bless($this, $class);

    $this->_addItem(@_);

    return $this;
}


sub copy($) {
    my $orig = shift;

    my $class = ref($orig);

    my $this = $class->SUPER::new();

    $this->{_impl} = $orig->{_impl}->copy();

    bless($this, $class);

    return $this;
}


sub cardinality($) {
    my $this = shift;

    return $this->{_impl}->size();
}


sub itemKey($) {
    my $item = shift;

    my $key;

    # Signature is a good key
    if (UNIVERSAL::can($item, "signature")) {
        $key = $item->signature();
    }

    # Scalar is on itself a key
    elsif ("SCALAR" eq ref(\$item)) {
        $key = $item;
    }

    # Can't assign a key
    else {
        FATAL("INTERNAL ERROR: Can't assign a key to item %s", $item);
    }

    return $key;
}


sub _addItem($@) {
    my $this = shift;

    my $impl = $this->{_impl};

    foreach my $item (@_) {
        my $key = itemKey($item);

        $impl->at($key, $item);
    }
}


sub _removeItem($@) {
    my $this = shift;

    my $impl = $this->{_impl};

    foreach my $item (@_) {
        my $key = itemKey($item);

        $impl->delete($key);
    }
}


sub isEmpty($) {
    my $this = shift;

    return $this->cardinality() == 0;
}


sub items($) {
    my $this = shift;

    my $impl = $this->{_impl};

    return $impl->values();
}


sub contains($@) {
    my $this = shift;

    my $impl = $this->{_impl};

    foreach my $item (@_) {
        my $key = itemKey($item);

        $impl->exists($key) || return;
    }

    return 1;
}


sub isSubset($$) {
    my ($this, $set) = @_;

    UNIVERSAL::isa($set, "CTXFryer::Math::Set")
    or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);

    foreach my $item ($this->items()) {
        $set->contains($item) || return;
    }

    return 1;
}


sub assign($$) {
    my ($this, $set) = @_;

    UNIVERSAL::isa($set, "CTXFryer::Math::Set")
    or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);

    $this->{_impl} = $set->{_impl}->copy();
}


sub unionAssign($@) {
    my $this = shift;

    foreach my $set (@_) {
        UNIVERSAL::isa($set, "CTXFryer::Math::Set")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);

        $this->_addItem($set->items());
    }
}


sub union(@) {
    @_ || return new CTXFryer::Math::Set;

    my $first_set = shift;

    my $this = $first_set->copy();

    $this->unionAssign(@_);

    return $this;
}


sub subtractionAssign($@) {
    my $this = shift;

    my $impl = $this->{_impl};

    foreach my $set (@_) {
        UNIVERSAL::isa($set, "CTXFryer::Math::Set")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);

        my $iter = new CTXFryer::Table::Iterator($set->{_impl});

        for (; $iter->isValid(); ++$iter) {
            my ($key, $val) = $iter->deref();

            if ($impl->exists($key)) {
                $impl->delete($key);
            }
        }
    }

    return $this;
}


sub subtraction($@) {
    my $this = new CTXFryer::Math::Set;

    my $left = shift;

    foreach my $set (@_) {
        UNIVERSAL::isa($set, "CTXFryer::Math::Set")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);
    }

    my $iter = new CTXFryer::Table::Iterator($left->{_impl});

    for (; $iter->isValid(); ++$iter) {
        my ($key, $val) = $iter->deref();

        my $exists;

        foreach my $set (@_) {
            if ($set->{_impl}->exists($key)) {
                $exists = 1;
                last;
            }
        }

        $exists || $this->_addItem($val);
    }

    return $this;
}


sub _intersectionAssign($@) {
    my $this = shift;

    my $impl = $this->{_impl};

    foreach my $key ($impl->keys()) {
        foreach my $set (@_) {
            unless ($set->{_impl}->exists($key)) {
                $impl->delete($key);

                last;
            }
        }
    }

    return $this;
}


sub _sortByCardinality(@) {
    foreach my $set (@_) {
        UNIVERSAL::isa($set, "CTXFryer::Math::Set")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);
    }

    return CORE::sort {
        $a->cardinality() <=> $b->cardinality
    } @_;
}


sub intersectionAssign($@) {
    my $this = shift;

    foreach my $set (@_) {
        UNIVERSAL::isa($set, "CTXFryer::Math::Set")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Math::Set", $set);
    }

    my @sets = _sortByCardinality(@_);

    return $this->_intersectionAssign(@sets);
}


sub intersection(@) {
    @_ || return new CTXFryer::Math::Set;

    my @sets = _sortByCardinality(@_);

    my $first = shift(@sets);

    my $this = $first->copy();

    return $this->_intersectionAssign(@sets);
}


sub signature($) {
    my $this = shift;

    my @keys     = $this->{_impl}->keys();
    my @keys_str = map("[" . join(", ", @$_) . "]", $this->{_impl}->keys());

    my $signature = ref($this) . "(";

    $signature .= join(", ", sort(@keys_str));
    $signature .= ")";

    return $signature;
}


sub str($) {
    my $this = shift;

    my @items = $this->items();

    local $" = ", ";

    return ref($this) . "(@items)";
}


sub xmlElementName { "math:set" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        cardinality => $this->cardinality(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    my @items = $this->items();

    return map(xmlNewElement("set-item", {}, $_), @items);
}


1;
