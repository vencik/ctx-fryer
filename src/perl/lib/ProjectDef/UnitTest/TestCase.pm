package ProjectDef::UnitTest::TestCase;

use strict;
use warnings;

use base qw(Base);

use Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        label      => undef,
        word       => "",
        accept     => 1,
        derivation => undef,
    };

    return bless($this, $class);
}


sub label($@) {
    my $this = shift;

    if (@_) {
        1 == @_ || FATAL("INTERNAL ERROR: Invalid arguments: %s", \@_);

        my $label = shift;

        defined $this->{label}
        and WARN("Unit test case \"%s\" renamed to \"%s\"", $this->{label}, $label);

        $this->{label} = $label;
    }

    return $this->{label};
}


sub word($@) {
    my $this = shift;

    while (@_) {
        $this->{word} .= shift;
    }

    return $this->{word};
}


sub accept($@) {
    my $this = shift;

    if (@_) {
        1 == @_ || FATAL("INTERNAL ERROR: Invalid arguments: %s", \@_);

        $this->{accept} = shift;
    }

    return $this->{accept};
}


sub derivation($@) {
    my $this = shift;

    if (@_) {
        defined $this->{derivation}
        or $this->{derivation} = [];

        push(@{$this->{derivation}}, @_);
    }

    return @{$this->{derivation}};
}


1;
