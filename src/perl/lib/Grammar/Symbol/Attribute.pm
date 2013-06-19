package Grammar::Symbol::Attribute;

use strict;
use warnings;

require Exporter;

use Serialised qw(:xml);

use Grammar::Symbol::Attribute::Evaluator;

use Logging qw(:all);

use List;

use constant ATTR_TYPE_INHERITED  => 0;
use constant ATTR_TYPE_AGGREGATED => 1;

use vars qw(
    @ISA
    @EXPORT_OK
    %EXPORT_TAGS
    );

@ISA = qw(Identified Serialised Exporter);

@EXPORT_OK = qw(
    ATTR_TYPE_INHERITED
    ATTR_TYPE_AGGREGATED
    );

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
    );


sub new($$$@) {
    my $class = shift; $class = ref $class || $class;

    my $type  = shift;
    my $ident = shift;

    @_ % 2
    and FATAL("INTERNAL ERROR: Invalid attribute \"%s\" constructor argument count", $ident);

    my %args = @_;

    my (@evals, $destr);

    exists $args{evaluators} and @evals = @{$args{evaluators}};
    exists $args{destructor} and $destr = $args{destructor};

    # Sanity checks
    ATTR_TYPE_INHERITED == $type || ATTR_TYPE_AGGREGATED == $type
    or FATAL("INTERNAL ERROR: Unexpected attribute type: %d", $type);

    foreach my $eval (@evals) {
        UNIVERSAL::isa($eval, "Grammar::Attribute::Evaluator")
        or FATAL("INTERNAL ERROR: Unexpected evaluator type: %s", ref($eval));
    }

    if (defined $destr) {
        UNIVERSAL::isa($destr, "Grammar::Attribute::Destructor")
        or FATAL("INTERNAL ERROR: Unexpected destructor type: %s", ref($destr));
    }

    my $this = new Identified;

    $this->{_type}  = $type;
    $this->{_ident} = $ident;
    $this->{_eval}  = new List(@_);
    $this->{_destr} = $destr;

    return bless($this, $class);
}


sub copy($$) {
    my ($orig, $rule_id_map) = @_;

    my $this = new Identified;

    $this->{_type}  = $orig->{_type};
    $this->{_ident} = $orig->{_ident};

    my @eval = $orig->{_eval}->items();

    @eval = map($_->copy($rule_id_map), @eval);

    $this->{_eval} = new List(@eval);

    my $destr = $orig->{_destr};

    $this->{_destr} = defined $destr ? $destr->copy() : $destr;

    my $class = ref $orig;

    return bless($this, $class);
}


sub type($) {
    my $this = shift;

    return $this->{_type};
}


sub isInherited($) {
    my $this = shift;

    return ATTR_TYPE_INHERITED == $this->{_type};
}


sub isAggregated($) {
    my $this = shift;

    return ATTR_TYPE_AGGREGATED == $this->{_type};
}


sub identifier($) {
    my $this = shift;

    return $this->{_ident};
}


sub evaluator($@) {
    my $this = shift;

    my $eval_list = $this->{_eval};

    @_ and $eval_list->push(@_);

    return wantarray ? $eval_list->items() : $eval_list;
}


sub destructor($@) {
    my $this = shift;

    @_ and $this->{_destr} = shift;

    return $this->{_destr};
}


sub str($) {
    my $this = shift;

    my $descr = $this->id() . ": \"" . $this->{_ident} . "\"";

    for my $key (sort grep("_ident" ne $_, keys %$this)) {
        my $val = $this->{$key};

        defined $val or $val = "<undef>";

        $descr .= ", " . $key . " => " . $val;
    }

    return ref($this) . "($descr)";
}


sub xmlElementName { "attribute" }

sub xmlElementAttrs($) {
    my $this = shift;

    my $type;

    if (ATTR_TYPE_INHERITED == $this->{_type}) {
        $type = "inherited";
    }
    elsif (ATTR_TYPE_AGGREGATED == $this->{_type}) {
        $type = "aggregated";
    }
    else {
        FATAL("INTERNAL ERROR: Unknown attribute type: %d", $this->{_type});
    }

    my %attrs = (
        id         => $this->id(),
        identifier => $this->{_ident},
        type       => $type,
    );

    return %attrs;
}

sub xmlChildren($) {
    my $this = shift;

    my @children = (
        xmlNewElement("evaluators", {}, $this->{_eval}),
    );

    my $destr = $this->destructor();

    defined $destr and push(@children, $destr);

    return @children;
}


1;
