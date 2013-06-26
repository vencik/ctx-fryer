package CTXFryer::ProjectDef::Grammar::Attribute;

use strict;
use warnings;

use vars qw(
    @ISA
    @EXPORT_OK
    %EXPORT_TAGS
    );

require Exporter;

@ISA = qw(Exporter CTXFryer::Base);

@EXPORT_OK = qw(
    ATTR_QUALIFIER_IMPLICIT
    ATTR_QUALIFIER_CLASS
    ATTR_QUALIFIER_INSTANCE
    parseName
    );

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
    );

use CTXFryer::Logging qw(:all);

use constant ATTR_QUALIFIER_IMPLICIT => 0;
use constant ATTR_QUALIFIER_CLASS    => 1;
use constant ATTR_QUALIFIER_INSTANCE => 2;


sub parseName($) {
    my $name = shift;

    my ($identifier, $qualifier, $qualifier_type);

    if ($name =~ /^[A-Za-z_]\w*$/) {
        $identifier     = $name;
        $qualifier_type = ATTR_QUALIFIER_IMPLICIT;
    }
    elsif ($name =~ /^([A-Za-z_]\w*)::([A-Za-z_]\w*)$/) {
        $qualifier      = $1;
        $identifier     = $2;
        $qualifier_type = ATTR_QUALIFIER_CLASS;
    }
    elsif ($name =~ /^\$(\d+)\.([A-Za-z_]\w*)$/) {
        $qualifier      = $1;
        $identifier     = $2;
        $qualifier_type = ATTR_QUALIFIER_INSTANCE;
    }
    else {
        ERROR("Parse error: Name \"%s\" is ani invalid name specification", $name);
    }

    return ($identifier, $qualifier, $qualifier_type);
}


sub parseFunction($) {
    my $name = shift;

    # Empty name accepted as an implicit reference
    "" eq $name && return "";

    my ($identifier, $qualifier, $qualifier_type) = parseName($name);

    if (!defined $identifier) {
        ERROR("Parse error: Function name \"%s\" is invalid", $name);

        return;
    }

    if (ATTR_QUALIFIER_IMPLICIT != $qualifier_type) {
        ERROR("Function \"$name\" qualification unacceptable", $name);

        return;
    }

    return $identifier;
}


sub new($) {
    my $class = shift; $class = ref $class || $class;

    @_ % 2 && FATAL("INTERNAL ERROR: Invalid arguments: %s", \@_);

    my %this = @_;

    my $name = $this{name};

    # Check mandatory attributes
    defined $name || FATAL("INTERNAL ERROR: Name not defined");

    my $this = \%this;

    bless($this, $class);

    # Parse identifier
    my ($identifier, $qualifier, $qualifier_type) = parseName($name);

    if (!defined $identifier) {
        ERROR("Parse error: Attribute name \"%s\" is invalid", $name);

        return;
    }

    $this->{"attribute-identifier"} = $identifier;

    if (ATTR_QUALIFIER_IMPLICIT == $qualifier_type) {
        ERROR("Parse error: Attribute \"%s\" lacks qualification", $identifier);

        return;
    }
    elsif (ATTR_QUALIFIER_CLASS == $qualifier_type) {
        $this->{"symbol-identifier"} = $qualifier;
    }
    elsif (ATTR_QUALIFIER_INSTANCE == $qualifier_type) {
        $this->{"symbol-number"} = $qualifier;
    }
    else {
        FATAL("INTERNAL ERROR: Unknown qualifier type: %d", $qualifier_type);
    }

    return $this;
}


sub name($) {
    my $this = shift;

    return $this->{name};
}


sub identifier($) {
    my $this = shift;

    return $this->{"attribute-identifier"};
}


sub symID($) {
    my $this = shift;

    return $this->{"symbol-identifier"};
}


sub symNo($) {
    my $this = shift;

    return $this->{"symbol-number"};
}


sub function($) {
    my $this = shift;

    return $this->{function};
}


sub arguments($) {
    my $this = shift;

    return @{$this->{arguments}};
}


1;
