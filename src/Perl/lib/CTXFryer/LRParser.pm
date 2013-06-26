package CTXFryer::LRParser;

use strict;
use warnings;

use base qw(CTXFryer::PDA);

use CTXFryer::PDA;

use CTXFryer::LRParser::Action::Shift;
use CTXFryer::LRParser::Action::Reduce;
use CTXFryer::LRParser::Action::Accept;

use CTXFryer::LRParser::State;
use CTXFryer::LRParser::Output;

use CTXFryer::LRParser::Transition;
use CTXFryer::LRParser::Reduction;

use CTXFryer::Grammar::Terminal::EOF;

use CTXFryer::Grammar::Item;
use CTXFryer::Grammar::ItemSet;

use CTXFryer::Table;
use CTXFryer::Table::Iterator;

use CTXFryer::Math::Set;
use CTXFryer::Math::Couple;
use CTXFryer::Math::Function::Unary;
use CTXFryer::Math::Relation::Binary;

use CTXFryer::Logging qw(:all);

use CTXFryer::Serialised qw(:xml);


our $eof;

BEGIN {
    $eof = new CTXFryer::Grammar::Terminal::EOF;
}


sub _createLR0itemSets($) {
    my $root_rule = shift;

    my $item_sets = new CTXFryer::List;
    my $sym_tab   = new CTXFryer::Table;
    my $trans_tab = new CTXFryer::Table(2);

    my $item_set_0 = new CTXFryer::Grammar::ItemSet(
         new CTXFryer::Grammar::Item($root_rule, 0));

    $item_sets->push($item_set_0);

    my %item_set_tab = ( $item_set_0->signature() => 0 );

    for (my $i = 0; $i < $item_sets->size(); ++$i) {
        my $item_set  = $item_sets->at($i);
        my $signature = $item_set->signature();

        # Construct next item sets kernels
        my %kernels;

        foreach my $item ($item_set->items()) {
            my $next_item = $item->next();

            defined $next_item || next;

            my $symbol = $item->symbol();
            my $sym_id = $symbol->id();

            # Pointer would provide unique key as well as instant access, here
            $sym_tab->at($sym_id, $symbol);

            my $kernel = $kernels{$sym_id} ||= [];

            push(@$kernel, $next_item);
        }

        # Filter existing next item sets created from the kernels
        while (my ($sym_id, $kernel) = each %kernels) {
            $item_set  = new CTXFryer::Grammar::ItemSet(@$kernel);
            $signature = $item_set->signature();

            my $target = $item_set_tab{$signature};

            unless (defined $target) {
                $target = $item_sets->size();

                $item_sets->push($item_set);

                $item_set_tab{$signature} = $target;
            }

            $trans_tab->at($i, $sym_id, $target);
        }
    }

    DEBUG("Item sets: %s", $item_sets);
    DEBUG("Item sets transitions: %s", $trans_tab);
    DEBUG("Terminal symbols IDs: %s", $sym_tab);

    return ($item_sets, $trans_tab, $sym_tab);
}


