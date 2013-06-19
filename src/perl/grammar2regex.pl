#!/usr/bin/perl

use strict;
use warnings;

use ProjectDef;

use Logging qw(:all);

# Default log level
$Logging::threshold = "INFO";

my $project_def = new ProjectDef($ARGV[0])        || exit(1);
my $terminals   = $project_def->terminalSymbols() || exit(0);

my %terminals = $terminals->map();

while (my ($ident, $regex) = each %terminals) {
    print $ident, " = ", $regex, "\n";
}
