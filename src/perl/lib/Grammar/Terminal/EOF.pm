package Grammar::Terminal::EOF;

use strict;
use warnings;

use base qw(Grammar::Terminal);

use Grammar::Terminal;


our $eof;

BEGIN {
    $eof = new Grammar::Terminal('$', '$');
}


sub new($) {
    my $class = shift; $class = ref $class || $class;

    # EOF is a singleton
    return $eof;
}


1;
