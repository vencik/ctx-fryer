package Grammar::Item;

use strict;
use warnings;

use base qw(Signed);

use Signed;

use Grammar::Rule;

use Logging qw(:all);

use overload (
    "<=>" => "cmp",
    "=="  => "isEqual",
    "!="  => "isNotEqual",
    );


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $rule = shift;

    UNIVERSAL::isa($rule, "Grammar::Rule")
    or FATAL("INTERNAL ERROR: %s isn't grammar rule", $rule);

    my $pos = shift || 0;

    my $rule_no = $rule->number();

    my $this = $class->SUPER::new("I($rule_no,$pos)");

    $this->{_rule} = $rule;
    $this->{_pos}  = $pos;

    return bless($this, $class);
}


sub rule($) {
    my $this = shift;

    return $this->{_rule};
}


sub nonTerminal($) {
    my $this = shift;

    return $this->rule()->left();
}


sub position($) {
    my $this = shift;

    return $this->{_pos};
}


sub symbol($) {
    my $this = shift;

    return $this->rule()->right($this->position());
}


sub isFinal($) {
    my $this = shift;

    return $this->position() == $this->rule()->length();
}


sub next($) {
    my $this = shift;

    $this->isFinal() && return;

    return $this->new($this->rule(), $this->position() + 1);
}


sub cmp($$) {
    my ($item1, $item2) = @_;

    my $sig1 = $item1->signature();
    my $sig2 = $item2->signature();

    return $sig1 cmp $sig2;
}


sub isEqual($$) {
    my ($item1, $item2) = @_;

    my $sig1 = $item1->signature();
    my $sig2 = $item2->signature();

    return $sig1 eq $sig2;
}


sub isNotEqual($$) {
    my ($item1, $item2) = @_;

    return !isEqual($item1, $item2);
}


sub xmlElementName { "item" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = (
        "rule-number" => $this->rule()->number(),
        "position"    => $this->position(),
        "is-final"    => $this->isFinal() ? "true" : "false",
    );

    my $symbol = $this->symbol();
    $symbol and$attrs{"symbol-id"} = $symbol->id();

    return %attrs;
}


1;
