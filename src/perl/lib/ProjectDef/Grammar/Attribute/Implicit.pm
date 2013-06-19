package ProjectDef::Grammar::Attribute::Implicit;

use strict;
use warnings;

use base qw(ProjectDef::Grammar::Attribute);

use ProjectDef::Grammar::Attribute qw(:all);

use Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new(@_) || return;

    # Check mandatory attributes
    defined $this->{"rule-no"} || defined $this->{"rule-id"}
    or FATAL("INTERNAL ERROR: Rule number not defined");

    # Parse function
    my $func = $this->{function} || "";

    $func = ProjectDef::Grammar::Attribute::parseFunction($func);

    if (!defined $func) {
        ERROR("Parse error: function name \"%s\" is invalid", $func);

        return;
    }

    $this->{function} = $func;

    # Parse arguments
    my @args = map {
        my ($ident, $qf, $qf_type) = ProjectDef::Grammar::Attribute::parseName($_);

        if (!defined $ident) {
            ERROR("Parse error: attribute name \"%s\" is invalid", $_);

            return;
        }

        if ($qf_type != ATTR_QUALIFIER_INSTANCE) {
            ERROR("Argument \"%s\" qualification is unacceptable", $_);

            return;
        }

        [$qf, $ident];

    } @{$this->{arguments}};

    $this->{arguments} = [ @args ];

    return bless($this, $class);
}


sub ruleID($@) {
    my ($this, $id) = @_;

    if (defined $id) {
        defined $this->{"rule-id"}
        and FATAL("INTERNAL ERROR: Redefinition of attribute rule ID (from %s to %s)",
                  $this->{"rule-id"}, $id);

        $this->{"rule-id"} = $id;
    }
    else {
        $id = $this->{"rule-id"};
    }

    return $id;
}


sub ruleNo($@) {
    my ($this, $no) = @_;

    if (defined $no) {
        defined $this->{"rule-no"}
        and WARN("Redefinition of rule number (from %d to %d)",
                 $this->{"rule-no"}, $no);

        $this->{"rule-no"} = $no;
    }
    else {
        $no = $this->{"rule-no"};
    }

    return $no;
}


1;
