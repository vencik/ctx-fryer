package CTXFryer::LRParser::Action::Accept;

use strict;
use warnings;

use base qw(CTXFryer::LRParser::Action);

use CTXFryer::LRParser::Action;


# Accept action singleton
our $acc;


sub new($) { return $acc; }


sub xmlElementName { "accept" }


bless($acc = new CTXFryer::LRParser::Action);

1;
