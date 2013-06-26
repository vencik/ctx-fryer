package CTXFryer::Grammar;

use strict;
use warnings;

use base qw(CTXFryer::Base CTXFryer::Serialised);

use CTXFryer::ProjectDef::Grammar::TerminalSymbols;
use CTXFryer::ProjectDef::Grammar::Rules;
use CTXFryer::ProjectDef::Grammar::Attributes;

use CTXFryer::Grammar::Terminal;
use CTXFryer::Grammar::NonTerminal;
use CTXFryer::Grammar::Rule;
use CTXFryer::Grammar::Symbol::Attribute qw(:all);

use CTXFryer::Grammar::Symbol::Attribute::Function;
use CTXFryer::Grammar::Symbol::Attribute::Reference;
use CTXFryer::Grammar::Symbol::Attribute::Dependency;
use CTXFryer::Grammar::Symbol::Attribute::Destructor;

use CTXFryer::List;
use CTXFryer::Table;

use CTXFryer::Math::Couple;
use CTXFryer::Math::Relation::Binary;
use CTXFryer::Math::Function::Unary;
use CTXFryer::Math::Boolean;

use CTXFryer::Logging qw(:all);

use CTXFryer::Serialised qw(:xml);


sub _resolveSymbolByRulePos($$$) {
    my ($rules, $rule_no, $sym_no) = @_;

    # Sanity check
    0 < $rule_no || FATAL("INTERNAL ERROR: Invalid rule number: %d", $rule_no);

    my $rule = $rules->at($rule_no - 1);

    if (!$rule) {
        ERROR("No rule #%d", $rule_no);

        return;
    }

    my $symbol = $sym_no ? $rule->right->at($sym_no - 1) : $rule->left();

    if (!$symbol) {
        ERROR("Rule #%d doesn't have symbol #%d", $rule_no, $sym_no);

        return;
    }

    return $symbol;
}


sub _new($$$$$$);

