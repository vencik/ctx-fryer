package CTXFryer::ProjectDef::Grammar::Attribute::Explicit;

use strict;
use warnings;

use base qw(CTXFryer::ProjectDef::Grammar::Attribute);

use CTXFryer::ProjectDef::Grammar::Attribute qw(:all);

use CTXFryer::Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_) || return;

    # Check mandatory attributes
    defined $this->{"attribute-identifier"} || FATAL("INTERNAL ERROR: Attribute identifier not defined");
    defined $this->{"symbol-identifier"}    || FATAL("INTERNAL ERROR: Symbol identifier not defined");

    # Parse function
    my $func = $this->{function} || "";

    $func = CTXFryer::ProjectDef::Grammar::Attribute::parseFunction($func);

    if (!defined $func) {
        ERROR("Parse error: function name \"%s\" is invalid", $func);

        return;
    }

    $this->{function} = $func;

    # Parse arguments
    my @args = map {
        my ($ident, $qf, $qf_type) = CTXFryer::ProjectDef::Grammar::Attribute::parseName($_);

        if (!defined $ident) {
            ERROR("Parse error: attribute name \"%s\" is invalid", $_);

            return;
        }

        if ($qf_type != ATTR_QUALIFIER_IMPLICIT) {
            ERROR("Argument \"%s\" qualification is unacceptable", $_);

            return;
        }

        [$ident];

    } @{$this->{arguments}};

    $this->{arguments} = [ @args ];

    return bless($this, $class);
}


1;
