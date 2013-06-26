package CTXFryer::Table;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Logging qw(:all);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $arity = shift || 1;

    # Sanity check
    @_ % 2 == 0
    or FATAL("INTERNAL ERROR: Invalid arguments: %s", \@_);

    my $this = {
        _arity => $arity,
        _impl  => { @_ },
    };

    return bless($this, $class);
}


sub arity($) {
    my $this = shift;

    return $this->{_arity};
}


sub clear($) {
    my $this = shift;

    $this->{_impl} = {};
}


sub copy($) {
    my $orig = shift;

    my $orig_arity = $orig->arity();
    my $orig_impl  = $orig->{_impl};

    my %impl;

    if (1 < $orig_arity) {
        while (my ($key, $subtab) = each %$orig_impl) {
            $impl{$key} = $subtab->copy();
        }
    }
    else {
        %impl = %$orig_impl;
    }

    my $this = {
        _arity => $orig_arity,
        _impl  => \%impl,
    };

    bless($this, ref($orig));

    return $this;
}


sub cut($@) {
    my $this = shift;
    my $key  = shift;

    my $arity = $this->arity();

    my $impl = $this->{_impl};
    my %impl;

    # Final level
    if (1 == $arity) {
        # Final cut
        if (defined $key) {
            my $val = shift;

            # All keys satisfying a predicate
            if ("CODE" eq ref($key)) {
                foreach my $k (keys %$impl) {
                    if (&$key($k)) {
                        my $v = $impl->{$k};

                        # Filter values
                        if (defined $val) {
                            if ("CODE" eq ref($val)) {
                                &$val($v) || next;
                            }
                            elsif ($val ne $v) {
                                next;
                            }
                        }

                        $impl{$k} = $v;
                    }
                }
            }

            # Specific key
            else {
                my $v = $impl->{$key};

                defined $v or return;

                # Filter value
                if (defined $val) {
                    if ("CODE" eq ref($val)) {
                        &$val($v) || return;
                    }
                    elsif ($val ne $v) {
                        return;
                    }
                }

                $impl{$key} = $v;
            }

            %impl || return;
        }

        # Whole table
        else {
            %impl = %$impl;
        }

        return new CTXFryer::Table(1, %impl);
    }

    my $subtab;

    if (defined $key) {
        # All keys satisfying a predicate
        if ("CODE" eq ref($key)) {
            my %sub_impl;

            foreach my $k (keys %$impl) {
                &$key($k) and $sub_impl{$k} = $impl->{$k};
            }

            %sub_impl || return;

            $impl = \%sub_impl;
        }

        # Specific key
        else {
            $subtab = $this->_subtab($key) || return;

            return $subtab->cut(@_);
        }
    }

    undef $arity;

    while (($key, $subtab) = each %$impl) {
        my $subcut = $subtab->cut(@_);

        defined $subcut || next;

        # Sanity check
        $subcut->isEmpty()
        and FATAL("INTERNAL ERROR: Sub-cut defined but empty");

        $impl{$key} = $subcut;

        my $subcut_arity = $subcut->arity();

        if (defined $arity) {
            $arity == $subcut_arity + 1
            or FATAL("INTERNAL ERROR: Invalid sub-cut arity of %d (expected %d)",
                     $subcut_arity, $arity - 1);
        }
        else {
            $arity = $subcut_arity + 1;
        }
    }

    %impl || return;

    my $cut = {
        _arity => $arity,
        _impl  => \%impl,
    };

    return bless($cut, ref($this));
}


sub keys($@) {
    my $this = shift;
    my $mask = shift;

    my $arity = $this->arity();
    my $impl  = $this->{_impl};

    if (1 == $arity) {
        my @keys;

        if (defined $mask) {
            if (CORE::exists $impl->{$mask}) {
                @keys = ($mask);
            }
        }
        else {
            @keys = CORE::keys %$impl;
        }

        wantarray && return map([ $_ ], @keys);

        return scalar(@keys);
    }

    if (wantarray) {
        my @subkeys;

        if (defined $mask) {
             CORE::exists $impl->{$mask} || return ();

             @subkeys = ($mask);
        }
        else {
             @subkeys = CORE::keys %$impl;
        }

        my @keys;

        foreach my $subkey (@subkeys) {
            my $subtab = $impl->{$subkey};

            push(@keys, map([ $subkey, @$_ ], $subtab->keys(@_)));
        }

        return @keys;
    }

    if (defined $mask) {
        return CORE::exists $impl->{$mask} ? 1 : 0;
    }

    my $keys_cnt = 0;

    while (my ($key, $subtab) = each(%$impl)) {
        $keys_cnt += $subtab->keys(@_);
    }

    return $keys_cnt;
}


