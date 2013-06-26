package CTXFryer::FSA::State;

use strict;
use warnings;

use base qw(CTXFryer::Identified CTXFryer::Serialised);

use overload (
    "==" => "equal",
    "!=" => "notEqual",
);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Logging qw(:all);

use CTXFryer::FSA::Branch;


our $xml_indent;


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $desc = shift;

    my $this = $class->SUPER::new();

    $this->{_desc}    = $desc;
    $this->{_accepts} = [];
    $this->{_branch}  = [];

    return bless($this, $class);
}


sub copy($) {
    my $orig = shift;

    DEBUX(2, "Original: %s", $orig);

    my $class = ref $orig;

    my $this = $class->SUPER::new();

    $this->{_desc}    = $orig->desc();
    $this->{_accepts} = [ $orig->accepts() ];
    $this->{_branch}  = [];

    bless($this, $class);

    DEBUX(2, "Copy FSA state: ", $this);

    return $this;
}


sub desc($) {
    my $this = shift;

    return $this->{_desc};
}


sub accepts($@) {
    my ($this, $accepts) = @_;

    # Keep accepted word classes unique and sorted
    if ($accepts) {
        my %accepts = map(($_ => 1), @$accepts);

        my @accepts = sort keys(%accepts);

        $this->{_accepts} = \@accepts;
    }

    return @{$this->{_accepts}};
}


sub cmpByAccept($$) {
    my ($state1, $state2) = @_;

    my @accept1 = $state1->accepts();
    my @accept2 = $state2->accepts();

    # Non-equal amount of accepted word classes, compare by amount
    @accept1 == @accept2 || return @accept1 - @accept2;

    # First accepted word classes that differ decide comparison
    while (@accept1) {
        my $class1 = shift @accept1;
        my $class2 = shift @accept2;

        my $cmp = $class1 cmp $class2;

        $cmp && return $cmp;
    }

    # Equal accepted word classes
    return 0;
}


sub branch($@) {
    my ($this, $branches) = @_;

    if ($branches) {
        $this->{_branch} = [];

        foreach my $branch (@$branches) {
            my $symbol = $branch->symbol();

            unless ($symbol->isEmpty()) {
                foreach my $b (@{$this->{_branch}}) {
                    my $sym = $b->symbol();

                    CTXFryer::FSA::Symbol::Set::intersection($symbol, $sym)->isEmpty()
                    or FATAL("Branch %s symbol set collides with branch %s", $branch, $b);
                }

                push(@{$this->{_branch}}, $branch);
            }
        }
    }

    return @{$this->{_branch}};
}


sub trans($$) {
    my ($this, $symbol) = @_;

    foreach my $branch ($this->branch()) {
        $branch->symbol()->match($symbol) && return $branch->target();
    }

    return;
}


sub equal($$) {
    my ($state1, $state2) = @_;

    return $state1->id() eq $state2->id();
}


sub notEqual($$) {
    my ($state1, $state2) = @_;

    return !equal($state1, $state2);
}


sub str($) {
    my $this = shift;

    my $id       = $this->id();
    my $desc     = $this->desc();
    my @accepts  = $this->accepts();
    my @branches = $this->branch();

    local $" = ", ";

    return ref($this) . "($id, description: \"$desc\", accepts: (@accepts), branches: (@branches))";
}


sub xmlElementName { "fsa-state" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        id => $this->id(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    return (
        xmlNewElement("description", {}, $this->desc()),
        map(xmlNewElement("accepts", {}, $_), $this->accepts()),
        $this->branch()
    );
}


1;