sub new($$) {
    my $class = shift; $class = ref $class || $class;

    my $grammar = shift;

    UNIVERSAL::isa($grammar, "CTXFryer::Grammar")
    or FATAL("INTERNAL ERROR: %s isn't a grammar", $grammar);

    # Augment the grammar (so there's only one root rule)
    $grammar = $grammar->augmented();

    # Get the augmented grammar root rule
    my @root_rules = $grammar->root()->LHSrules();

    1 == @root_rules
    or FATAL("INTERNAL ERROR: Augmented grammar %s has %d root rules: %s",
             $grammar, scalar(@root_rules), \@root_rules);

    my $root_rule = $grammar->rule($root_rules[0]);

    # Create LR0 item sets and transition table
    my ($item_sets, $trans_tab, $sym_tab) = _createLR0itemSets($root_rule);

    # Create LR0 parser states from item sets
    my @states;

    for (my $i = 0; $i < $item_sets->size(); ++$i) {
        push(@states, new CTXFryer::LRParser::State($i, $item_sets->at($i)));
    }

    # Copy the transitions to action and goto tables
    my $action_tab = new CTXFryer::Table(2);
    my $goto_tab   = new CTXFryer::Table(2);

    my $iter = new CTXFryer::Table::Iterator($trans_tab);

    while (my ($i, $sym_id, $target) = $iter->each()) {
        my $symbol = $sym_tab->at($sym_id);

        # Terminal transitions form shift actions
        if (UNIVERSAL::isa($symbol, "CTXFryer::Grammar::Terminal")) {
            $action_tab->at($i, $sym_id,
                new CTXFryer::List(
                    new CTXFryer::LRParser::Action::Shift($target)));
        }

        # Non-terminal transitions form the goto table
        elsif (UNIVERSAL::isa($symbol, "CTXFryer::Grammar::NonTerminal")) {
            $goto_tab->at($i, $sym_id, $target);
        }

        # Unexpected object instead of a symbol
        else {
            FATAL("INTERNAL ERROR: Unexpected object %s", $symbol);
        }
    }

    DEBUG("Goto table: $goto_tab");

    DEBUG("Action table (shift only): $action_tab");

    # Add accept and reduce actions (and create lists of conflicts)
    my @terminal_ids = map {
        @$_
    }
    grep {
        UNIVERSAL::isa($sym_tab->at(@$_), "CTXFryer::Grammar::Terminal")
    } $sym_tab->keys();

    push(@terminal_ids, $eof->id());

    my $accept_item = new CTXFryer::Grammar::Item($root_rule, $root_rule->length());

    my $shift_reduce_conflicts  = new CTXFryer::List;
    my $reduce_reduce_conflicts = new CTXFryer::List;

    for (my $i = 0; $i < $item_sets->size(); ++$i) {
        my $item_set = $item_sets->at($i);

        # Accept action
        my $accept_action = $item_set->contains($accept_item);

        if ($accept_action) {
            $action_tab->at($i, $eof->id(),
                new CTXFryer::List(
                    new CTXFryer::LRParser::Action::Accept));
        }

        # Reduce action(s) rule numbers
        my @reduce_rule_no = map {
            $_->rule()->number()
        }
        grep {
            $_ != $accept_item
        } $item_set->finalItems();

        @reduce_rule_no || next;

        DEBUX(1, "Item set #%d: %s shall be transformed into %d reduce actions",
                 $i, $item_set, scalar(@reduce_rule_no));

        # Reduce-reduce conflict
        if (1 < @reduce_rule_no) {
            DEBUG("Reduce-reduce conflict in state %d, rules %s", $i, \@reduce_rule_no);

            $reduce_reduce_conflicts->push(map(new CTXFryer::List($i, $_), @terminal_ids));
        }

        foreach my $term_id (@terminal_ids) {
            my $action_list = $action_tab->at($i, $term_id);

            # Conflict
            if ($action_list) {
                # At most 1 conflicting action may appear
                1 == $action_list->size()
                or FATAL("INTERNAL ERROR: Unexpected actions at (%d, %s): %s",
                         $i, $term_id, $action_list);

                # Accept action derived from the reduce action; skip adding reduce actions
                $eof->id() eq $term_id && $accept_action
                and next;

                # Shift-reduce conflict is only accepted
                UNIVERSAL::isa($action_list->at(0), "CTXFryer::LRParser::Action::Shift")
                or FATAL("INTERNAL ERROR: Unexpected action at (%d, %s): %s",
                         $i, $term_id, $action_list->at(0));

                DEBUG("Shift-reduce conflict in state %d, %s vs reduction(s) by rule(s) %s",
                      $i, $action_list->at(0), \@reduce_rule_no);

                $shift_reduce_conflicts->push(new CTXFryer::List($i, $term_id));
            }

            # Only action
            else {
                $action_list = new CTXFryer::List;

                $action_tab->at($i, $term_id, $action_list);
            }

            my @reduce = map(new CTXFryer::LRParser::Action::Reduce($_), @reduce_rule_no);

            DEBUX(1, "Adding reduce action(s) %s to action table at [%d, %s]",
                     \@reduce, $i, $term_id);

            $action_list->push(@reduce);
        }
    }

    DEBUG("Action table: $action_tab");

    my $this = $class->SUPER::new(@states);

    $this->{_grammar}    = $grammar;
    $this->{_sym_tab}    = $sym_tab;
    $this->{_trans_tab}  = $trans_tab;
    $this->{_item_sets}  = $item_sets,
    $this->{_action_tab} = $action_tab;
    $this->{_goto_tab}   = $goto_tab;
    $this->{_output}     = new CTXFryer::LRParser::Output;

    $this->{_shift_reduce_conflicts}  = $shift_reduce_conflicts;
    $this->{_reduce_reduce_conflicts} = $reduce_reduce_conflicts;

    $this->{_is_LR0} = 0 == $shift_reduce_conflicts->size() &&
                       0 == $reduce_reduce_conflicts->size();

    bless($this, $class);

    DEBUG("The parser is%s an LR(0) parser", $this->isLR0() ? "" : "n't");

    # Try to turn the parser into LALR(1) parser (unless already LR(0))
    if ($this->isLR0()) {
        $this->{_is_LALR1} = 1;
    }
    else {
        $this->_makeLALR1();

        DEBUG("The parser is%s an LALR(1) parser", $this->isLALR1() ? "" : "n't");
    }

    return $this;
}