sub new($$$$$) {
    my $class = shift; $class = ref $class || $class;

    my $non_terminals_def = shift;
    my $terminals_def     = shift;
    my $root              = shift;
    my $rules_def         = shift;
    my $attrs_def         = shift;

    # Sanity checks
    UNIVERSAL::isa($terminals_def, "CTXFryer::ProjectDef::Grammar::TerminalSymbols")
    or FATAL("INTERNAL ERROR: Terminal symbols argument is invalid");

    UNIVERSAL::isa($rules_def, "CTXFryer::ProjectDef::Grammar::Rules")
    or FATAL("INTERNAL ERROR: Rules argument is invalid");

    UNIVERSAL::isa($attrs_def, "CTXFryer::ProjectDef::Grammar::Attributes")
    or FATAL("INTERNAL ERROR: Attributes argument is invalid");

    # Create non-terminals table
    my $non_terminals = new CTXFryer::Table;

    foreach my $ident (@$non_terminals_def) {
        $non_terminals->at($ident, new CTXFryer::Grammar::NonTerminal($ident));
    }

    # Create terminals table
    my %terminals = $terminals_def->map();
    my $terminals = new CTXFryer::Table;

    while (my ($ident, $regex) = each %terminals) {
        my $terminal = new CTXFryer::Grammar::Terminal($ident, $regex);

        $terminals->at($ident, $terminal);
    }

    # Get root
    my $root_non_terminal = $non_terminals->at($root)
    or FATAL("Root symbol %s isn't among non-terminals", $root);

    # Create rule list
    my @rules = $rules_def->all();
    my $rules = new CTXFryer::List;

    foreach my $rule (@rules) {
        my $left  = $rule->lhs();
        my @right = $rule->rhs();

        my $left_non_terminal = $non_terminals->at($left)
        or FATAL("Rule $left => @right left side isn't among non-terminals");

        my @right_symbols = map {
            $non_terminals->at($_) || $terminals->at($_)
            or FATAL("Rule $left => @right right side symbol $_ " .
                     "isn't among non-terminals nor terminals");
        } @right;

        $rules->push(new CTXFryer::Grammar::Rule($left_non_terminal => @right_symbols));
    }

    # Create attributes
    my @attr_and_def;  # implement by std::list<std::pair<G::S::A, std::pair<Pdef::G::A::type, Pdef::G::A> > >

    # Create built-in attributes
    my $token_getter = new CTXFryer::Grammar::Symbol::Attribute::Function("*", 0, "builtin::get_token");

    foreach my $terminal ($terminals->values()) {
        my $token = new CTXFryer::Grammar::Symbol::Attribute(ATTR_TYPE_AGGREGATED, "token");

        $token->evaluator($token_getter);

        $terminal->attr($token);
    }

    # Create defined attributes
    my $attr_error_cnt = 0;

    if ($attrs_def) {
        # Attributes defined explicitly per symbol
        foreach my $attr_def ($attrs_def->explicit()) {
            # Sanity check
            UNIVERSAL::isa($attr_def, "CTXFryer::ProjectDef::Grammar::Attribute::Explicit")
            or FATAL("INTERNAL ERROR: %s isn't an explicit attribute definition", $attr_def);

            my $attr_ident = $attr_def->identifier();
            my $sym_ident  = $attr_def->symID();
            my $attr_type  = ATTR_TYPE_AGGREGATED;
            my $symbol     = $terminals->at($sym_ident) || $non_terminals->at($sym_ident);

            if ($symbol) {
                my $attr = new CTXFryer::Grammar::Symbol::Attribute($attr_type, $attr_ident);

                # Add destructor (if exists)
                my $destroy = $attrs_def->destructor($sym_ident . '::' . $attr_ident);

                $destroy && $attr->destructor(new CTXFryer::Grammar::Symbol::Attribute::Destructor($destroy));

                DEBUG("Created %s for sym %s", $attr, $symbol);

                $symbol->attr($attr);

                push(@attr_and_def, [$attr, 'E', $attr_def]);
            }
            else {
                ERROR("Error creating explicit attribute %s: no symbol \"%s\"",
                      $attr_ident, $sym_ident);

                ++$attr_error_cnt;
            }
        }

        # Attributes defined implicitly via usage in relations
        foreach my $attr_def ($attrs_def->implicit()) {
            # Sanity check
            UNIVERSAL::isa($attr_def, "CTXFryer::ProjectDef::Grammar::Attribute::Implicit")
            or FATAL("INTERNAL ERROR: %s isn't an implicit attribute definition", $attr_def);

            my $attr_ident = $attr_def->identifier();
            my $rule_id    = $attr_def->ruleID();
            my $rule_no    = $attr_def->ruleNo();
            my $sym_no     = $attr_def->symNo();
            my $attr_type  = $sym_no ? ATTR_TYPE_INHERITED : ATTR_TYPE_AGGREGATED,

            # Make sure that rule number is resolved
            defined $rule_no or $rule_no = $rules_def->id2no($rule_id);
            if (!defined $rule_no) {
                ERROR("Error resolving rule %s number", $rule_id);

                ++$attr_error_cnt;
                next;
            }

            my $symbol = _resolveSymbolByRulePos($rules, $rule_no, $sym_no);

            if ($symbol) {
                # The attribute may have been already created
                my $attr = $symbol->getAttr($attr_ident);

                if ($attr) {
                    DEBUG("%s found for rule \"%s\" #%d, for sym #%d, %s",
                          $attr, $rule_id, $rule_no, $sym_no, $symbol);
                }
                else {
                    $attr = new CTXFryer::Grammar::Symbol::Attribute($attr_type, $attr_ident);

                    DEBUG("Created %s from rule \"%s\" #%d, for sym #%d: %s",
                          $attr, $rule_id, $rule_no, $sym_no, $symbol);

                    $symbol->attr($attr);
                }

                # Add destructor (if exists)
                my $destroy = $attrs_def->destructor($symbol->ident() . '::' . $attr_ident);

                $destroy && $attr->destructor(new CTXFryer::Grammar::Symbol::Attribute::Destructor($destroy));

                push(@attr_and_def, [$attr, 'I', $attr_def, $rule_no, $sym_no]);
            }
            else {
                ERROR("Error creating implicit attribute %s: rule #%d symbol #%d resolution error",
                      $attr_ident, $rule_no, $sym_no);

                ++$attr_error_cnt;
            }
        }
    }

    # Resolve attribute dependencies
    foreach my $attr_and_def (@attr_and_def) {
        my $attr        = $attr_and_def->[0];
        my $attr_type   = $attr_and_def->[1];
        my $attr_def    = $attr_and_def->[2];
        my $rule_no     = $attr_and_def->[3];
        my $attr_sym_no = $attr_and_def->[4];

        # Resolve rule ID for rule-specific attribute evaluator
        my $rule_id = '*';

        if (defined $rule_no) {
            my $rule = $rules->at($rule_no - 1);

            if ($rule) {
                $rule_id = $rule->id();
            }
            else {
                ERROR("Rule #%d doesn't exist", $rule_no);

                ++$attr_error_cnt;

                next;
            }
        }

        my @deps;

        foreach my $arg_def ($attr_def->arguments()) {
            my $arg_ident = pop(@$arg_def);
            my $arg_qual  = pop(@$arg_def);

            my $dep;

            DEP_DEF: {  # implement by pragmatic do while (0) loop

                my $sym;
                my $sym_no;

                # Implement as a switch
                if ('E' eq $attr_type) {
                    my $sym_ident = $attr_def->symID();

                    $sym = $terminals->at($sym_ident) || $non_terminals->at($sym_ident);

                    if (!$sym) {
                        ERROR("Unable to resolve symbol for explicit attribute %s::%s",
                              $sym_ident, $arg_ident);

                        ++$attr_error_cnt;

                        last DEP_DEF;
                    }
                }
                elsif ('I' eq $attr_type) {
                    my $rule_id = $attr_def->ruleID();
                    my $rule_no = $attr_def->ruleNo();

                    # Make sure that rule number is resolved
                    defined $rule_no or $rule_no = $rules_def->id2no($rule_id);
                    if (!defined $rule_no) {
                        ERROR("Error resolving rule %s number", $rule_id);

                        ++$attr_error_cnt;

                        last DEP_DEF;
                    }

                    $sym_no = $arg_qual;

                    $sym = _resolveSymbolByRulePos($rules, $rule_no, $sym_no);

                    if (!$sym) {
                        ERROR("Unable to resolve symbol for implicit attribute @%d::\$%d::%s",
                              $rule_no, $sym_no, $arg_ident);

                        ++$attr_error_cnt;

                        last DEP_DEF;
                    }
                }
                else {
                    FATAL("INTERNAL ERROR: Unexpected attribute type: %s", $attr_type);
                }

                my $dep_attr = $sym->getAttr($arg_ident);

                if (!$dep_attr) {
                    ERROR("Symbol %s has no attribute %s", $sym, $arg_ident);

                    ++$attr_error_cnt;

                    last DEP_DEF;
                }

                $dep = new CTXFryer::Grammar::Symbol::Attribute::Dependency($dep_attr->identifier(), $sym_no);

            }  # end of DEP_DEF block

            $dep ||= $CTXFryer::Grammar::Symbol::Attribute::Dependency::undef;

            push(@deps, $dep);
        }

        my $eval_def = $attr_def->function();
        my $eval;

        # Reference
        if ($eval_def eq "") {
            # Sanity check
            1 == @deps
            or FATAL("INTERNAL ERROR: Attribute %s is a reference to %d attributes",
                     $attr->identifier(), scalar(@deps));

            $eval = new CTXFryer::Grammar::Symbol::Attribute::Reference($rule_id, $attr_sym_no, @deps);
        }

        # Function
        else {
            $eval = new CTXFryer::Grammar::Symbol::Attribute::Function($rule_id, $attr_sym_no, $eval_def, @deps);
        }

        if ($eval) {
            $attr->evaluator($eval);
        }
        else {
            ERROR("Failed to create attribute %s evaluator", $attr);

            ++$attr_error_cnt;
        }
    }

    if ($attr_error_cnt) {
        ERROR("Grammar creation failed due %d attribution errors", $attr_error_cnt);

        return;
    }

    return $class->_new($non_terminals, $terminals, $root_non_terminal, $rules);
}


