package CTXFryer::Grammar::Symbol::Attribute::Dependency;

use strict;
use warnings;

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Logging qw(:all);

require Exporter;

use vars qw(
    @ISA
    @EXPORT_OK
    %EXPORT_TAGS
    $undef
);


@ISA = qw(Exporter CTXFryer::Serialised);

@EXPORT_OK = qw(
    $undef
);

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
);


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _attr_ident => shift,
        _sym_no     => shift,
    };

    return bless($this, $class);
}


BEGIN {
    $undef = new(__PACKAGE__, undef);
}


sub _isUndef($) {
    my $this = shift;

    return !defined $this->{_attr_ident};
}


sub copy($) {
    my $orig = shift;

    my $this = {
        _attr_ident => $orig->{_attr_ident},
        _sym_no     => $orig->{_sym_no},
    };

    my $class = ref $orig;

    return bless($this, $class);
}


sub attributeIdent() {
    my $this = shift;

    return $this->{_attr_ident};
}


sub symbolNo() {
    my $this = shift;

    return $this->{_sym_no};
}


sub str($) {
    my $this = shift;

    if ($this->_isUndef()) {
        return ref($this) . "(undefined)";
    }

    my $attr_ident = $this->{_attr_ident};
    my $sym_no     = $this->{_sym_no};

    defined $sym_no or $sym_no = "<undef>";

    return ref($this) . "(attr identifier: $attr_ident, sym. no: $sym_no)";
}


sub xmlElementName { "dependency" }

sub xmlElementAttrs($) {
    my $this = shift;

    $this->_isUndef() && return {
        undefined => "true",
    };

    my $attr_ident = $this->{_attr_ident};
    my $sym_no     = $this->{_sym_no};

    my %attrs = (
        "attribute-identifier" => $attr_ident,
    );

    defined $sym_no and $attrs{"symbol-number"} = $sym_no;

    return %attrs;
}


1;