sub label($@) {
    my $this = shift;

    @_ and $this->{_label} = shift;

    return $this->{_label};
}


sub author($@) {
    my $this = shift;

    @_ and $this->{_author} = shift;

    return $this->{_author};
}


sub description($@) {
    my $this = shift;

    @_ and $this->{_descr} = shift;

    return $this->{_descr};
}


sub grammar($) {
    my $this = shift;

    return $this->{_grammar};
}


sub _symbolTable($) {
    my $this = shift;

    return $this->{_sym_tab};
}


sub _transitionTable($) {
    my $this = shift;

    return $this->{_trans_tab};
}


sub _itemSets($) {
    my $this = shift;

    return $this->{_item_sets};
}


sub _actionTable($) {
    my $this = shift;

    return $this->{_action_tab};
}


sub _gotoTable($) {
    my $this = shift;

    return $this->{_goto_tab};
}


sub _DirectReadFunction($) {
    my $this = shift;

    return $this->{_DR};
}


sub _readsRelation($) {
    my $this = shift;

    return $this->{_reads};
}


sub _ReadFunction($) {
    my $this = shift;

    return $this->{_Read};
}


sub _includesRelation($) {
    my $this = shift;

    return $this->{_includes};
}


sub _FollowFunction($) {
    my $this = shift;

    return $this->{_Follow};
}


sub _lookbackRelation($) {
    my $this = shift;

    return $this->{_lookback};
}


sub _LookAheadTable($) {
    my $this = shift;

    return $this->{_LA_tab};
}


sub output($) {
    my $this = shift;

    return $this->{_output};
}


sub _init($) {
    my $this = shift;

    $this->stack()->push(0);

    $this->output()->clear();
}


sub _func($$$$) {
    my ($this, $state_no, $input, $stack) = @_;

    my $next_term = $input->head()->id();

    my $action_list = $this->_actionTable()->at($state_no, $next_term);

    # Reject
    if (!$action_list) {
        $this->output()->push("Syntax error");

        return;
    }

    my $action_list_size = $action_list->size();

    # Sanity check
    0 < $action_list_size
    or FATAL("INTERNAL ERROR: Action table contains an empty action list");

    # Conflict
    if (1 < $action_list_size) {
        WARN("Action conflict in state %d for terminal ID %s", $state_no, $next_term);
        WARN("Conflicting actions: %s", $action_list);
        WARN("Shall continue anyway using the 1st action");
    }

    my $action = $action_list->at(0);

    # Shift
    if ($action->isa("CTXFryer::LRParser::Action::Shift")) {
        $input->shift();

        $state_no = $action->state();
    }

    # Reduce
    elsif ($action->isa("CTXFryer::LRParser::Action::Reduce")) {
        my $rule_no = $action->rule();

        $this->output()->push($rule_no);

        my $rule = $this->grammar()->rule($rule_no);

        $stack->pop($rule->length());

        my $stack_top    = $stack->top();
        my $rule_left_id = $rule->left()->id();

        $state_no = $this->_gotoTable()->at($stack_top, $rule_left_id)
        or FATAL("INTERNAL ERROR: No state in goto table for (%d, %s)",
                 $stack_top, $rule_left_id);
    }

    # Accept
    elsif ($action->isa("CTXFryer::LRParser::Action::Accept")) {
        # Sanity checks
        $next_term == $eof
        or FATAL("INTERNAL ERROR: Automaton accepts but stack isn't empty");

        $input->isEmpty()
        or FATAL("INTERNAL ERROR: Automaton accepts but input isn't empty");

        return;
    }

    # Unexpected action
    else {
        FATAL("INTERNAL ERROR: Unexpected action %s", $action);
    }

    $stack->push($state_no);

    return $state_no;
}


sub accepts($$) {
    my ($this, $input) = @_;

    # Augment input with special end-of-file terminal
    $input->push($eof);

    return $this->SUPER::accepts($input);
}


sub _shiftReduceConflicts($) {
    my $this = shift;

    return $this->{_shift_reduce_conflicts};
}


sub _reduceReduceConflicts($) {
    my $this = shift;

    return $this->{_reduce_reduce_conflicts};
}


