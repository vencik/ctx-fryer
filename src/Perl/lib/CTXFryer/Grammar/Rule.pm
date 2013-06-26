package CTXFryer::Grammar::Rule;

use strict;
use warnings;

use base qw(CTXFryer::Grammar::Component CTXFryer::Identified CTXFryer::Serialised);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::List;


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $this = new CTXFryer::Identified;

    $this->{_left}  = shift;
    $this->{_right} = new CTXFryer::List(@_);

    return bless($this, $class);
}


sub number($@) {
    my ($this, $no) = @_;

    if (defined $no) {
        $this->{_number} = $no;
    }

    return $this->{_number};
}


sub left($) {
    my $this = shift;

    return $this->{_left};
}


sub right($@) {
    my ($this, $index) = @_;

    my $right = $this->{_right};

    defined $index && return $right->at($index);

    return wantarray ? $right->items() : $right;
}


sub length($) {
    my $this = shift;

    return $this->right()->size();
}


sub str($) {
    my $this = shift;

    my $left  = $this->left();
    my @right = $this->right();

    return ref($this) . "($left => @right)";
}


sub xmlElementName { "rule" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = (
        id     => $this->id(),
        length => $this->length(),
    );

    my $number = $this->number();

    defined $number and $attrs{number} = $number;

    return %attrs;
}

sub xmlChildren($) {
    my $this = shift;

    my $right_list = new CTXFryer::List(map($_->ident(), $this->right()));

    return (
        xmlNewElement("left-side",  {}, $this->left()->ident()),
        xmlNewElement("right-side", {}, $right_list),
    );
}


1;
