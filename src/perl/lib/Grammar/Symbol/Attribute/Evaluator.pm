package Grammar::Symbol::Attribute::Evaluator;

use strict;
use warnings;

use base qw(Base Serialised);

use Logging qw(:all);


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my $rule_id = shift;
    my $sym_no  = shift;

    defined $rule_id || FATAL("INTERNAL ERROR: Rule ID not defined");

    my $this = {
        _rule_id => $rule_id,
        _sym_no  => $sym_no,
    };

    return bless($this, $class);
}


sub copy($$) {
    my ($orig, $rule_id_map) = @_;

    my $orig_rule_id = $orig->{_rule_id};

    my $rule_id = '*';

    if ('*' ne $orig_rule_id) {
        my $rule = $rule_id_map->{$orig_rule_id}
        or FATAL("INTERNAL ERROR: Rule ID %s mapping not defined", $orig_rule_id);

        $rule_id = $rule->id();
    }

    my $this = {
        _rule_id => $rule_id,
        _sym_no  => $orig->{_sym_no},
    };

    my $class = ref $orig;

    return bless($this, $class);
}


sub ruleID($) {
    my $this = shift;

    my $rule_id = $this->{_rule_id};

    '*' eq $rule_id && return;

    return $rule_id;
}


sub symNo($) {
    my $this = shift;

    '*' eq $this->{_rule_id} && return;

    return $this->{_sym_no};
}


1;