sub _conflicts($) {
    my $this = shift;

    my $sr = $this->_shiftReduceConflicts();
    my $rr = $this->_reduceReduceConflicts();

    return CTXFryer::List::join($sr, $rr);
}


sub isLR0($) {
    my $this = shift;

    return $this->{_is_LR0};
}


sub isLALR1($) {
    my $this = shift;

    return $this->{_is_LALR1};
}


sub _terminalTransitions($@) {
    my ($this, $state_no) = @_;

    my $sym_tab = $this->_symbolTable();

    my $term_trans_tab = $this->_transitionTable()->cut($state_no, sub {
        my $sym_id = shift;
        my $symbol = $sym_tab->at($sym_id)
        or FATAL("INTERNAL ERROR: No symbol with ID %s", $sym_id);

        UNIVERSAL::isa($symbol, "CTXFryer::Grammar::Terminal")
    });

    return $term_trans_tab;
}


sub _originalRootNonTerminal($) {
    my $this = shift;

    my $grammar = $this->grammar();

    my @root_rules = $grammar->root()->LHSrules();

    1 == @root_rules
    or FATAL("INTERNAL ERROR: The grammar isn't augmented");

    my @root_rule_right = $grammar->rule($root_rules[0])->right();

    1 == @root_rule_right
    or FATAL("INTERNAL ERROR: The grammar isn't augmented");

    return $root_rule_right[0];
}


sub _directRead($) {
    my $this = shift;

    my @DR;

    my $orig_root = $this->_originalRootNonTerminal()->id();
    my $goto_tab  = $this->_gotoTable();

    my $goto_tab_iter = new CTXFryer::Table::Iterator($goto_tab);

    # For each non-terminal transition p -A-> r
    while (my ($p, $A, $r) = $goto_tab_iter->each()) {
        my $r_terminal_trans_tab = $this->_terminalTransitions($r);

        my @terminals;

        if ($r_terminal_trans_tab) {
            @terminals = map($_->[0], $r_terminal_trans_tab->keys());
        }

        # Add EOF to DR(0, original root)
        if (0 == $p && $A eq $orig_root) {
            push(@terminals, $eof->id());
        }

        # DR(p,A) = {t in T | p -A-> r -t->}
        push(@DR, new CTXFryer::LRParser::Transition($p, $A),
                  new CTXFryer::Math::Set(@terminals));
    }

    return new CTXFryer::Math::Function::Unary(@DR);
}


sub _reads($) {
    my $this = shift;

    my @reads;

    my $goto_tab = $this->_gotoTable();
    my $sym_tab  = $this->_symbolTable();

    my $goto_tab_iter = new CTXFryer::Table::Iterator($goto_tab);

    # For each non-terminal transition p -A-> r
    while (my ($p, $A, $r) = $goto_tab_iter->each()) {
        my $r_nonterminal_trans = $goto_tab->cut($r);

        $r_nonterminal_trans || next;

        # (p, A) reads (r, C) iff p -A-> r -C-> and C is nullable
        my $r_nt_trans_iter = new CTXFryer::Table::Iterator($r_nonterminal_trans);

        while (my ($C, $q) = $r_nt_trans_iter->each()) {
            my $C_nt = $sym_tab->at($C);

            # Sanity check
            UNIVERSAL::isa($C_nt, "CTXFryer::Grammar::NonTerminal")
            or FATAL("INTERNAL ERROR: %s isn't a non-terminal", $C_nt);

            if ($C_nt->isNullable()) {
                my $pA = new CTXFryer::LRParser::Transition($p, $A);
                my $rC = new CTXFryer::LRParser::Transition($r, $C);

                push(@reads, new CTXFryer::Math::Couple($pA, $rC));
            }
        }
    }

    return new CTXFryer::Math::Relation::Binary(@reads);
}


sub _traverseTransBackwards($$@) {
    my $this  = shift;
    my $state = shift;

    my $trans_tab = $this->_transitionTable();

    my $states = new CTXFryer::Math::Set($state);

    while (@_) {
        my $symbol = pop;

        DEBUX(2, "Searching for %s transitions to any state in %s",
                 $symbol, $states);

        my @prev_states;

        foreach my $state ($states->items()) {
            my $sym_trans = $trans_tab->cut(undef, $symbol->id(), $state);

            $sym_trans || next;

            DEBUX(1, "Found transitions from %s via %s to state #%d",
                     $sym_trans, $symbol, $state);

            push(@prev_states, map($_->[0], $sym_trans->keys()));
        }

        @prev_states || return;

        $states = new CTXFryer::Math::Set(@prev_states);
    }

    return $states;
}


