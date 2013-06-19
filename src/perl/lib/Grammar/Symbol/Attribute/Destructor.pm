package Grammar::Symbol::Attribute::Destructor;

use strict;
use warnings;

use base qw(Base Serialised);

use Serialised qw(:xml);

use Logging qw(:all);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $ident = shift;

    my $this = {
        _ident => $ident,
    };

    return bless($this, $class);
}


sub copy($) {
    my $orig = shift;

    my $this = {
        _ident => $orig->{_ident},
    };

    my $class = ref $orig;

    return bless($this, $class);
}


sub identifier($) {
    my $this = shift;

    return $this->{_ident};
}


sub str($) {
    my $this = shift;

    my $ident = $this->{_ident};

    return ref($this) . "(\"$ident\")";
}


sub xmlElementName { "destructor" }


sub xmlChildren($) {
    my $this = shift;

    my $func = xmlNewElement("function", {
        identifier => $this->{_ident},
    });

    return ($func);
}


1;
