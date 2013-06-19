package Math::Multiplet;

use strict;
use warnings;

use base qw(Math::Base Signed);

use List;

use Logging qw(:all);

use Serialised qw(:xml);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    @_ || FATAL("INTERNAL ERROR: 0-item multiplets are not supported");

    my $items = new List(@_);

    my $this = $class->SUPER::new;

    $this->{_impl} = $items;

    return bless($this, $class);
}


sub arity($) {
    my $this = shift;

    return $this->{_impl}->size();
}


sub item($@) {
    my ($this, $index) = @_;

    my $impl = $this->{_impl};

    if (defined $index) {
        $index < $this->arity()
        or FATAL("INTERNAL ERROR: Index %d is invalid for %s", $index, $this);

        return $impl->at($index);
    }

    return $impl->items();
}


sub itemSignature($@) {
    my $this = shift;

    my @items_sig = map(Math::Set::itemKey($_), $this->item(@_));

    wantarray && return @items_sig;

    return join(", ", @items_sig);
}


sub signature($) {
    my $this = shift;

    my $signature = ref($this);

    $signature .= "(";
    $signature .= $this->itemSignature();
    $signature .= ")";

    return $signature;
}


sub str($) {
    my $this = shift;

    my $arity = $this->arity();
    my $impl  = $this->{_impl};

    return ref($this) . "(arity: $arity, items: $impl)";
}


sub xmlElementName { "math:multiplet" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        arity => $this->arity(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    my $impl = $this->{_impl};

    return (
        xmlNewElement("items", {}, $impl),
    );
}


1;
