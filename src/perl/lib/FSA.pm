package FSA;

use strict;
use warnings;

use base qw(Identified Serialised);

use Serialised qw(:xml);

use Logging qw(:all);

use FSA::Symbol;
use FSA::Symbol::ASCII;
use FSA::Symbol::Interval;
use FSA::Symbol::Set;

use FSA::Branch;
use FSA::State;


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $arg = shift;

    UNIVERSAL::isa($arg, "FSA::Regex") && return FSA::Regex::fsa($arg);

    unless (defined $arg) {
        my @caller = caller(0);

        $arg = "anonymous from " . $caller[1] . ":" . $caller[2];
    }

    my $root = shift || new FSA::State($arg . " root");

    my $this = $class->SUPER::new();

    $this->{_desc} = $arg;
    $this->{_root} = $root;

    bless($this, $class);

    DEBUX(1, "%s created", $this);

    return $this;
}


sub desc($@) {
    my ($this, $desc) = @_;

    $desc and $this->{_desc} = $desc;

    return $this->{_desc};
}


sub root($) {
    my $this = shift;

    return $this->{_root};
}


sub accepts($$@) {
    my ($this, $word, $symbol_class) = @_;

    $symbol_class ||= "FSA::Symbol::ASCII";

    my @symbols = map($symbol_class->new($_), split("", $word));

    my $state = $this->root();

    while (@symbols) {
        my $symbol = shift @symbols;

        $state = $state->trans($symbol) || return;
    }

    return $state->accepts();
}


sub acceptsPrefix($$@) {
    my ($this, $word, $symbol_class) = @_;

    $symbol_class ||= "FSA::Symbol::ASCII";

    my $state = $this->root();

    DEBUG("Word: \"%s\" (symbol class %s)", $word, $symbol_class);

    my $accept_length = 0;
    my $accept_state;

    for (my $i = 0; $i < length($word);) {
        my $symbol = $symbol_class->new(substr($word, $i, 1));

        $state = $state->trans($symbol) || last;

        DEBUX(1, "Transition to state %s via symbol \"%s\" done",
                 $state, $symbol);

        ++$i;

        if ($state->accepts()) {
            $accept_length = $i;
            $accept_state  = $state;

            DEBUX(1, "State accepts (length %d)", $accept_length);
        }
    }

    DEBUG("Accepted length: %d", $accept_length);

    if (wantarray) {
        my @accept = ($accept_length);

        $accept_state && push(@accept, $accept_state->accepts());

        return @accept;
    }

    return $accept_length;
}


sub rejects($$@) {
    my ($this, $word, $symbol_class) = @_;

    return !$this->accepts($word, $symbol_class);
}


sub _unionBranch($$$$$$) {
    my ($state_queue, $state_set, $state, $symbol, $target1, $target2) = @_;

    my $target_key;
    my $target_desc;

    if (defined $target1) {
        if (defined $target2) {
            $target_key  = $target1->id() . "," . $target2->id();
            $target_desc = "U(" . $target1->desc() . ", " . $target2->desc() . ")";
        }
        else
        {
            $target_key  = $target1->id();
            $target_desc = $target1->desc();
        }
    }
    else
    {
        $target_key  = $target2->id();
        $target_desc = $target2->desc();
    }

    my $target = $state_set->{$target_key};

    # Create target state unless already exists
    unless ($target) {
        $target = new FSA::State($target_desc);

        push(@$state_queue, [ $target, $target1, $target2 ]);

        $state_set->{$target_key} = $target;

        DEBUX(2, "New state: %s", $target);
    }

    # Create branch
    my $branch = new FSA::Branch;

    $branch->symbol($symbol);
    $branch->target($target);

    $state->branch([$state->branch(), $branch]);

    DEBUX(2, "Result: %s", $state);
}


