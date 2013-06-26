package CTXFryer::FSA::Symbol::Interval;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);

use CTXFryer::Serialised qw(:xml);

use CTXFryer::Logging qw(:all);

use CTXFryer::FSA::Symbol;


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _lo => undef,
        _hi => undef,
    };

    bless($this, $class);

    my $lo = shift;
    my $hi = shift;

    $lo and $this->lo($lo);
    $hi and $this->hi($hi);

    DEBUX(3, "%s created", $this);

    return $this;
}


sub copy($) {
    my $this = shift;

    return new CTXFryer::FSA::Symbol::Interval($this->lo(), $this->hi());
}


sub lo($@) {
    my $this = shift;

    if (@_) {
        my $lo = shift;

        DEBUX(5, "Setting lo bound to %s", $lo);

        UNIVERSAL::isa($lo, "CTXFryer::FSA::Symbol")
        or FATAL("$lo is not CTXFryer::FSA::Symbol");

        my $hi = $this->{_hi};

        $hi or $this->{_hi} = $hi = $lo;

        CTXFryer::FSA::Symbol::commonAncestor($lo, $hi)
        or FATAL("$lo and $hi are not compatible");

        $lo <= $hi or FATAL("$lo is not less or equal to $hi");

        $this->{_lo} = $lo;
    }

    return $this->{_lo};
}


sub hi($@) {
    my $this = shift;

    if (@_) {
        my $hi = shift;

        DEBUX(5, "Setting hi bound to %s", $hi);

        UNIVERSAL::isa($hi, "CTXFryer::FSA::Symbol")
        or FATAL("$hi is not CTXFryer::FSA::Symbol");

        my $lo = $this->{_lo};

        $lo or $this->{_lo} = $lo = $hi;

        CTXFryer::FSA::Symbol::commonAncestor($lo, $hi)
        or FATAL("$lo and $hi are not compatible");

        $lo <= $hi or FATAL("$lo is not less or equal to $hi");

        $this->{_hi} = $hi;
    }

    return $this->{_hi};
}


sub isEmpty($) {
    my $this = shift;

    return !defined $this->lo();
}


sub length($) {
    my $this = shift;

    my $lo = $this->lo();

    $lo or return 0;

    my $hi = $this->hi();

    $hi or FATAL("Lower bound $lo defined while higher is not");

    return $lo->distance($hi) + 1;
}


sub order(@) {
    # Order by lo
    return sort {
        my $cmp = 0;

        if ($a->isEmpty()) {
            unless ($b->isEmpty()) {
                $cmp = -1;
            }
        }
        elsif ($b->isEmpty()) {
            $cmp = 1;
        }
        else {
            if ($a->lo() < $b->lo()) {
                $cmp = -1;
            }
            elsif ($a->lo() != $b->lo()) {
                $cmp = 1;
            }
        }

        $cmp;
    } @_;
}


sub _intersection2($$) {
    my ($interval1, $interval2) = order(@_);

    # Empty (note that unless the 1st interval isn't empty then the second is neither)
    if ($interval1->isEmpty()) {
        return $interval1;
    }

    # Inclusive
    if ($interval2->hi() <= $interval1->hi()) {
        return $interval2;
    }

    # Disjoint
    if ($interval1->hi() < $interval2->lo()) {
        return new CTXFryer::FSA::Symbol::Interval();
    }

    # Overlay
    return new CTXFryer::FSA::Symbol::Interval($interval2->lo(), $interval1->hi());
}


sub intersection(@) {
    DEBUX(3, "Intersection of %s", \@_);

    @_ || return new CTXFryer::FSA::Symbol::Interval();

    my $intersection = shift;

    while (@_) {
        # Once the intersection gets empty it is empty
        $intersection->isEmpty() && last;

        $intersection = _intersection2($intersection, shift);
    }

    DEBUX(3, "Result: %s", $intersection);

    return $intersection;
}