sub _includes($) {
    my $this = shift;

    my @includes;

    my $grammar   = $this->grammar();
    my $goto_tab  = $this->_gotoTable();
    my $sym_tab   = $this->_symbolTable();
    my $trans_tab = $this->_transitionTable();

    foreach my $pA ($goto_tab->keys()) {
        my ($p, $A_id) = @$pA;

        my $A = $sym_tab->at($A_id)
        or FATAL("INTERNAL ERROR: %s isn't a symbol ID", $A_id);

        # Sanity check
        UNIVERSAL::isa($A, "CTXFryer::Grammar::NonTerminal")
        or FATAL("INTERNAL ERROR: %s isn't a non-terminal", $A);

        DEBUX(1, "Investigating non-terminal transition (%d, %s == %s)",
                 $p, $A_id, $A);

        foreach my $rule_no ($A->RHSrules()) {
            my $rule = $grammar->rule($rule_no)
            or FATAL("INTERNAL ERROR: Rule number %d doesn't exist",
                     $rule_no);

            # Consider rules in form
            # B => beta A gamma
            # where gamma is nullable sentence form
            my @right = $rule->right();

            # Pessimistic assumption
            my $rule_matches = 0;

            while (@right) {
                my $symbol = pop(@right);

                # Promissing rule found, continue
                if ($symbol == $A) {
                    $rule_matches = 1;
                    last;
                }

                # gamma contains a terminal, ergo isn't nullable
                UNIVERSAL::isa($symbol, "CTXFryer::Grammar::Terminal")
                and last;

                # Sanity check
                UNIVERSAL::isa($symbol, "CTXFryer::Grammar::NonTerminal")
                or FATAL("INTERNAL ERROR: %s isn't a non-terminal", $symbol);

                # gamma contains a non-nullable non-terminal, ergo isn't nullable
                $symbol->isNullable() || last;
            }

            $rule_matches || next;

            my $B = $rule->left();

            DEBUX(2, "Found rule %s matching pattern", $rule);

            # Traverse beta transition(s) backwards from p
            my $states = $this->_traverseTransBackwards($p, @right);

            $states || next;

            # Associate (q, B)-transitions (q stands for resulting state(s))
            my $B_id = $B->id();

            foreach my $q ($states->items()) {
                if (defined $trans_tab->at($q, $B_id)) {
                    push(@includes,
                        new CTXFryer::Math::Couple(
                            new CTXFryer::LRParser::Transition($p, $A_id),
                            new CTXFryer::LRParser::Transition($q, $B_id)));
                }
            }
        }
    }

    return new CTXFryer::Math::Relation::Binary(@includes);
}


sub _lookback($) {
    my $this = shift;

    my @lookback;

    my $grammar    = $this->grammar();
    my $action_tab = $this->_actionTable();
    my $sym_tab    = $this->_symbolTable();
    my $trans_tab  = $this->_transitionTable();

    DEBUG("Transitions table: %s", $trans_tab);

    # Compute reductions table
    my $reduce_tab = $action_tab->cut(undef, undef, sub {
        my $actions = shift;

        my $reduce;

        foreach my $action ($actions->items()) {
            $reduce = UNIVERSAL::isa($action, "CTXFryer::LRParser::Action::Reduce");

            $reduce && last;
        }

        $reduce;
    });

    DEBUG("Reductions table: %s", $reduce_tab);

    # Investigate all reductions (q, A -> omega)
    my @reductions;

    my $reduce_tab_iter = new CTXFryer::Table::Iterator($reduce_tab);

    while (my ($q, $t_id, $actions) = $reduce_tab_iter->each()) {
        foreach my $action ($actions->items()) {
            UNIVERSAL::isa($action, "CTXFryer::LRParser::Action::Reduce") || next;

            my $reduction = new CTXFryer::LRParser::Reduction($q, $action->rule());

            push(@reductions, $reduction);
        }
    }

    # Reduction set is created to get unique reductions, only
    my $reductions = new CTXFryer::Math::Set(@reductions);

    foreach my $reduction ($reductions->items()) {
        my $q       = $reduction->state();
        my $rule_no = $reduction->rule();
        my $rule    = $grammar->rule($rule_no);
        my $A       = $rule->left();
        my $omega   = $rule->right();

        DEBUX(1, "Investigating reduction (%d, %d: %s -> %s)",
                 $q, $rule_no, $A, $omega);

        # Traverse omega transitions backwards from q
        # to find state(s) p so that p -omega-> q
        my $states = $this->_traverseTransBackwards($q, $omega->items());

        $states || next;

        # Associate (p, A)-transitions (q stands for resulting state(s))
        my $A_id = $A->id();

        foreach my $p ($states->items()) {
            if (defined $trans_tab->at($p, $A_id)) {
                DEBUX(1, "(%d, %d: %s -> %s) lookback (%d, %s)",
                         $q, $rule_no, $A, $omega, $p, $A);

                push(@lookback,
                    new CTXFryer::Math::Couple(
                        $reduction, new CTXFryer::LRParser::Transition($p, $A_id)));
            }
        }
    }

    return new CTXFryer::Math::Relation::Binary(@lookback);
}


