package List;

use strict;
use warnings;

use base qw(Serialised);

use Serialised qw(:xml);

use overload (
    "[]" => "at",
);


sub new($@) {
    my $class = CORE::shift; $class = ref $class || $class;

    my $this = {
        _impl => [ @_ ],
    };

    return bless($this, $class);
}


sub copy($@) {
    my $orig = shift;

    my @item_copies = map {
        my $item = $_;

        UNIVERSAL::can($item, "copy")
        and $item = $item->copy(@_);

        $item;
    } @{$orig->{_impl}};

    my $this = {
        _impl => [ @item_copies ],
    };

    my $class = ref $orig;

    return bless($this, $class);
}


sub size($) {
    my $this = CORE::shift;

    return scalar(@{$this->{_impl}});
}


sub isEmpty($) {
    my $this = CORE::shift;

    return 0 == $this->size();
}


sub items($) {
    my $this = shift;

    return @{$this->{_impl}};
}


sub at($$) {
    my ($this, $index) = @_;

    return $this->{_impl}->[$index];
}


sub head($) {
    my $this = CORE::shift;

    return $this->at(0);
}


sub tail($) {
    my $this = CORE::shift;

    return $this->at(-1);
}


sub unshift($@) {
    my $this = CORE::shift;

    CORE::unshift(@{$this->{_impl}}, @_);
}


sub push($@) {
    my $this = CORE::shift;

    CORE::push(@{$this->{_impl}}, @_);
}


sub splice($$@) {
    my ($this, $off, $len) = @_;

    my @splice;

    @splice = CORE::splice(@{$this->{_impl}}, $off, $len);

    wantarray && return $this->new(@splice);
    1 == $len && return $splice[0];

    return scalar(@splice);
}


sub shift($@) {
    my ($this, $cnt) = @_;

    defined $cnt or $cnt = 1;

    return $this->splice(0, $cnt);
}


sub pop($@) {
    my ($this, $cnt) = @_;

    defined $cnt or $cnt = 1;

    return $this->splice($this->size() - $cnt, $cnt);
}


sub clear($) {
    my $this = CORE::shift;

    $this->{_impl} = [];
}


sub join(@) {
    my $this = new List;

    foreach my $list (@_) {
        $this->push($list->items());
    }

    return $this;
}


sub str($) {
    my $this = CORE::shift;

    my $impl = $this->{_impl};

    local $" = ", ";

    return ref($this) . "(@$impl)";
}


sub xmlElementName { "list" }

sub xmlElementAttrs($) {
    my $this = CORE::shift;

    return (
        size => $this->size(),
    );
}

sub xmlChildren($) {
    my $this = CORE::shift;

    my $impl = $this->{_impl};

    my @children;

    for (my $i = 0; $i < $this->size(); ++$i) {
        CORE::push(@children,
            xmlNewElement(
               "list-item",
               {
                   "index" => $i,
               },
               $impl->[$i]
               ));
    }

    return @children;
}


1;