sub _union2($$) {
    my ($interval1, $interval2) = order(@_);

    # Empty (note that unless the 1st interval isn't empty then the second is neither)
    if ($interval1->isEmpty()) {
        return ($interval2);
    }

    my $hi1_next = $interval1->hi()->succ();

    # 1st interval spans to base last
    unless ($hi1_next) {
        return ($interval1);
    }

    # Disjoint
    if ($hi1_next < $interval2->lo()) {
        return ($interval1, $interval2);
    }

    # Overlay or extension
    my $lo = $interval1->lo();

    my $hi = $interval1->hi() < $interval2->hi()
           ? $interval2->hi()
           : $interval1->hi();

    return new CTXFryer::FSA::Symbol::Interval($lo, $hi);
}


sub union(@) {
    DEBUX(3, "Union of: %s", \@_);

    @_ || return new CTXFryer::FSA::Symbol::Interval();

    my @order = order(@_);

    my @union = (shift(@order));

    # Only union with the last interval from current union
    # is necessary because for each interval from the pre-ordered
    # list, the following hold:
    # 1/ $last->lo() <= $interval->lo()
    # 2/ $last->hi() <  $interval->lo()  =>  intersection($last, $interval) = 0
    foreach my $interval (@order) {
        my $last = pop(@union);

        push(@union, _union2($last, $interval));
    }

    DEBUX(3, "Result: %s", \@union);

    return @union;
}


sub _complement1($) {
    my $this = shift;

    DEBUX(3, "Complement of %s", $this);

    $this->isEmpty() && FATAL("%s is empty", $this);

    my $lo = $this->lo();
    my $hi = $this->hi();

    my $base = CTXFryer::FSA::Symbol::commonAncestor($lo, $hi);

    my $first = $base->first();
    my $last  = $base->last();

    my @complement;

    my $sym = $this->lo()->pred();

    defined $sym && push(@complement, new CTXFryer::FSA::Symbol::Interval($first, $sym));

    $sym = $this->hi()->succ();

    defined $sym && push(@complement, new CTXFryer::FSA::Symbol::Interval($sym, $last));

    DEBUX(3, "Result: %s", \@complement);

    return @complement;
}


sub complement(@) {
    DEBUX(3, "Complement of %s", \@_);

    @_ || FATAL("At least one interval required for complement");

    my $this = shift;

    my @co = $this->_complement1();

    foreach $this (@_) {
        my @sub_co = $this->_complement1();
        my @new_co;

        foreach my $co (@co) {
            foreach my $sub_co (@sub_co) {
                push(@new_co, intersection($co, $sub_co));
            }
        }

        @co = union(@new_co);
    }

    DEBUX(3, "Result: %s", \@co);

    return @co;
}


sub subtraction($@) {
    my $left = shift;

    DEBUX(3, "Subtraction of %s from %s", \@_, $left);

    # No need to subtract empty intervals
    my @right = grep(!$_->isEmpty(), @_);

    @right || return $left;

    my @right_co = complement(@right);

    DEBUX(3, "Complement of %s: %s", \@right, \@right_co);

    my @subtraction = map(intersection($left, $_), @right_co);

    @subtraction = union(@subtraction);

    DEBUX(3, "Result: %s", \@subtraction);

    return @subtraction;
}


sub match($$) {
    my ($this, $symbol) = @_;

    $this->isEmpty() && return;

    return $this->lo() <= $symbol && $symbol <= $this->hi();
}


sub str($) {
    my $this = shift;

    my $str = ref($this) . "(";

    unless ($this->isEmpty) {
        my $lo = $this->lo();
        my $hi = $this->hi();

        $str .= "$lo..$hi";
    }

    $str .= ")";

    return $str;
}


sub xmlElementName { "symbol-interval" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        empty  => $this->isEmpty() ? "true" : "false",
        length => $this->length(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    return (
        xmlNewElement("lower-bound",  {}, $this->lo()),
        xmlNewElement("higher-bound", {}, $this->hi()),
    );
}


1;
