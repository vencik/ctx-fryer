package CTXFryer::Grammar::Terminal::EOF;

use strict;
use warnings;

use base qw(CTXFryer::Grammar::Terminal);

use CTXFryer::Grammar::Terminal;


our $eof;

BEGIN {
    $eof = new CTXFryer::Grammar::Terminal('$', '$');
}


sub new($) {
    my $class = shift; $class = ref $class || $class;

    # EOF is a singleton
    return $eof;
}


1;