sub _LAtable($) {
    my $this = shift;

    my $Follow = $this->_FollowFunction()
    or FATAL("INTERNAL ERROR: Follow function isn't defined");

    my $lookback = $this->_lookbackRelation()
    or FATAL("INTERNAL ERROR: lookback relation isn't defined");

    my $LA_tab = new CTXFryer::Table(2);

    foreach my $couple ($lookback->items()) {
        my ($reduction, $transition) = $couple->item();

        my $terminals = $Follow->project($transition) || next;

        DEBUX(1, "Follow(%s) == %s", $transition, $terminals);

        $terminals->isEmpty() && next;

        my $state = $reduction->state();
        my $rule  = $reduction->rule();

        my $Follow_union = $LA_tab->at($state, $rule);

        if (defined $Follow_union) {
            $Follow_union->unionAssign($terminals);

            DEBUX(1, "LA(%d, %d) is now %s", $state, $rule, $Follow_union);

            next;
        }

        DEBUX(1, "LA(%d, %d) initialised to %s", $state, $rule, $terminals);

        $LA_tab->at($state, $rule, $terminals->copy());
    }

    return $LA_tab;
}


sub _checkLALR1($) {
    my $this = shift;

    my $LA_tab = $this->_LookAheadTable()
    or FATAL("INTERNAL ERROR: Look-ahead table isn't defined");

    my $states = $this->_states();

    my $LA_conflicts_cnt = 0;

    for (my $q = 0; $q < @$states; ++$q) {
        my $LA_q = $LA_tab->cut($q);

        DEBUX(1, "LA(%d, *) == %s", $q, $LA_q);

        $LA_q || next;

        my @terminals = $LA_q->keys();

        for (my $i_idx = 0; $i_idx < @terminals - 1; ++$i_idx) {
            my $i     = $terminals[$i_idx];
            my $LA_qi = $LA_q->at($i);

            $LA_qi || next;

            for (my $j_idx = $i_idx + 1; $j_idx < @terminals; ++$j_idx) {
                my $j     = $terminals[$j_idx];
                my $LA_qj = $LA_q->at($j);

                $LA_qj || next;

                my $LA_intersect = CTXFryer::Math::Set::intersection($LA_qi, $LA_qj);

                # Sanity check
                $LA_intersect
                or FATAL("INTERNAL ERROR: LA(%d, %d) intersection LA(%d, %d) isn't defined",
                         $q, $i, $q, $j);

                # Conflict means that the grammar isn't LALR(1)
                if (!$LA_intersect->isEmpty()) {
                    WARN("LA(%d, %d) intersection LA(%d, %d) == %s (not an empty set), " .
                         "grammar isn't LALR(1)", $q, $i, $q, $j, $LA_intersect);

                    ++$LA_conflicts_cnt;
                }
            }
        }
    }

    $this->{_is_LALR1} = 0 == $LA_conflicts_cnt;

    $LA_conflicts_cnt
    and WARN("Found %d conflicts in LA sets, the grammar isn't LALR(1)",
             $LA_conflicts_cnt);
}


