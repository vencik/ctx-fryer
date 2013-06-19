package Table::Iterator;

use strict;
use warnings;

use Table;

use overload (
    "bool" => "isValid",
    "++"   => "next",
    );


sub reset($);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $tab = shift;

    UNIVERSAL::isa($tab, "Table")
    or FATAL("INTERNAL ERROR: %s isn't a table", $tab);

    my $this = {
        _tab  => $tab,
    };

    bless($this, $class);

    return $this->reset();
}


sub reset($) {
    my $this = shift;

    my $tab  = $this->{_tab};
    my @keys = $tab->keys();

    $this->{_keys} = \@keys;
    $this->{_idx}  = 0;

    return $this;
}


sub deref($) {
    my $this = shift;

    my $keys = $this->{_keys};
    my $idx  = $this->{_idx};

    $idx < @$keys
    or FATAL("INTERNAL ERROR: Dereference of end iterator");

    my $tab = $this->{_tab};
    my $key = $keys->[$idx];

    my $val = $tab->at(@$key);

    wantarray || return $val;

    return (@$key, $val);
}


sub isValid($) {
    my $this = shift;

    my $idx  = $this->{_idx};
    my $keys = $this->{_keys};

    return $idx < @$keys;
}


sub atEnd($) {
    my $this = shift;

    return !$this->isValid();
}


sub next($) {
    my $this = shift;

    ++$this->{_idx};

    return $this;
}


sub each($) {
    my $this = shift;

    my @deref;

    if ($this->isValid()) {
        @deref = $this->deref();

        $this->next();
    }

    return @deref;
}


1;
