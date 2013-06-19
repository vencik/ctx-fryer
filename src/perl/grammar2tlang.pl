#!/usr/bin/perl

use strict;
use warnings;

use ProjectDef;

use Logging qw(:all);

# Default log level
$Logging::threshold = "INFO";

my $project_def = new ProjectDef($ARGV[0]) || exit(1);
my $metainfo    = $project_def->metaInfo() || exit(0);

print(join(" ", $metainfo->targetLanguages()), "\n");