sub _invalidateNonLALR1actions($) {
    my $this = shift;

    my $action_tab = $this->_actionTable();
    my $LA_tab     = $this->_LookAheadTable();

    my $action_tab_iter = new CTXFryer::Table::Iterator($action_tab);

    while (my ($state, $terminal_id, $actions) = $action_tab_iter->each()) {
        my @actions = $actions->items()
        or FATAL("INTERNAL ERROR: Action table at (%d, %s) is empty",
                 $state, $terminal_id);

        # Accept action should be a solitair
        if (UNIVERSAL::isa($actions[0], "CTXFryer::LRParser::Action::Accept")) {
            1 == @actions
            or FATAL("INTERNAL ERROR: Action table at (%d, %s) contains %s " .
                     "(not only the accept action)", $state, $terminal_id, \@actions);

            next;
        }

        # Shift action (if any) is on the list head
        my $shift;

        if (UNIVERSAL::isa($actions[0], "CTXFryer::LRParser::Action::Shift")) {
            $shift = shift(@actions);
        }

        # Filter reduce actions by LA table
        my $reduce_cnt = 0;

        foreach my $reduce (@actions) {
            UNIVERSAL::isa($reduce, "CTXFryer::LRParser::Action::Reduce")
            or FATAL("INTERNAL ERROR: %s isn't a reduce action", $reduce);

            my $rule_no = $reduce->rule();

            my $LA = $LA_tab->at($state, $rule_no);

            DEBUX(1, "Investigating %s at actions(%d, %s): LA(%d, %d) == %s",
                     $reduce, $state, $terminal_id, $state, $rule_no, $LA);

            if ($LA && $LA->contains($terminal_id)) {
                DEBUG("%s remains valid for action(%d, %s)",
                      $reduce, $state, $terminal_id);

                ++$reduce_cnt;
            }
            else {
                DEBUG("Invalidating %s at action(%d, %s)",
                      $reduce, $state, $terminal_id);

                $reduce->invalidate();
            }
        }

        # Invalidate shift action
        if ($shift && $reduce_cnt) {
            DEBUG("Invalidating %s at action(%d, %s), %d reduce action(s) kept",
                  $shift, $state, $terminal_id, $reduce_cnt);

            $shift->invalidate();
        }
    }
}


sub _makeLALR1($) {
    my $this = shift;

    # Pessimistic assumption
    $this->{_is_LALR1} = 0;

    # Compute DR function
    # DR(p, A) = {t in T | p -A-> q -t->}
    my $DR = $this->_directRead();

    $this->{_DR} = $DR;

    DEBUG("DR == %s", $DR);

    # Complute reads relation
    # (p, A) reads (q, B) <=> p -A-> q -B-> and B =>* e
    my $reads = $this->_reads();

    $this->{_reads} = $reads;

    DEBUG("reads == %s", $reads);

    # Compute Read function as DR closure with respect to reads
    # Read(p, A) = DR(p, A) u U{Read(q, B) | (p, A) reads (q, B)}
    my @ReadSCCs;
    my $Read = $DR->closure($reads, \@ReadSCCs);

    $this->{_Read} = $Read;

    DEBUG("Read == %s", $Read);
    DEBUG("Read SCCs: %s", \@ReadSCCs);

    # Check for non-trivial SCCs in Read
    my $non_trivial_scc_cnt = 0;

    foreach my $scc (@ReadSCCs) {
        if ($scc->cardinality() != 1) {
            ++$non_trivial_scc_cnt;

            WARN("Found non-trivial SCC %s in reads", $scc);
        }
    }

    if ($non_trivial_scc_cnt) {
        WARN("%d non-trivial SCC(s) exist in reads; " .
             "the grammar is not LR(k) for any k");

        return;
    }

    # Compute includes relation
    # (p, A) includes (q, B) <=> B -> beta A gamma, gamma =>* e and q -beta-> p
    my $includes = $this->_includes();

    $this->{_includes} = $includes;

    DEBUG("includes == %s", $includes);

    # Compute Follow function as Read closure with respect to includes
    # Follow(p, A) = Read(p, A) u U{Follow(q, B) | (p, A) includes (q, B)}
    my @FollowSCCs;
    my $Follow = $Read->closure($includes, \@FollowSCCs);

    $this->{_Follow} = $Follow;

    DEBUG("Follow == %s", $Follow);
    DEBUG("Follow SCCs: %s", \@FollowSCCs);

    # Check for non-trivial SCCs with non-empty Read sets in Follow
    my $non_trivial_scc_with_non_empty_Read_cnt = 0;

    foreach my $scc (@FollowSCCs) {
        if ($scc->cardinality() != 1) {
            my @scc_items = $scc->items();

            my $Read_scc = $Read->project($scc_items[0]);

            # Non-trivial SCC with non-empty Read set
            if (defined $Read_scc) {
                # Sanity check
                $Read_scc->isEmpty()
                and FATAL("INTERNAL ERROR: Read(%s) == %s (empty set)");

                ++$non_trivial_scc_with_non_empty_Read_cnt;

                WARN("Found non-trivial SCC %s in includes " .
                     "with non-empty Read set %s", $scc, $Read_scc);
            }
        }
    }

    if ($non_trivial_scc_with_non_empty_Read_cnt) {
        WARN("%d non-trivial SCC(s) with non-empty Read sets " .
             "exist in includes; the grammar is conjectured not LR(k) for any k");

        return;
    }

    # Compute lookback relation
    # (q, A -> omega) lookback (p, A) <=> p -omega->q
    my $lookback = $this->_lookback();

    $this->{_lookback} = $lookback;

    DEBUG("lookback == %s", $lookback);

    # Compute LA sets as union of Follow of transitions associated via lookback
    # LA(q, A -> omega) = U{Follow(p, A) | (q, A -> omega) lookback (p, A)}
    my $LA_tab = $this->_LAtable();

    $this->{_LA_tab} = $LA_tab;

    DEBUG("LA == %s", $LA_tab);

    # if grammar is LALR(1) then the following hold:
    # foreach state q and rules i, j:
    #  *  LA(q, i) intersection LA(q, j) = 0
    #     Reversed implication provides check if the grammar is LALR(1)
    #  *  t in LA(q, i) => shift(j) not in action(q, t)
    #     Gives shift-reduce conflicts resolution
    #  *  t in LA(q, i) => reduce(i) in action(q, t)
    #     Gives reduce-reduce conflicts resolution

    # Check for conflicts in LA sets for a state (LALR(1) validity check)
    $this->_checkLALR1();

    # Filter non-LALR(1) actions out by LA table
    $this->_invalidateNonLALR1actions();
}


