package CTXFryer::Grammar::Symbol::Attribute::Function;

use strict;
use warnings;

use base qw(CTXFryer::Grammar::Symbol::Attribute::Evaluator);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Logging qw(:all);

use CTXFryer::List;


sub new($$$$@) {
    my $class = shift; $class = ref $class || $class;

    my $rule_id = shift;
    my $sym_no  = shift;

    my $this = $class->SUPER::new($rule_id, $sym_no);

    my $ident = shift;

    # Sanity checks
    defined $ident || FATAL("INTERNAL ERROR: Function identifier not defined");

    foreach my $arg (@_) {
        UNIVERSAL::isa($arg, "CTXFryer::Grammar::Symbol::Attribute::Dependency")
        or FATAL("INTERNAL ERROR: Function %s argument %s isn't an attribute dependency",
                 $ident, $arg);
    }

    $this->{_ident} = $ident;
    $this->{_arity} = scalar(@_);
    $this->{_args}  = new CTXFryer::List(@_);

    return bless($this, $class);
}


sub copy($$) {
    my ($orig, $rule_id_map) = @_;

    my $class = ref $orig;

    my $this = $orig->SUPER::copy($rule_id_map);

    $this->{_ident} = $orig->{_ident};
    $this->{_arity} = $orig->{_arity};

    my @orig_args = $orig->{_args}->items();

    my @args = map($_->copy(), @orig_args);

    $this->{_args}  = new CTXFryer::List(@args);

    return bless($this, $class);
}


sub ident($) {
    my $this = shift;

    return $this->{_ident};
}


sub arity($) {
    my $this = shift;

    return $this->{_arity};
}


sub arguments($) {
    my $this = shift;

    my $args = $this->{_args};

    return wantarray ? $args->items() : $args;
}


sub str($) {
    my $this = shift;

    my $rule_id = $this->ruleID() || '<any>';
    my $ident   = $this->{_ident};
    my $arity   = $this->{_arity};
    my $args    = $this->{_args};

    return ref($this) . "(\"$ident\", $arity args: $args, rule ID: $rule_id)";
}


sub xmlElementName { "function" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = (
        identifier => $this->{_ident},
        arity      => $this->{_arity},
    );

    my $rule_id = $this->ruleID();
    my $sym_no  = $this->symNo();

    $rule_id        and $attrs{"rule-id"}       = $rule_id;
    defined $sym_no and $attrs{"symbol-number"} = $sym_no;

    return %attrs;
}

sub xmlChildren($) {
    my $this = shift;

    my $args = $this->{_args};

    return xmlNewElement("arguments", {}, $args);
}


1;
