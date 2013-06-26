package CTXFryer::LRParser::Reduction;

use strict;
use warnings;

use base qw(CTXFryer::Math::Couple);

use CTXFryer::Logging qw(:all);


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_);

    return bless($this, $class);
}


sub state($); *state = "CTXFryer::Math::Couple::first";
sub rule($);  *rule  = "CTXFryer::Math::Couple::second";


1;
