package LRParser::Transition;

use strict;
use warnings;

use base qw(Math::Couple);

use Logging qw(:all);


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_);

    return bless($this, $class);
}


sub state($);       *state = "Math::Couple::first";
sub nonTerminal($); *nonTerminal = "Math::Couple::second";


1;