sub _union2($$) {
    my ($fsa1, $fsa2) = @_;

    my @state_queue;
    my %state_set;

    my $this = new FSA("U(" . $fsa1->desc() . ", " . $fsa2->desc() . ")");

    # Initialise state queue
    push(@state_queue, [ $this->root(), $fsa1->root(), $fsa2->root() ]);

    $state_set{$fsa1->root()->id() . "," . $fsa2->root()->id()} = $this->root();

    # Main loop over unprocessed result states
    while (@state_queue) {
        my ($state, $state1, $state2) = @{ shift(@state_queue) };

        DEBUX(1, "State %s", $state);

        my @branches1;
        my @branches2;

        if (defined $state1) {
            DEBUX(1, "  + state %s", $state1);

            $state->accepts([$state->accepts(), $state1->accepts()]);

            @branches1 = map([ FSA::Symbol::Set::copy($_->symbol()), $_->target() ], $state1->branch());
        }

        if (defined $state2) {
            DEBUX(1, "  + state %s", $state2);

            $state->accepts([$state->accepts(), $state2->accepts()]);

            @branches2 = map([ FSA::Symbol::Set::copy($_->symbol()), $_->target() ], $state2->branch());
        }

        # Compute result state branches char. specs
        while (@branches1) {
            my ($symbol1, $target1) = @{ shift(@branches1) };

            foreach my $branch2 (@branches2) {
                my $intersect = FSA::Symbol::Set::intersection($symbol1, $branch2->[0]);

                # Combined state transition
                unless ($intersect->isEmpty()) {
                    DEBUX(2, "Intersection: %s", $intersect);

                    _unionBranch(\@state_queue, \%state_set, $state,
                                 $intersect, $target1, $branch2->[1]);

                    # Subtract the intersection from parent char specs
                    $symbol1 = FSA::Symbol::Set::subtraction($symbol1, $intersect);

                    $branch2->[0] = FSA::Symbol::Set::subtraction($branch2->[0], $intersect);
                }

                # Nothing else to intersect
                $symbol1->isEmpty() && last;
            }

            # Remove branches 2 with empty char specs
            @branches2 = grep(!$_->[0]->isEmpty(), @branches2);

            # Single transition by char spec 1 residual
            unless ($symbol1->isEmpty()) {
                DEBUX(2, "Residual 1: %s", $symbol1);

                _unionBranch(\@state_queue, \%state_set, $state,
                             $symbol1, $target1, undef);
            }
        }

        # Single transitions by char specs 2 residuals
        foreach my $branch2 (@branches2) {
            DEBUX(2, "Residual 2: %s", $branch2->[0]);

            _unionBranch(\@state_queue, \%state_set, $state,
                         $branch2->[0], undef, $branch2->[1]);
        }
    }

    return $this;
}


sub union(@) {
    my $union = shift;

    while (@_) {
        $union = _union2($union, shift);
    }

    return $union;
}


sub _addRootBranchResidual($$) {
    my ($state, $fsa) = @_;

    DEBUX(1, "State %s", $state);

    my $symbol_exclude = FSA::Symbol::Set::union(map($_->symbol(), $state->branch()));

    DEBUX(1, "Excluding symbols %s", $symbol_exclude);

    my @branches;

    foreach my $root_branch ($fsa->root()->branch()) {
        my $symbol = FSA::Symbol::Set::subtraction($root_branch->symbol(), $symbol_exclude);

        $symbol->isEmpty() && next;

        my $branch = new FSA::Branch;

        $branch->symbol($symbol);

        $branch->target($root_branch->target());

        push(@branches, $branch);
    }

    $state->branch([ $state->branch(), @branches ]);

    DEBUX(1, "Adapted state: %s", $state);
}


