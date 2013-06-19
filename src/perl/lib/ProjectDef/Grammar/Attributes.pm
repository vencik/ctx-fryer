package ProjectDef::Grammar::Attributes;

use strict;
use warnings;

use base qw(Base);

use ProjectDef qw(:all);

use ProjectDef::Grammar::Attribute::Explicit;
use ProjectDef::Grammar::Attribute::Implicit;

use Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _implicit => [],
        _explicit => [],
        _destruct => {}
    };

    return bless($this, $class);
}


sub implicit($) {
    my $this = shift;

    return @{$this->{_implicit}};
}


sub explicit($) {
    my $this = shift;

    return @{$this->{_explicit}};
}


sub destructor($@) {
    my $this = shift;

    my @destr;

    if (@_) {
        my @destr = map($this->{_destruct}->{$_}, @_);

        wantarray && return @destr;

        1 == @_ && return $destr[0];

        return @destr;
    }

    return values(%{$this->{_destruct}});
}


sub _add($$@) {
    my $this  = shift;
    my $class = shift;

    push(@{$this->{$class}}, @_);
}


sub addImplicit($@) {
    my $this = shift;

    return $this->_add("_implicit", @_);
}


sub addExplicit($@) {
    my $this = shift;

    return $this->_add("_explicit", @_);
}


sub addDestructor($$$) {
    my ($this, $attr_spec, $destr) = @_;

    defined $this->{_destruct}->{$attr_spec} && return;

    $this->{_destruct}->{$attr_spec} = $destr;

    return 1;
}


sub parse($$$) {
    my ($this, $line, $pos) = @_;

    my $error_cnt = 0;

    # Explicit attribute definition
    if ($line =~ /^\s*(\S*)\s*=\s*(\S*)\s*\((.*)\)\s*$/) {
        my $name = $1;
        my $func = $2;
        my @args = ProjectDef::str2list(", ", $3);

        my $attr = new ProjectDef::Grammar::Attribute::Explicit(
                       name      => $name,
                       function  => $func,
                       arguments => [ @args ],
                       );

        if ($attr) {
            $this->addExplicit($attr);
        }
        else {
            ERROR("Failed to parse explicit attribute definition");

            ++$error_cnt;
        }
    }

    # Implicit attribute definition
    elsif ($line =~ /^\s*(([A-Za-z_]\w*)|(\d+))\s*:\s*(\S*)\s*=\s*(\S*)\s*\((.*)\)\s*$/) {
        my $rule_id = $2;
        my $rule_no = $3;
        my $name    = $4;
        my $func    = $5;
        my @args    = ProjectDef::str2list(", ", $6);

        my $attr = new ProjectDef::Grammar::Attribute::Implicit(
                       name      => $name,
                       "rule-id" => $rule_id,
                       "rule-no" => $rule_no,
                       function  => $func,
                       arguments => [ @args ],
                       );

        if ($attr) {
            $this->addImplicit($attr);
        }
        else {
            ERROR("Failed to parse implicit attribute definition");

            ++$error_cnt;
        }
    }

    # Implicit attribute definition (reference)
    elsif ($line =~ /^\s*(([A-Za-z_]\w*)|(\d+))\s*:\s*(\S*)\s*=\s*(\S*)\s*$/) {
        my $rule_id = $2;
        my $rule_no = $3;
        my $name    = $4;
        my $orig    = $5;

        my $attr = new ProjectDef::Grammar::Attribute::Implicit(
                       name      => $name,
                       "rule-id" => $rule_id,
                       "rule-no" => $rule_no,
                       arguments => [ $orig ],
                       );

        if ($attr) {
            $this->addImplicit($attr);
        }
        else {
            ERROR("Failed to parse implicit reference definition");

            ++$error_cnt;
        }
    }

    # Attribute destructor
    elsif ($line =~ /^\s*~(\S*)\s*=\s*(\S*)\s*$/) {
        my $name = $1;
        my $func = $2;

        DEBUG("Attribute \"%s\" destructor \"%s\" defined", $name, $func);

        if (!$this->addDestructor($name, $func)) {
            ERROR("Attribute \"%s\" destructor redefined", $name);

            ++$error_cnt;
        }
    }

    # Parse error
    else {
        ERROR("Grammar attribute definition syntax error at %s", $pos);

        $error_cnt = 1;
    }

    return $error_cnt;
}


1;
