package Grammar::Symbol::Attribute::Reference;

use strict;
use warnings;

use base qw(Grammar::Symbol::Attribute::Evaluator);

use Serialised qw(:xml);

use Logging qw(:all);


sub new($$$$) {
    my $class = shift; $class = ref $class || $class;

    my $rule_id = shift;
    my $sym_no  = shift;

    my $this = $class->SUPER::new($rule_id, $sym_no);

    my $target = shift;

    # Sanity checks
    UNIVERSAL::isa($target, "Grammar::Symbol::Attribute::Dependency")
    or FATAL("INTERNAL ERROR: %s isn't an attribute dependency", $target);

    $this->{_target} = $target;

    return bless($this, $class);
}


sub copy($$) {
    my ($orig, $rule_id_map) = @_;

    my $this = $orig->SUPER::copy($rule_id_map);

    my $orig_target = $orig->{_target};

    $this->{_target} = $orig_target->copy();

    my $class = ref $orig;

    return bless($this, $class);
}


sub target($) {
    my $this = shift;

    return $this->{_target};
}


sub str($) {
    my $this = shift;

    my $rule_id = $this->ruleID() || "<any>";
    my $target  = $this->{_target};

    return ref($this) . "(ID: $target, rule ID: $rule_id)";
}


sub xmlElementName { "reference" }

sub xmlElementAttrs($) {
    my $this = shift;

    my $rule_id = $this->ruleID();
    my $sym_no  = $this->symNo();

    my %attrs = (
        "rule-id"       => $rule_id,
        "symbol-number" => $sym_no,
    );

    return %attrs;
}

sub xmlChildren($) {
    my $this = shift;

    my $target = $this->{_target};

    return xmlNewElement("target", {}, $target);
}


1;
