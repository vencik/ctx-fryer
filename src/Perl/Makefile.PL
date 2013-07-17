# Traditional Perl Makefile creator

use strict;
use warnings;

use ExtUtils::MakeMaker;


# Makefile postamble
sub MY::postamble {
    return <<HERE;
# For compatibility with GNU autotools-generated Makefiles targets
check: test

HERE
}


WriteMakefile(
    NAME         => "CTXFryer",
    VERSION_FROM => "lib/CTXFryer.pm",
    EXE_FILES    => [ glob("ctx-fryer-*") ]
);