sub _concat2($$) {
    my $fsa1 = shift;
    my $fsa2 = shift;

    $fsa1 = $fsa1->copy();
    $fsa2 = $fsa2->copy();

    DEBUX(1, "Concatenation: %s", $fsa1);
    DEBUX(1, "and %s", $fsa2);

    # Add all 2nd FSA root branches to all 1st FSA accepting states
    my @accept_states1 = grep($_->accepts(), $fsa1->_state());

    my $root2          = $fsa2->root();
    my $root2_accepts  = $root2->accepts();

    my @state_queue;

    foreach my $state (@accept_states1) {
        # The result FSA state keeps accepting iff FSA 2 root accepts
        $root2_accepts || $state->accepts([]);

        push(@state_queue, [ $state, $root2 ]);
    }

    while (@state_queue) {
        # Add all branches from the 2nd state to the 1st state
        my ($state, $branch_state) = @{ shift(@state_queue) };

        DEBUX(2, "Adding branches from %s", $branch_state);
        DEBUX(2, "to %s", $state);

        my @added_branches = map($_->copy(), $branch_state->branch());

        if (@added_branches) {
            my @current_branches = $state->branch();

            my @branches;

            # Make branches symbol sets disjoint
            foreach my $added_branch (@added_branches) {
                DEBUX(2, "Adding %s to %s", $added_branch, $state);

                my $added_symbol = $added_branch->symbol();

                # Make branches symbol sets disjoint
                foreach my $current_branch (@current_branches) {
                    my $current_symbol = $current_branch->symbol();

                    my $intersect_symbol = FSA::Symbol::Set::intersection($added_symbol, $current_symbol);

                    # Current branch and added branch symbol sets are not disjoint
                    unless ($intersect_symbol->isEmpty()) {
                        DEBUX(2, "Non-empty intersection: %s", $intersect_symbol);

                        # New intersection branch target inherits branches from targets of both branches
                        my $added_target   = $added_branch->target();
                        my $current_target = $current_branch->target();

                        my $intersect_state = new FSA::State($current_target->desc() . "." . $added_target->desc());

                        $intersect_state->accepts([ $current_target->accepts(), $added_target->accepts() ]);

                        DEBUX(2, "Intersection branch target: %s", $intersect_state);

                        push(@state_queue, [ $intersect_state, $current_target ],
                                           [ $intersect_state, $added_target   ]);

                        my $intersect_branch = new FSA::Branch;

                        $intersect_branch->symbol($intersect_symbol);
                        $intersect_branch->target($intersect_state);

                        push(@branches, $intersect_branch);

                        # Update both branches symbol sets
                        $current_symbol = FSA::Symbol::Set::subtraction($current_symbol, $intersect_symbol);
                        $added_symbol   = FSA::Symbol::Set::subtraction($added_symbol,   $intersect_symbol);
                    }

                    # Only keep branches with non-empty symbol sets
                    unless ($current_symbol->isEmpty())
                    {
                        $current_branch->symbol($current_symbol);

                        push(@branches, $current_branch);
                    }

                    $added_symbol->isEmpty() && last;
                }

                # Only keep branches with non-empty symbol sets
                unless ($added_symbol->isEmpty()) {
                    $added_branch->symbol($added_symbol);

                    push(@branches, $added_branch);
                }

                DEBUX(2, "Actual branch set: %s", \@branches);
            }

            # Set new branches
            $state->branch([ @branches ]);
        }

        DEBUX(2, "Addition result: %s", $state);
    }

    DEBUX(1, "Result: %s", $fsa1);

    return $fsa1;
}

sub concat(@) {
    my $concat = shift;

    while (@_) {
        $concat = _concat2($concat, shift);
    }

    return $concat;
}


sub iteration($$@) {
    my $fsa = shift;

    my $lo = shift || 0;
    my $hi = shift;

    defined $hi && $hi < $lo && FATAL("Invalid iteration bounds specification: $lo, $hi");

    DEBUX(1, "Iteration %d..%s of %s", $lo, defined $hi ? $hi : "", $fsa);

    if ($lo == 1 && defined $hi && $hi == $lo) {
        DEBUX(1, "Nothing to do");

        return $fsa;
    }

    $fsa = $fsa->copy();

    my $iter = new FSA;

    $iter->root()->accepts([ "iteration head" ]);

    DEBUX(1, "Head: %s", $iter);

    my $i = 0;

    # Finite prefix iteration
    for (; $i < $lo; ++$i) {
        $iter = FSA::concat($iter, $fsa);
    }

    DEBUX(1, "Finite prefix: %s", $iter);

    $fsa->root()->accepts([ "null iteration" ]);

    # Finite iteration
    if (defined $hi) {
        for (; $i < $hi; ++$i) {
            $iter = FSA::concat($iter, $fsa);
        }

        DEBUX(1, "Finite iteration: %s", $iter);
    }

    # Infinite iteration
    else {
        DEBUX(1, "Cycling %s", $fsa);

        # Get accepting states
        my @accepting_states = grep($_->accepts() && $_ != $fsa->root(), $fsa->_state());

        DEBUX(1, "Accepting states: @accepting_states");

        # Add root branches copies to all accepting states...
        foreach my $state (@accepting_states) {
            _addRootBranchResidual($state, $fsa);
        }

        DEBUX(1, "Cycled: %s", $fsa);

        $iter = FSA::concat($iter, $fsa);

        DEBUX(1, "Infinite iteration: %s", $iter);
    }

    return $iter;
}