sub size($) {
    my $this = shift;

    return scalar($this->keys());
}


sub isEmpty($) {
    my $this = shift;

    return 0 == $this->size();
}


sub values($@) {
    my $this = shift;
    my $mask = shift;

    my $arity = $this->arity();
    my $impl  = $this->{_impl};

    if (1 == $arity) {
        if (defined $mask) {
            my $val = $impl->{$mask};

            return ($val);
        }

        return CORE::values %$impl;
    }

    if (defined $mask) {
        my $subtab = $impl->{$mask};

        unless (defined $subtab) {
            return wantarray ? () : 0;
        }

        return $subtab->values(@_);
    }

    if (wantarray) {
        my @values;

        while (my ($key, $subtab) = each(%$impl)) {
            push(@values, $subtab->values(@_));
        }

        return @values;
    }

    my $values_cnt = 0;

    while (my ($key, $subtab) = each(%$impl)) {
        $values_cnt += $subtab->values(@_);
    }

    return $values_cnt;
}


sub _subtab($$@) {
    my ($this, $key, $create) = @_;

    my $arity = $this->arity();

    1 < $arity
    or FATAL("INTERNAL ERROR: Can't get sub-table of table with arity %d", $arity);

    my $subtab = $this->{_impl}->{$key};

    if ($subtab) {
        UNIVERSAL::isa($subtab, "CTXFryer::Table")
        or FATAL("INTERNAL ERROR: %s isn't a Table", $subtab);

        return $subtab;
    }

    $create || return;

    $subtab = new CTXFryer::Table($arity - 1);

    return $this->{_impl}->{$key} = $subtab;
}


sub at($@) {
    my $this = shift;

    my $arity = $this->arity();
    my $set   = $arity < @_;

    my $key = shift;

    defined $key
    or FATAL("Key not defined");

    if (1 < $arity) {
        my $subtab = $this->_subtab($key, $set) || return;

        return $subtab->at(@_);
    }

    ($set ? 1 : 0) == @_
    or FATAL("INTERNAL ERROR: Invalid arguments: %s", \@_);

    my $val;

    if ($set) {
        $this->{_impl}->{$key} = $val = shift;
    }
    else {
        $val = $this->{_impl}->{$key};
    }

    return $val;
}


sub delete($@) {
    my $this = shift;

    my $arity = $this->arity();
    my $key   = shift;

    defined $key
    or FATAL("Key not defined");

    if (1 < $arity) {
        my $delete_entire_subtab = scalar(@_);

        unless ($delete_entire_subtab) {
            my $subtab = $this->_subtab($key) || return;

            $subtab->delete(@_);

            $delete_entire_subtab = $subtab->isEmpty();
        }

        $delete_entire_subtab || return;
    }

    CORE::delete($this->{_impl}->{$key});
}


sub exists($@) {
    my $this = shift;

    my $cut = $this->cut(@_);

    return defined $cut;
}


sub str($) {
    my $this = shift;

    my $arity = $this->arity();
    my @items;

    foreach my $key ($this->keys()) {
        my $val = $this->at(@$key);

        local $" = ", ";

        push(@items, "(@$key) => $val");
    }

    local $" = ", ";

    return ref($this) . "(arity: $arity, items: @items)";
}


sub xmlElementName { "table" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        arity => $this->arity(),
        size  => $this->size(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    my $key_index_max   = $this->arity() - 1 || 1;
    my $key_index_len   = int(log($key_index_max) / log(10) + 1);
    my $key_attr_format = "key%0" . $key_index_len . "u";

    return map {
        my $key = $_;
        my %attrs;

        for (my $i = 0; $i < @$key; ++$i) {
            my $key_attr = sprintf($key_attr_format, $i);

            $attrs{$key_attr} = $key->[$i];
        }

        my $entry = $this->at(@$key);

        xmlNewElement("table-entry", \%attrs, $entry);
    }
    $this->keys();
}


1;
