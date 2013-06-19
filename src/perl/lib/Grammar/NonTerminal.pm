package Grammar::NonTerminal;

use strict;
use warnings;

use base qw(Grammar::Symbol);

use Grammar::Symbol;


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_);

    $this->{_LHS_rules}   = [];
    $this->{_RHS_rules}   = [];
    $this->{_is_nullable} = undef;

    return bless($this, $class);
}


sub _rules($$) {
    my $this = shift;
    my $key  = shift;

    return @{$this->{$key}};
}


sub _bindRules($$@) {
    my $this = shift;
    my $key  = shift;

    push(@{$this->{$key}}, @_);
}


sub LHSrules($) {
    my $this = shift;

    return $this->_rules("_LHS_rules");
}


sub bindLHSrules($) {
    my $this = shift;

    return $this->_bindRules("_LHS_rules", @_);
}


sub RHSrules($) {
    my $this = shift;

    return $this->_rules("_RHS_rules");
}


sub bindRHSrules($) {
    my $this = shift;

    return $this->_bindRules("_RHS_rules", @_);
}


sub isNullable($@) {
    my $this = shift;

    @_ and $this->{_is_nullable} = shift;

    return $this->{_is_nullable};
}


sub xmlElementName { "non-terminal-symbol" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = $this->SUPER::xmlElementAttrs();

    $attrs{"is-nullable"} = $this->isNullable() ? "true" : "false";

    return %attrs;
}


1;