sub _state($@) {
    my ($this, $state, $state_ids) = @_;

    $state or $state = $this->root();

    $state_ids or $state_ids = {};

    $state_ids->{$state->id()} && return;

    $state_ids->{$state->id()} = 1;

    return ($state, map($this->_state($_->target(), $state_ids), $state->branch()));
}


sub _branch($@) {
    my ($this, $state, $state_ids) = @_;

    $state or $state = $this->root();

    $state_ids or $state_ids = {};

    $state_ids->{$state->id()} && return;

    $state_ids->{$state->id()} = 1;

    my @branches = $state->branch();

    return (@branches, map($this->_branch($_->target(), $state_ids), @branches));
}


sub copy($) {
    my $fsa = shift;

    DEBUX(2, "Original FSA: %s", $fsa);

    my $class = ref($fsa);

    # Copy root
    my $fsa_root = $fsa->root();

    my $this = $class->SUPER::new();

    $this->{_desc} = $fsa->desc();
    $this->{_root} = FSA::State::copy($fsa_root);

    bless($this, ref($fsa));

    my @states = ([$fsa_root, $this->root()]);

    my %states = ($fsa_root->id() => $this->root());

    # Copy accessible states
    while (@states) {
        my ($orig_state, $copy_state) = @{shift(@states)};

        my @branches = $orig_state->branch();

        # Copy branches
        foreach my $orig_branch (@branches) {
            my $copy_branch = FSA::Branch::copy($orig_branch);

            my $orig_target = $orig_branch->target();

            my $copy_target = $states{$orig_target->id()};

            unless ($copy_target) {
                $copy_target = FSA::State::copy($orig_target);

                push(@states, [$orig_target, $copy_target]);

                $states{$orig_target->id()} = $copy_target;
            }

            $copy_branch->target($copy_target);

            $copy_state->branch([$copy_state->branch(), $copy_branch]);
        }
    }

    DEBUX(2, "Copy FSA: %s", $this);

    return $this;
}


sub setAcceptStatesTerminal($) {
    my $this = shift;

    # Remove all branches from all accepting states
    my @accept_states = grep($_->accepts(), $this->_state());

    foreach my $state (@accept_states) {
        $state->branch([]);
    }
}


