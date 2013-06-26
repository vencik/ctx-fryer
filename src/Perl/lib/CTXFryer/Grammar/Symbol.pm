package CTXFryer::Grammar::Symbol;

use strict;
use warnings;

use base qw(CTXFryer::Identified CTXFryer::Grammar::Component CTXFryer::Serialised);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Table;

use CTXFryer::Logging qw(:all);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $ident = shift;

    my $this = new CTXFryer::Identified;

    $this->{_ident} = $ident;
    $this->{_attrs} = new CTXFryer::Table;

    return bless($this, $class);
}


sub ident($) {
    my $this = shift;

    return $this->{_ident};
}


sub attr($@) {
    my $this = shift;

    my $attrs = $this->{_attrs};

    foreach my $attr (@_) {
        # Sanity check
        UNIVERSAL::isa($attr, "CTXFryer::Grammar::Symbol::Attribute")
        or FATAL("INTERNAL ERROR: %s isn't a CTXFryer::Grammar::Symbol::Attribute", $attr);

        my $attr_id = $attr->identifier();

        $attrs->exists($attr_id)
        and WARN("Redefinition of attribute %s from %s to %s", $attr_id,
                 $attrs->at($attr_id), $attr);

        $attrs->at($attr_id, $attr);
    }

    return wantarray ? $attrs->values() : $attrs;
}


sub getAttr($@) {
    my $this = shift;

    my $attrs = $this->{_attrs};

    my @attrs = map($attrs->at($_), @_);

    wantarray && return @attrs;

    1 == @_ && return $attrs[0];

    return \@attrs;
}


sub str($@) {
    my $this = shift;

    my $id    = $this->id();
    my $ident = $this->{_ident};
    my @attrs = $this->{_attrs};

    local $" = ", ";

    return ref($this) . "($id, \"$ident\", attributes: @attrs)";
}


sub signature($) {
    my $this = shift;

    return $this->str();
}


sub xmlElementName { "symbol" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        id         => $this->id(),
        identifier => $this->ident(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    return $this->attr();
}


1;
