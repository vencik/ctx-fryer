package LRParser::Action::Accept;

use strict;
use warnings;

use base qw(LRParser::Action);

use LRParser::Action;


# Accept action singleton
our $acc;


sub new($) { return $acc; }


sub xmlElementName { "accept" }


bless($acc = new LRParser::Action);

1;
