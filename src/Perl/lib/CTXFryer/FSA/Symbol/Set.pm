package CTXFryer::FSA::Symbol::Set;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);

use CTXFryer::Logging qw(:all);

use CTXFryer::FSA::Symbol::Interval;

use overload (
    "==" => "equal",
);


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _interval => [ new CTXFryer::FSA::Symbol::Interval() ],
    };

    bless($this, $class);

    $this->add(@_);

    DEBUX(2, "%s created", $this);

    return $this;
}


sub _interval($@) {
    my $this = shift;

    if (@_) {
        $this->{_interval} = [ @_ ];
    }

    return @{$this->{_interval}};
}


sub isEmpty($) {
    my $this = shift;

    my @interval = $this->_interval();

    @interval || FATAL("At least an empty interval must be set in %s", $this);

    foreach my $interval (@interval) {
        $interval->isEmpty() || return;
    }

    return 1;
}


sub copy($) {
    my $this = shift;

    my @interval = map($_->copy(), $this->_interval());

    return new CTXFryer::FSA::Symbol::Set(@interval);
}


sub _toInterval(@) {
    my @interval;

    foreach my $obj (@_) {
        ref($obj) or FATAL("%s is not an object", $obj);

        my $interval;

        if (UNIVERSAL::isa($obj, "CTXFryer::FSA::Symbol::Interval")) {
            $interval = $obj;

            DEBUX(5, "%s is an interval", $interval);
        }
        elsif (UNIVERSAL::isa($obj, "CTXFryer::FSA::Symbol")) {
            $interval = new CTXFryer::FSA::Symbol::Interval($obj);

            DEBUX(5, "%s transformed to %s", $obj, $interval);
        }
        else {
            FATAL("%s isn't a CTXFryer::FSA::Symbol::Interval nor a CTXFryer::FSA::Symbol", $obj);
        }

        push(@interval, $interval);
    }

    return @interval;
}


sub add($@) {
    my $this = shift;

    DEBUX(2, "Adding %s to %s", \@_, $this);

    my @add = _toInterval(@_);

    my @interval = CTXFryer::FSA::Symbol::Interval::union($this->_interval(), @add);

    $this->_interval(@interval);

    DEBUX(2, "Result: %s", $this);

    return $this;
}


sub remove($@) {
    my $this = shift;

    my @remove = _toInterval(@_);

    my @interval = map(CTXFryer::FSA::Symbol::Interval::subtraction($_, @remove), $this->_interval());

    return $this->_interval(@interval);
}


sub match($$) {
    my ($this, $symbol) = @_;

    foreach my $interval ($this->_interval()) {
        $interval->match($symbol) && return 1;
    }

    return;
}


sub union(@) {
    DEBUX(2, "Union of %s", \@_);

    my @interval_union = CTXFryer::FSA::Symbol::Interval::union(map($_->_interval(), @_));

    my $union = new CTXFryer::FSA::Symbol::Set(@interval_union);

    DEBUX(2, "Result: %s", $union);

    return $union;
}


sub _intersection2($$) {
    my ($set1, $set2) = @_;

    # Intersection with an empty set is empty
    $set1->isEmpty() && return $set1;
    $set2->isEmpty() && return $set2;

    my @intervals1 = $set1->_interval();
    my @intervals2 = $set2->_interval();

    my @intersect_intervals;

    foreach my $interval1 (@intervals1) {
        foreach my $interval2 (@intervals2) {
            my $intersect_interval = CTXFryer::FSA::Symbol::Interval::intersection($interval1, $interval2);

            push(@intersect_intervals, $intersect_interval);
        }
    }

    @intersect_intervals = CTXFryer::FSA::Symbol::Interval::union(@intersect_intervals);

    return new CTXFryer::FSA::Symbol::Set(@intersect_intervals);
}


sub intersection(@) {
    DEBUX(2, "Intersection of %s", \@_);

    my $intersect = shift || return new CTXFryer::FSA::Symbol::Set;

    foreach my $set (@_) {
        $intersect = _intersection2($intersect, $set);

        # Once intersection gets empty it stays empty
        $intersect->isEmpty() && last;
    }

    DEBUX(2, "Result: %s", $intersect);

    return $intersect;
}


sub subtraction($@) {
    my $this = shift;

    DEBUX(2, "Subtraction of %s from %s", \@_, $this);

    my @left_intervals = $this->_interval();

    my @right_intervals_union = CTXFryer::FSA::Symbol::Interval::union(map($_->_interval(), @_));

    DEBUX(2, "Subtracting union %s from %s", \@right_intervals_union, \@left_intervals);

    my @sub_intervals;

    foreach my $left_interval (@left_intervals) {
        DEBUX(2, "Subtracting from %s", $left_interval);

        my @left_sub = CTXFryer::FSA::Symbol::Interval::subtraction($left_interval, @right_intervals_union);

        DEBUX(2, "Sub-result: %s", \@left_sub);

        push(@sub_intervals, @left_sub);
    }

    @sub_intervals = CTXFryer::FSA::Symbol::Interval::union(@sub_intervals);

    my $subtract = new CTXFryer::FSA::Symbol::Set(@sub_intervals);

    DEBUX(2, "Result: %s", $subtract);

    return $subtract;
}


sub complement($) {
    my $this = shift;

    DEBUX(2, "Complement of %s", $this);

    my @co = CTXFryer::FSA::Symbol::Interval::complement($this->_interval());

    my $co = new CTXFryer::FSA::Symbol::Set(@co);

    DEBUX(2, "Result: %s", $co);

    return $co;
}


sub equal($$) {
    my ($set1, $set2) = @_;

    my $intersect = intersection($set1, $set2);

    my $sub = subtraction($set1, $intersect);

    $sub->isEmpty() || return;

    $sub = subtraction($set2, $intersect);

    $sub->isEmpty() || return;

    return 1;
}


sub str($) {
    my $this = shift;

    my @interval_str = map($_->str(), $this->_interval());

    local $" = ", ";

    return ref($this) . "(intervals: @interval_str)";
}


sub xmlElementName { "symbol-set" }

sub xmlChildren($) {
    my $this = shift;

    # Intervals are provided sorted by length in descending order
    return sort { $b->length() <=> $a->length(); } $this->_interval();
}


1;