sub _new($$$$$$) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _non_terminals => shift,
        _terminals     => shift,
        _root          => shift,
        _rules         => shift,
    };

    bless($this, $class);

    # Bind rules
    for (my $rule_no = 0; $rule_no < $this->{_rules}->size(); ++$rule_no) {
        my $rule = $this->{_rules}->at($rule_no);

        $rule->grammar($this);
        $rule->number($rule_no);
    }

    # Bind terminals
    foreach my $t ($this->{_terminals}->values()) {
        $t->grammar($this);
    }

    # Bind non-terminals
    foreach my $nt ($this->{_non_terminals}->values()) {
        $nt->grammar($this);

        my $ident = $nt->ident();

        foreach my $rule ($this->{_rules}->items()) {
            if ($rule->left()->ident() eq $ident) {
                $nt->bindLHSrules($rule->number());
            }

            if (0 < grep($_->ident() eq $ident, $rule->right()->items())) {
                $nt->bindRHSrules($rule->number());
            }
        }
    }

    # Compute nullable non-terminals
    $this->_markNullableNonTerminals();

    return $this;
}


sub _tab($$@) {
    my $this = shift;
    my $name = shift;

    my $tab = $this->{$name}
    or FATAL("INTERNAL ERROR: Table %s doesn't exist", $name);

    @_ || return wantarray ? $tab->values() : $tab;

    my @cut = map($tab->at($_), @_);

    return 1 == @cut ? $cut[0] : @cut;
}