sub str($) {
    my $this = shift;

    my $label      = $this->label() || "<undef>";
    my $grammar    = $this->grammar();
    my $action_tab = $this->_actionTable();
    my $goto_tab   = $this->_gotoTable();

    return ref($this) . "(" .
           "label: $label, " .
           "grammar: $grammar, " .
           "action table: $action_tab, " .
           "goto table: $goto_tab)";
}


sub xmlElementName { "lr-parser" }

sub xmlElementAttrs($) {
    my $this = shift;

    my %attrs = (
        "is-LR0"   => $this->isLR0()   ? "true" : "false",
        "is-LALR1" => $this->isLALR1() ? "true" : "false",
    );

    my $label = $this->label();

    defined $label and $attrs{label} = $label;

    my $author = $this->author();

    defined $author and $attrs{author} = $author;

    return %attrs;
}

sub xmlChildren($) {
    my $this = shift;

#    my $sym_tab_child = xmlNewElement(
#        "symbol-table",
#        {},
#        $this->_symbolTable()
#    );

    my $item_set_list_child = xmlNewElement(
        "item-sets",
        {},
        $this->_itemSets()
    );

    my $trans_tab_child = xmlNewElement(
        "transition-table",
        {},
        $this->_transitionTable()
    );

    my $action_tab_child = xmlNewElement(
        "action-table",
        {
            "eof-id" => $eof->id(),
        },
        $this->_actionTable()
    );

    my $goto_tab_child = xmlNewElement(
        "goto-table",
        {},
        $this->_gotoTable()
    );

    my @children = (
        $this->grammar(),
#        $sym_tab_child,
        $item_set_list_child,
        $trans_tab_child,
        $action_tab_child,
        $goto_tab_child,
    );

    my $sr = $this->_shiftReduceConflicts();

    0 < $sr->size()
    and push(@children, xmlNewElement(
        "shift-reduce-conflicts",
        {},
        $sr
    ));

    my $rr = $this->_reduceReduceConflicts();

    0 < $rr->size()
    and push(@children, xmlNewElement(
        "reduce-reduce-conflicts",
        {},
        $rr
    ));

    my $DR = $this->_DirectReadFunction();

    defined $DR
    and push(@children, xmlNewElement(
        "DR",
        {},
        $DR
    ));

    my $reads = $this->_readsRelation();

    defined $reads
    and push(@children, xmlNewElement(
        "reads",
        {},
        $reads
    ));

    my $Read = $this->_ReadFunction();

    defined $Read
    and push(@children, xmlNewElement(
        "Read",
        {},
        $Read
    ));

    my $includes = $this->_includesRelation();

    defined $includes
    and push(@children, xmlNewElement(
        "includes",
        {},
        $includes
    ));

    my $Follow = $this->_FollowFunction();

    defined $Follow
    and push(@children, xmlNewElement(
        "Follow",
        {},
        $Follow
    ));

    my $lookback = $this->_lookbackRelation();

    defined $lookback
    and push(@children, xmlNewElement(
        "lookback",
        {},
        $lookback
    ));

    my $LA_tab = $this->_LookAheadTable();

    defined $LA_tab
    and push(@children, xmlNewElement(
        "look-ahead-table",
        {},
        $LA_tab
    ));

    my $descr = $this->description();

    defined $descr
    and unshift(@children, xmlNewElement(
        "description",
        {},
        $descr
    ));

    return @children;
}


1;
