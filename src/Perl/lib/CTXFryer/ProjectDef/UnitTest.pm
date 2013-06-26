package CTXFryer::ProjectDef::UnitTest;

use strict;
use warnings;

use base qw(CTXFryer::Base);

use CTXFryer::ProjectDef qw(:all);

use CTXFryer::ProjectDef::UnitTest::TestCase;

use CTXFryer::Logging qw(:all);


use constant UNIT_TEST_SCOPE => 0;
use constant TEST_CASE_SCOPE => 1;
use constant WORD_SCOPE      => 2;
use constant DERIV_SCOPE     => 3;


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        test_cases => [],
        scope      => UNIT_TEST_SCOPE,
    };

    return bless($this, $class);
}


sub testCase($@) {
    my ($this, $no) = @_;

    defined $no || return @{$this->{test_cases}};

    return $this->{test_cases}->[$no];
}


sub parse($$$) {
    my ($this, $line, $pos) = @_;

    my $ret   = 0;
    my $scope = $this->{scope};

    # Test case
    if (UNIT_TEST_SCOPE == $scope && /^\s*TestCase\s*$/i) {
        $scope = TEST_CASE_SCOPE;

        push(@{$this->{test_cases}},
            new CTXFryer::ProjectDef::UnitTest::TestCase);
    }

    # Label
    elsif (TEST_CASE_SCOPE == $scope && /^\s*label\s*=\s*(.*)\s*$/i) {
        $this->{test_cases}->[-1]->label($1);
    }

    # Word
    elsif (TEST_CASE_SCOPE == $scope && /^\s*word\s*=\s*"(.*)"\s*$/i) {
        $this->{test_cases}->[-1]->word(CTXFryer::ProjectDef::unescape($1));
    }

    # Multi-line word
    elsif (TEST_CASE_SCOPE == $scope && /^\s*Word\s*$/i) {
        $scope = WORD_SCOPE;
    }

    # Multi-line word end
    elsif (WORD_SCOPE == $scope && /^\s*WordEnd\s*$/i) {
        $scope = TEST_CASE_SCOPE;
    }

    # Multi-line word definition
    elsif (WORD_SCOPE == $scope) {
        $this->{test_cases}->[-1]->word(CTXFryer::ProjectDef::unescape($_));
    }

    # Derivation
    elsif (TEST_CASE_SCOPE == $scope && /^\s*derivation\s*=\s*(([0-9]+\s?){1,})\s*$/i) {
        $this->{test_cases}[-1]->derivation(CTXFryer::ProjectDef::str2list(" ", $1));
    }

    # Derivation (long list spanning multiple lines)
    elsif (TEST_CASE_SCOPE == $scope && /^\s*Derivation\s*$/i) {
        $scope = DERIV_SCOPE;
    }

    # Derivation end
    elsif (DERIV_SCOPE == $scope && /^\s*DerivationEnd\s*$/i) {
        $scope = TEST_CASE_SCOPE;
    }

    # Derivation definition
    elsif (DERIV_SCOPE == $scope) {
        my $line = chomp($_);

        $this->{test_cases}->[-1]->derivation(CTXFryer::ProjectDef::str2list(", ", $line));
    }

    # Accept flag
    elsif (TEST_CASE_SCOPE == $scope && /^\s*accept\s*=\s*(.*)\s*$/i) {
        $this->{test_cases}->[-1]->accept(CTXFryer::ProjectDef::str2bool($1));
    }

    # Test case end
    elsif (TEST_CASE_SCOPE == $scope && /^\s*TestCaseEnd\s*$/i) {
        $scope = UNIT_TEST_SCOPE;
    }

    # Parse error
    else {
        ERROR("Unit test syntax error at %s", $pos);

        $ret = 1;
    }

    $this->{scope} = $scope;

    return $ret;
}


1;