sub _list($$@) {
    my $this = shift;
    my $name = shift;

    my $list = $this->{$name}
    or FATAL("INTERNAL ERROR: List %s doesn't exist", $name);

    @_ || return wantarray ? $list->items() : $list;

    my @splice = map($list->at($_), @_);

    return 1 == @splice ? $splice[0] : @splice;
}


sub nonTerminal($@) {
    my $this = shift;

    return $this->_tab("_non_terminals", @_);
}


sub nullableNonTerminals($) {
    my $this = shift;

    return grep($_->isNullable(), $this->nonTerminal());
}


sub terminal($@) {
    my $this = shift;

    return $this->_tab("_terminals", @_);
}

sub root($) {
    my $this = shift;

    return $this->{_root};
}

sub rule($@) {
    my $this = shift;

    return $this->_list("_rules", @_);
}


sub reduce($) {
    my $this = shift;

    # 1st reduction phase
    my %r1_non_terminals;
    my %r1_rules;

    # Add all non-terminals N and rules N => X where X is terminal string
    foreach my $rule ($this->rule()) {
        my $right_nt_count = grep {
            UNIVERSAL::isa($_, "CTXFryer::Grammar::NonTerminal")
        } $rule->right();

        unless ($right_nt_count) {
            my $left = $rule->left();

            $r1_non_terminals{$left->id()} = $left;

            $r1_rules{$rule->number()} = $rule;

            DEBUG("1st phase init: accepted non-terminal %s and rule %s",
                  $left, $rule);
        }
    }

    # Close the set above by (transitively) adding non-terminals
    # that appear on left hand side of any rule that expands to
    # a string containing any non-terminal already in the set
    my @r1_new_non_terminals = values %r1_non_terminals;

    while (@r1_new_non_terminals) {
        my $nt = shift(@r1_new_non_terminals);

        foreach my $rule_number ($nt->RHSrules()) {
            my $rule    = $this->rule($rule_number);
            my $left    = $rule->left();
            my $left_id = $left->id();

            $r1_rules{$rule_number} = $rule;

            $r1_non_terminals{$left_id} && next;

            $r1_non_terminals{$left_id} = $left;

            push(@r1_new_non_terminals, $left);

            DEBUX(2, "1st phase: accepted non-terminal %s, by rule %s",
                     $left, $rule);
        }
    }

    DEBUG("1st phase concluded, %d non-terminals and %d rules accepted",
          scalar(keys %r1_non_terminals), scalar(keys %r1_rules));

    # 2nd reduction phase
    my $root = $this->root();

    my %r2_non_terminals = ($root->id() => $root);
    my %r2_rules;

    DEBUG("2nd phase init: accepted root non-terminal %s", $root);

    # Close the set by adding non-terminals reachable via rules
    # filtered in the 1st phase
    my @r2_new_non_terminals = $root;

    while (@r2_new_non_terminals) {
        my $nt = shift(@r2_new_non_terminals);

        foreach my $rule_number ($nt->LHSrules()) {
            $r1_rules{$rule_number} || next;

            my $rule = $this->rule($rule_number);

            $r2_rules{$rule_number} = $rule;

            my @right = grep {
                UNIVERSAL::isa($_, "CTXFryer::Grammar::NonTerminal")
            } $rule->right();

            foreach my $right (@right) {
                my $right_id = $right->id();

                $r2_non_terminals{$right_id} && next;

                $r2_non_terminals{$right_id} = $right;

                push(@r2_new_non_terminals, $right);

                DEBUX(2, "2nd phase: accepted non-terminal %s, by rule %s",
                         $right, $rule);
            }
        }
    }

    DEBUG("2nd phase: concluded, %d non-terminals and %d rules accepted",
          scalar(keys %r2_non_terminals), scalar(keys %r2_rules));

    # Create reduced grammar non-terminals
    my %symbol_map;

    my $non_terminals = new CTXFryer::Table;

    foreach my $nt (values %r2_non_terminals) {
        my $nt_copy = new CTXFryer::Grammar::NonTerminal($nt->ident());

        $symbol_map{$nt->id()} = $nt_copy;

        $non_terminals->at($nt_copy->ident(), $nt_copy);
    }

    # Create reduced grammar terminals
    my $terminals = new CTXFryer::Table;

    my @terminals = grep {
        UNIVERSAL::isa($_, "CTXFryer::Grammar::Terminal")
    }
    map {
        $_->right()
    }
    values %r2_rules;

    foreach my $t (@terminals) {
        my $t_copy = new CTXFryer::Grammar::Terminal($t->ident(), $t->regex());

        $symbol_map{$t->id()} = $t_copy;

        $terminals->at($t_copy->ident(), $t_copy);
    }

    # Create reduced grammar rules
    my %rule_id_map;  # implement by std::map<const ID, Rule>

    my $rules = new CTXFryer::List;

    foreach my $rule (sort { $a->number() <=> $b->number() } values %r2_rules) {
        my $left = $symbol_map{$rule->left()->id()};

        my @right = map($symbol_map{$_->id()}, $rule->right());

        my $rule_copy = new CTXFryer::Grammar::Rule($left => @right);

        $rule_id_map{$rule->id()} = $rule_copy;

        $rules->push($rule_copy);
    }

    # Copy attributes
    foreach my $sym (@terminals, values %r2_non_terminals) {
        my $sym_copy = $symbol_map{$sym->id()}
        or FATAL("INTERNAL ERROR: Failed to get symbol %s copy from symbol map", $sym);

        $sym_copy->attr(map($_->copy(\%rule_id_map), $sym->attr()));
    }

    return $this->_new($non_terminals, $terminals, $symbol_map{$this->root()->id()}, $rules);
}


