#!/usr/bin/perl

use strict;
use warnings;

use ProjectDef;

use Logging qw(:all);

# Default log level
$Logging::threshold = "INFO";

my $project_def = new ProjectDef($ARGV[0]) || exit(1);
my $unit_test   = $project_def->unitTest() || exit(0);
my @test_cases  = $unit_test->testCase();

foreach my $test_case (@test_cases) {
    # Test case label
    print($test_case->label(), "\n");

    # Compressed test case info
    my $info  = $test_case->accept() ? '*' : '+';
    my @deriv = $test_case->derivation();

    $info .= ' ' . length($test_case->word());
    $info .= ' ' . (@deriv ? scalar(@deriv) : -1);

    print($info, "\n");

    # Word
    print($test_case->word(), "\n");

    # Derivation
    if (@deriv) {
        print(join(" ", @deriv), "\n");
    }
}
