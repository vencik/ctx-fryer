# Traditional Perl Makefile creator

use strict;
use warnings;

use ExtUtils::MakeMaker;


WriteMakefile(
    NAME         => 'CTXFryer',
    VERSION_FROM => 'lib/CTXFryer.pm'
);
