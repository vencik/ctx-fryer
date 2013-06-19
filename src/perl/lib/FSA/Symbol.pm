package FSA::Symbol;

use strict;
use warnings;

use base qw(Base Serialised);

use Logging qw(:all);

use overload (
    "==" => "equal",
    "!=" => "notEqual",
    '<'  => "less",
    "<=" => "lessOrEqual",
    ">=" => "greaterOrEqual",
    '>'  => "greater",
);


our $xml_indent;


sub specChain($) {
    my $arg = shift;

    my $class = ref $arg || $arg;

    my @isa = eval("(@" . $class . "::ISA, \"$class\")");

    $@ and FATAL("%s is not an object nor class: %s", $arg, $@);

    return @isa;
}


sub commonAncestor($$) {
    my ($arg1, $arg2) = @_;

    my @isa1 = specChain($arg1);
    my @isa2 = specChain($arg2);

    DEBUX(5, "%s is a @isa1", $arg1);
    DEBUX(5, "%s is a @isa2", $arg2);

    my $class;

    while (@isa1 && @isa2) {
        my $isa1 = shift(@isa1);
        my $isa2 = shift(@isa2);

        $isa1 eq $isa2 or last;

        $class = $isa1;
    }

    DEBUX(5, "%s and %s common ancestor is %s", $arg1, $arg2, $class || "not defined");

    return $class;
}


sub compatibleBinOp($$$) {
    my ($op, $arg1, $arg2) = @_;

    my $common_ancestor = commonAncestor($arg1, $arg2);

    $common_ancestor or FATAL("%s and %s have no common ancestor", $arg1, $arg2);

    eval("\\&$common_ancestor" . "::$op") eq eval("\\&FSA::Symbol::$op")
    and FATAL("%s(%s, %s) is not defined", $op, $arg1, $arg2);

    my $scalar;
    my @array;

    if (wantarray) {
        @array = eval($common_ancestor . "::$op(\$arg1, \$arg2)");
    }
    else {
        $scalar = eval($common_ancestor . "::$op(\$arg1, \$arg2)");
    }

    $@ and FATAL("%s::%s(%s, %s) failed: %s", $common_ancestor, $op, $arg1, $arg2, $@);

    return wantarray ? @array : $scalar;
}


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $this = $class->SUPER::new();

    $this->{char} = shift;

    bless($this, $class);

    DEBUX(5, "%s created", $this);

    return $this;
}


sub copy($) {
    my $this = shift;

    my $class = ref($this);

    return $class->new($this->{char});
}


sub first($) {
    my $class = shift; $class = ref $class || $class;

    FATAL("$class first symbol not defined");
}


sub last($) {
    my $class = shift; $class = ref $class || $class;

    FATAL("$class last symbol not defined");
}


sub str($) {
    my $this = shift;

    my $class = ref $this;

    FATAL("$class stringifisation not defined");
}


sub equal($$) {
    my ($sym1, $sym2) = @_;

    return compatibleBinOp("equal", $sym1, $sym2);
}


sub notEqual($$) {
    my ($sym1, $sym2) = @_;

    return !equal($sym1, $sym2);
}


sub less($$) {
    my ($sym1, $sym2) = @_;

    return compatibleBinOp("less", $sym1, $sym2);
}


sub lessOrEqual($$) {
    my ($sym1, $sym2) = @_;

    $sym1 < $sym2 && return 1;

    $sym1 == $sym2 && return 1;

    return;
}


sub greater($$) {
    my ($sym1, $sym2) = @_;

    return !lessOrEqual($sym1, $sym2);
}


sub greaterOrEqual($$) {
    my ($sym1, $sym2) = @_;

    return !less($sym1, $sym2);
}


sub pred($) {
    my $this = shift;

    FATAL(ref($this) . " predecessor function not defined");
}


sub succ($) {
    my $this = shift;

    FATAL(ref($this) . " successor function not defined");
}


sub distance($$) {
    my ($sym1, $sym2) = @_;

    FATAL(ref($sym1) . " to " . ref($sym2) . " distance is not defined");
}


1;