sub minimise($) {
    my $this = shift;

    DEBUG("Minimising %s", $this);

    my @states = $this->_state();

    # Create initial state table and transition symbol list
    my @state_tab;
    my %state_map;
    my @symbol_list;

    foreach my $state (@states) {
        # Add transition symbols of the state branches to the list
        my @symbols;

        foreach my $symbol (map($_->symbol()->copy(), $state->branch())) {
            # Keep symbol sets in the list disjoint
            foreach my $enlisted (@symbol_list) {
                my $intersection = FSA::Symbol::Set::intersection($enlisted, $symbol);

                unless ($intersection->isEmpty()) {
                    push(@symbols, $intersection);

                    $enlisted = FSA::Symbol::Set::subtraction($enlisted, $intersection);
                    $symbol   = FSA::Symbol::Set::subtraction($symbol,   $intersection);

                    $symbol->isEmpty() && last;
                }
            }

            # Remove empty symbol sets from symbol list
            @symbol_list = grep(!$_->isEmpty(), @symbol_list);

            $symbol->isEmpty() || push(@symbols, $symbol);
        }

        push(@symbol_list, @symbols);

        # Add state to appropriate class by accepted word classes
        my $i = 0;

        for (; $i < @state_tab; ++$i) {
            my $class = $state_tab[$i];

            my $representant = $class->[0];

            # Suitable class for the state found
            FSA::State::cmpByAccept($representant, $state) || last;
        }

        push(@{$state_tab[$i]}, $state);

        $state_map{$state->id()} = $i;
    }

    DEBUG("Symbol list: %s", \@symbol_list);

    # Create transition table
    my %trans_tab;

    foreach my $state (@states) {
        my $state_id = $state->id();

        # The table provides info about transition target (if any)
        # from every state via every symbol set from the symbol set list
        my $trans_tab_row = $trans_tab{$state_id} = [];

        foreach my $symbol (@symbol_list) {
            my $trans_id;

            # Note that as symbol sets in the symbol list are disjoint
            # and so are the state branches symbol sets,
            # there may be at most one branch with symbol set that
            # has non-empty intersection with a symbol set from the list
            foreach my $branch ($state->branch()) {
                my $intersect = FSA::Symbol::Set::intersection($symbol, $branch->symbol());

                # Also note that the symbol sets in the list were created
                # by mutually intersecting all symbol sets from all branches
                # of all states of the FSA.  Therefore if the intersection is
                # non-empty then the symbol set from the list is entirely
                # under the branch symbol set (i.e. the intersection is
                # equal to the symbol set from the list).
                unless ($intersect->isEmpty()) {
                    $trans_id = $branch->target()->id();

                    last;
                }
            }

            push(@$trans_tab_row, $trans_id);
        }
    }

    DEBUG("Transition table: %s", \%trans_tab);

    # Main loop (iterative state table re-factoring)
    my $refactor = 1;

    while ($refactor) {
        DEBUG("State table: %s", \@state_tab);
        DEBUG("State map: %s",   \%state_map);

        my @state_tab_next;
        my %state_map_next;

        my $i = 0;

        # Re-factor table
        for (; $i < @state_tab; ++$i) {
            my $class = $state_tab[$i];

            my $offset = @state_tab_next;

            # Re-factor class
            foreach my $state (@$class) {
                my $state_id = $state->id();

                my $state_trans_tab_row = $trans_tab{$state_id};

                my $j = $offset;

                # Note that in the 1st iteration of the foreach loop, the following
                # loop body is never done.
                # That means that if a state once becomes a class representant
                # then it stays the class representant ever since.
                for (; $j < @state_tab_next; ++$j) {
                    my $representant = $state_tab_next[$j]->[0];

                    my $representant_id = $representant->id();

                    my $representant_trans_tab_row = $trans_tab{$representant_id};

                    # Check if transitions match
                    my $trans_match = 1;

                    for (my $k = 0; $trans_match && $k < @$state_trans_tab_row; ++$k) {
                        my $state_trans = $state_trans_tab_row->[$k];

                        my $representant_trans = $representant_trans_tab_row->[$k];

                        if (defined $state_trans) {
                            $trans_match = defined $representant_trans
                                         ? $state_trans eq $representant_trans
                                         : undef;
                        }
                        else {
                            $trans_match = not defined $representant_trans;
                        }
                    }

                    # Matching representant, class found
                    $trans_match && last;
                }

                push(@{$state_tab_next[$j]}, $state);

                $state_map_next{$state->id()} = $j;
            }
        }

        # Classes added (iteration fixed point not reached)
        $refactor = @state_tab_next != @state_tab;

        DEBUG("Refactoring %s", $refactor ? "continues" : "finished");

        @state_tab = @state_tab_next;
        %state_map = %state_map_next;
    }

    # Construct minimal FSA from classes representants
    my @min_states = map($_->[0]->copy(), @state_tab);

    for (my $i = 0; $i < @state_tab; ++$i) {
        my $representant = $state_tab[$i]->[0];

        my $min_state = $min_states[$i];

        # Create minimal FSA state branches
        my %min_branches;

        foreach my $branch ($representant->branch()) {
            my $symbol = $branch->symbol();
            my $target = $branch->target();

            my $min_branch_target = $min_states[$state_map{$target->id()}];

            # Join branches to the same target
            if (exists $min_branches{$min_branch_target}) {
                my $min_branch = $min_branches{$min_branch_target};

                my $min_branch_symbol = $min_branch->symbol();

                $min_branch->symbol(FSA::Symbol::Set::union($min_branch_symbol, $symbol));
            }
            else {
                my $min_branch_symbol = $symbol->copy();

                my $min_branch = new FSA::Branch;

                $min_branch->symbol($min_branch_symbol);
                $min_branch->target($min_branch_target);

                $min_branches{$min_branch_target} = $min_branch;
            }
        }

        $min_state->branch([ values %min_branches ]);
    }

    my $min_root = $min_states[$state_map{$this->root()->id()}];

    my $min_fsa = new FSA($this->desc() . " minimal", $min_root);

    DEBUG("Result: %s", $min_fsa);

    return $min_fsa;
}


sub str($) {
    my $this = shift;

    my $id     = $this->id();
    my @states = $this->_state();

    local $" = ", ";

    return ref($this) . "($id, description: \"" . $this->desc() . "\", states: (@states))";
}


sub xmlElementName { "fsa" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        "id"      => $this->id(),
        "root-id" => $this->root()->id(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    return (
        xmlNewElement("description", {}, $this->desc()),
        $this->_state()
    );
}


1;