sub augmented($) {
    my $this = shift;

    my $root = $this->root();

    my %symbol_map;
    my %rule_id_map;

    my $non_terminals = $this->nonTerminal();
    my $terminals     = $this->terminal();
    my $rules         = $this->rule();

    # Add new root
    my $new_root = new CTXFryer::Grammar::NonTerminal("_" . $root->ident());

    # Copy non-terminals
    my $new_non_terminals = new CTXFryer::Table;

    $new_non_terminals->at($new_root->ident(), $new_root);

    foreach my $nt ($non_terminals->values()) {
        my $nt_copy = new CTXFryer::Grammar::NonTerminal($nt->ident());

        $symbol_map{$nt->id()} = $nt_copy;

        $new_non_terminals->at($nt_copy->ident(), $nt_copy);
    }

    # Copy terminals
    my $new_terminals = new CTXFryer::Table;

    foreach my $t ($terminals->values()) {
        my $t_copy = new CTXFryer::Grammar::Terminal($t->ident(), $t->regex());

        $symbol_map{$t->id()} = $t_copy;

        $new_terminals->at($t_copy->ident(), $t_copy);
    }

    # Copy rules
    my $new_rules = new CTXFryer::List(new CTXFryer::Grammar::Rule($new_root => $symbol_map{$root->id()}));

    foreach my $rule ($rules->items()) {
        my $left = $symbol_map{$rule->left()->id()};

        my @right = map($symbol_map{$_->id()}, $rule->right());

        my $rule_copy = new CTXFryer::Grammar::Rule($left => @right);

        $rule_id_map{$rule->id()} = $rule_copy;

        $new_rules->push($rule_copy);
    }

    # Copy attributes
    foreach my $sym ($terminals->values(), $non_terminals->values()) {
        my $sym_copy = $symbol_map{$sym->id()}
        or FATAL("INTERNAL ERROR: Failed to get symbol %s copy from symbol map", $sym);

        $sym_copy->attr(map($_->copy(\%rule_id_map), $sym->attr()));
    }

    return $this->_new($new_non_terminals, $new_terminals, $new_root, $new_rules);
}


