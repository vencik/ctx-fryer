package LRParser::Action::Reduce;

use strict;
use warnings;

use base qw(LRParser::Action);

use LRParser::Action;

use Logging qw(:all);


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $rule = shift;

    defined $rule
    or FATAL("INTERNAL ERROR: Rule not defined");

    my $this = $class->SUPER::new();

    $this->{_rule} = $rule;

    return bless($this, $class);
}


sub rule($) {
    my $this = shift;

    return $this->{_rule};
}


sub str($) {
    my $this = shift;

    my $rule = $this->rule();

    return $this->SUPER::str($rule);
}


sub xmlElementName { "reduce" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = $this->SUPER::xmlElementAttrs();

    $attrs{"rule-number"} = $this->rule();

    return %attrs;
}


1;
