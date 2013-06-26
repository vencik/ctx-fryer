package CTXFryer::Math::Function::Unary;

use strict;
use warnings;

use base qw(CTXFryer::Math::Function);

use CTXFryer::Math::Set;
use CTXFryer::Math::Relation;
use CTXFryer::Math::Digraph;

use CTXFryer::Logging qw(:all);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    @_ % 2
    and FATAL("INTERNAL ERROR: Odd number of arguments: %d", scalar(@_));

    my @def;

    while (@_) {
        my $arg = shift;
        my $val = shift;

        push(@def, (new CTXFryer::Math::Multiplet($arg) => $val));
    }

    my $this = $class->SUPER::new(1, @def);

    return bless($this, $class);
}


sub definitionRange($) {
    my $this = shift;

    my $def_range_1plets = $this->SUPER::definitionRange();
    my @def_range_1plets = $def_range_1plets->items();

    return new CTXFryer::Math::Set(map($_->item(), @def_range_1plets));
}


sub project($$) {
    my ($this, $arg) = @_;

    return $this->SUPER::project(new CTXFryer::Math::Multiplet($arg));
}


sub _closure_kernel($$) {
    my ($arg, $this, $closure) = @_;

    my $arg_key = CTXFryer::Math::Set::itemKey($arg);

    # Sanity check
    exists $closure->{$arg_key}
    and FATAL("INTERNAL ERROR: Closure already defined for %arg", $arg);

    my $kernel = $this->project($arg);

    if (UNIVERSAL::isa($kernel, "CTXFryer::Math::Set")) {
        $closure->{$arg_key} = $kernel->copy();
    }
    else {
        $closure->{$arg_key} = new CTXFryer::Math::Set($kernel);
    }
}


sub _closure_growth($$$$) {
    my ($arg, $rel_arg, $this, $closure) = @_;

    my $arg_key     = CTXFryer::Math::Set::itemKey($arg);
    my $rel_arg_key = CTXFryer::Math::Set::itemKey($rel_arg);

    my $arg_vals     = $closure->{$arg_key};
    my $rel_arg_vals = $closure->{$rel_arg_key};

    # Sanity checks
    defined $arg_vals
    or FATAL("INTERNAL ERROR: No closure for %s", $arg);

    defined $rel_arg_vals
    or FATAL("INTERNAL ERROR: No closure for %s", $rel_arg);

    DEBUX(1, "%s = %s U %s", $arg_vals, $arg_vals, $rel_arg_vals);

    $arg_vals->unionAssign($rel_arg_vals);

    DEBUX(1, "Result: %s", $arg_vals);
}


sub _closure_rel_cycle($$$$) {
    my ($arg, $rel_arg, $this, $closure) = @_;

    my $arg_key     = CTXFryer::Math::Set::itemKey($arg);
    my $rel_arg_key = CTXFryer::Math::Set::itemKey($rel_arg);

    my $arg_vals = $closure->{$arg_key};

    # Sanity checks
    defined $arg_vals
    or FATAL("INTERNAL ERROR: No closure for %s", $arg);

    $closure->{$rel_arg_key} = $arg_vals->copy();
}


sub closure($@) {
    my $this = shift;
    my $rel  = shift || $this;
    my $sccs = shift;

    # Sanity checks
    UNIVERSAL::isa($rel, "CTXFryer::Math::Relation")
    or FATAL("INTERNAL ERROR: %s isn't a relation", $rel);

    2 == $rel->arity()
    or FATAL("INTERNAL ERROR: %s isn't a binary relation", $rel);

    my $def_range = $this->definitionRange();

    my $digraph = new CTXFryer::Math::Digraph($def_range, $rel);

    my %closure;

    my @sccs = $digraph->determineSCC(
        \&_closure_kernel,
        \&_closure_growth,
        \&_closure_rel_cycle,
        $this, \%closure);

    if (defined $sccs) {
        @$sccs = @sccs;
    }

    my @closure_def = map {
        my $key = CTXFryer::Math::Set::itemKey($_);
        my $val = $closure{$key};

        ($_ => $val)
    }
    $def_range->items();

    return $this->new(@closure_def);
}


1;