sub _markNullableNonTerminals($) {
    my $this = shift;

    # 1st step: filter rules of type A => X ,  X in N*
    my @rules = grep {
        my @right = $_->right();

        my $nt_only = 1;

        while ($nt_only && @right) {
            my $symbol = shift(@right);

            $nt_only &&= UNIVERSAL::isa($symbol, "CTXFryer::Grammar::NonTerminal");
        }

        $nt_only;
    }
    $this->rule();

    # 2nd step: loop through the rules and propagate
    # nullability right-to-left
    # Rules for nullable non-terminals are removed
    # during this process (since they can't change
    # anything any longer)
    my $set_cnt;

    do {
        $set_cnt = 0;

        DEBUX(1, "Checking for nullable non-terminals via %d rules",
                 scalar(@rules));

        for (my $i = 0; $i < @rules; ) {
            my $rule = $rules[$i];

            my $left = $rule->left();

            # Left non-terminal isn't declared nullable (yet)
            if (!$left->isNullable()) {
                my @right = $rule->right();
                my $is_nullable = 1;

                while ($is_nullable && @right) {
                    my $right = shift(@right);

                    $is_nullable &&= $right->isNullable();
                }

                # If the rule analysis is inconclusive then try next one
                $is_nullable or ++$i, next;

                # Nullable non-terminal detected
                DEBUX(1, "%s detected as nullable by %s",
                         $left, $rule);

                $left->isNullable(1);

                ++$set_cnt;
            }

            # Remove useless/used-up rule
            DEBUX(1, "Removing %s from list", $rule);

            splice(@rules, $i, 1);
        }

        DEBUX(1, "%d non-terminals were detected as nullable", $set_cnt);

    } while ($set_cnt);
}


sub str($) {
    my $this = shift;

    my $non_terminals = $this->nonTerminal();
    my $terminals     = $this->terminal();
    my $root          = $this->root();
    my $rules         = $this->rule();

    return ref($this) .
        "(non terminal symbols: $non_terminals, " .
        "terminal symbols: $terminals, " .
        "root: $root, " .
        "rules: $rules)";
}


sub xmlElementName { "grammar" }

sub xmlElementAttrs($) {
    my $this = shift;

    return (
        "root-non-terminal-id" => $this->root()->id(),
    );
}

sub xmlChildren($) {
    my $this = shift;

    my $non_terminals = $this->nonTerminal();
    my $terminals     = $this->terminal();
    my $rules         = $this->rule();

    return (
        xmlNewElement("non-terminals", {}, new CTXFryer::List($non_terminals->values())),
        xmlNewElement("terminals",     {}, new CTXFryer::List($terminals->values())),
        xmlNewElement("rules",         {}, $rules),
    );
}


1;
