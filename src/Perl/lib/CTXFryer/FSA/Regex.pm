package CTXFryer::FSA::Regex;

use strict;
use warnings;

use base qw(CTXFryer::Base CTXFryer::Serialised);

use CTXFryer::Logging qw(:all);

use CTXFryer::FSA;
use CTXFryer::FSA::Symbol::ASCII;


our %ASCII_map;
our %ASCII_esc;

our $debug = 0;


sub _ASCII_range($$) {
    my ($lo, $hi) = @_;

    return join("", map(chr($_), (ord($lo) .. ord($hi))));
}


sub _ASCII_complement($) {
    my $chars = shift;

    return join("", grep(index($chars, $_) == -1, @CTXFryer::FSA::Symbol::ASCII::all));
}


BEGIN {
    # Create ASCII symbol map
    %ASCII_map = map(($_ => new CTXFryer::FSA::Symbol::ASCII($_)),
                     @CTXFryer::FSA::Symbol::ASCII::all);
    # ASCII CR
    $ASCII_esc{'r'} = "\r";

    # ASCII LF
    $ASCII_esc{'n'} = "\n";

    # ASCII tab
    $ASCII_esc{'t'} = "\t";

    # ASCII whitespace
    $ASCII_esc{'s'} = " \t\n";

    # ASCII numeric
    $ASCII_esc{'d'} = _ASCII_range('0', '9');

    # ASCII alpha
    $ASCII_esc{'a'} = _ASCII_range('a', 'z') . _ASCII_range('A', 'Z');

    # ASCII word characters
    $ASCII_esc{'w'} = $ASCII_esc{'a'} . $ASCII_esc{'d'} . "_";

    # ASCII escaped characters complements
    my @esc = keys %ASCII_esc;

    foreach my $esc (@esc) {
        my $complement = uc($esc);

        $esc eq $complement and $complement = lc($esc);

        $ASCII_esc{$complement} = _ASCII_complement($ASCII_esc{$esc});
    }
}


sub new($$@) {
    my $class = shift; $class = ref $class || $class;

    my $def = shift;

    my $this = $class->SUPER::new();

    $this->{_def}      = $def;
    $this->{_base}     = "CTXFryer::FSA::Symbol::ASCII";
    $this->{_map}      = \%ASCII_map;
    $this->{_esc}      = \%ASCII_esc;
    $this->{_greedy}   = 1;
    $this->{_level}    = 0;
    $this->{_lang}     = $def;
    $this->{_dom_tree} = undef;

    bless($this, $class);

    @_ % 2 && FATAL("Options expected, got %s", \@_);

    my %opts = @_;

    while (my ($opt, $arg) = each %opts) {
        # Base
        if ($opt eq "-base") {
            $this->{_base} = $arg;
        }

        # Character -> symbol map
        elsif ($opt eq "-map") {
            $this->{_map} = $arg;
        }

        # Escaped character -> symbols map
        elsif ($opt eq "-esc") {
            $this->{_esc} = $arg;
        }

        # Greedy flag
        elsif ($opt eq "-greedy") {
            $this->{_greedy} = $arg ? 1 : 0;
        }

        # Nesting level
        elsif ($opt eq "-level") {
            $this->{_level} = $arg;
        }

        # Language
        elsif ($opt eq "-language") {
            $this->{_lang} = $arg;
        }

        # Unknown option
        else {
            FATAL("Unknown option: %s", $opt);
        }
    }

    return $this->_parse();
}


sub definition($) {
    my $this = shift;

    return $this->{_def};
}


sub base($) {
    my $this = shift;

    return $this->{_base};
}


sub isGreedy($) {
    my $this = shift;

    return $this->{_greedy};
}


sub language($) {
    my $this = shift;

    return $this->{_lang};
}


sub _map($@) {
    my $this = shift;

    my @sym;

    foreach my $char (@_) {
        my $sym = $this->{_map}->{$char};

        unless ($sym) {
            FATAL("Symbol base " . $this->base() . " does not define symbol for character $char");
        }

        push(@sym, $sym);
    }

    wantarray && return @sym;

    @sym > 1 && FATAL("Requested mapping of " . scalar(@sym) . " symbols in scalar context");

    return $sym[0];
}


sub _esc($@) {
    my $this = shift;

    @_ || return %{$this->{_esc}};

    my @sym;

    foreach my $char (@_) {
        my $chars = $this->{_esc}->{$char};

        if (defined $chars) {
            push(@sym, $this->_map(split("", $chars)));
        }
        else {
            push(@sym, $this->_map($char));
        }
    }

    return @sym;
}


sub _parse($@) {
    my $this = shift;
    my $def  = shift || $this->definition();
    my $pos  = shift || 0;

    my $len = length($def);

    # List of alternatives of lists of iterations of symbol sets or regexps
    my @dom_tree = ([]);

    # Escaped char flag
    my $esc;

    # Iteration specification
    my $iter;
    my $iter_lo;
    my $iter_hi;
    my $iter_param;

    # Symbol set specification
    my @set;
    my $set;
    my $set_neg;
    my $set_range;

    DEBUG("Parsing \"%s\" (level %d)", $def, $this->{_level});

    for (; $pos < $len; ++$pos) {
        my $char = substr($def, $pos, 1);

        DEBUG("Character '%s' at \"%s\"[%d]", $char, $def, $pos);

        my $sym;

        # Escaped character
        if ($esc) {
            my @esc_set = $this->_esc($char);

            # Set definition in progress
            if ($set) {
                push(@set, @esc_set);
                $set += @esc_set;
            }

            # Normal sequence
            else {
                @set = @esc_set;
                $set = "done";
            }

            $esc = undef;
        }

        # Iteration definition in progress
        elsif ($iter) {
            # Definition end
            if ($char eq '}') {
                $iter = "done";
            }

            # Lo/hi bounds separator
            elsif ($char eq ',') {
                $iter_param eq \$iter_hi
                and FATAL("Iteration specification syntax error (',' unexpected) at \"%s\"[%d]", $def, $pos);

                $iter_param = \$iter_hi;
            }

            # Parameter definition
            elsif ($char =~ /\d/) {
                $$iter_param ||= 0;
                $$iter_param  *= 10;
                $$iter_param  += $char;
            }

            # Nothing else works
            else {
                FATAL("Iteration specification syntax error at \"%s\"[%d]", $def, $pos);
            }
        }

        # Escape sequence
        elsif ($char eq '\\') {
            $esc = 1;
        }

        # Set definition in progress
        elsif ($set) {
            # Definition end
            if ($char eq ']') {
                @set or FATAL("Symbol set specification syntax error (empty) at \"%s\"[%d]", $def, $pos);

                $set = "done";
            }

            # Negative set
            elsif ($char eq '^' && $set == 1) {
                $set_neg = 1;
            }

            else {
                # Range
                if ($char eq '-' && $set > 1) {
                    $set_range = 1;
                }

                # Symbol
                else {
                    $sym = $this->_map($char);
                }

                ++$set;
            }
        }

        # Sub-regex
        elsif ($char eq '(') {
            my $sub_regex = new CTXFryer::FSA::Regex(substr($def, $pos + 1),
                                    "-base"     => $this->{_base},
                                    "-map"      => $this->{_map},
                                    "-esc"      => $this->{_esc},
                                    "-level"    => $this->{_level} + 1,
                                    "-language" => $this->{_lang});

            # Store to DOM tree
            my $iter = { obj => $sub_regex, lo => 1, hi => 1 };

            push(@{$dom_tree[-1]}, $iter);

            # Move the position to the closing parenthesis
            $pos += length($sub_regex->{_def}) + 1;
        }

        # Sub-regex ends
        elsif ($char eq ')') {
            DEBUG("Regex definition ended by ')' (level %d)", $this->{_level});

            $this->{_level} || FATAL("Unmatched ')' at \"%s\"[%d]", $def, $pos);

            $this->{_def} = substr($this->{_def}, 0, $pos);

            last;
        }

        # {0,} iteration
        elsif ($char eq '*') {
            $iter    = "done";
            $iter_lo = 0;
            $iter_hi = undef;

            $iter_param = \$iter_hi;
        }

        # {1,} iteration
        elsif ($char eq '+') {
            $iter    = "done";
            $iter_lo = 1;
            $iter_hi = undef;

            $iter_param = \$iter_hi;
        }

        # {0,1} iteration
        elsif ($char eq '?') {
            $iter    = "done";
            $iter_lo = 0;
            $iter_hi = 1;
        }

        # Iteration definition begins
        elsif ($char eq '{') {
            $iter    = 1;
            $iter_lo = 0;
            $iter_hi = undef;

            $iter_param = \$iter_lo;
        }

        # Symbol set definition begins
        elsif ($char eq '[') {
            $set = 1;
        }

        # Any symbol
        elsif ($char eq '.') {
            $set     = "done";
            $set_neg = 1;
        }

        # Alternative chain
        elsif ($char eq '|') {
            push(@dom_tree, []);
        }

        # Symbol
        else {
            $sym = $this->_map($char);

            DEBUG("Character '%s' mapped to %s", $char, $sym);
        }

        # Symbol set definition in progress
        if ($set) {
            # Symbol set definition is complete
            if ($set eq "done") {
                # Create symbol specification
                my $sym_spec = new CTXFryer::FSA::Symbol::Set(@set);

                if ($set_neg) {
                    if (@set) {
                        $sym_spec = CTXFryer::FSA::Symbol::Set::complement($sym_spec);
                    }
                    else {
                        my $lo = $this->base()->first();
                        my $hi = $this->base()->last();

                        my $interval = new CTXFryer::FSA::Symbol::Interval($lo, $hi);

                        $sym_spec = new CTXFryer::FSA::Symbol::Set($interval);
                    }
                }

                # Store to DOM tree
                my $iter = { obj => $sym_spec, lo => 1, hi => 1 };

                push(@{$dom_tree[-1]}, $iter);

                @set     = ();
                $set     = undef;
                $set_neg = undef;
            }

            # Set symbol defined
            elsif ($sym) {
                # Range hi bound defined
                if ($set_range) {
                    DEBUG("Set high bound %s defined", $sym);

                    my $lo = pop(@set);

                    DEBUG("Set low bound was %s", $lo);

                    my $interval = new CTXFryer::FSA::Symbol::Interval($lo, $sym);

                    push(@set, $interval);

                    $set_range = undef;
                }

                # Symbol defined
                else {
                    DEBUG("Set symbol %s defined", $sym);

                    push(@set, $sym);
                }

                $sym = undef;
            }
        }

        # Symbol defined
        elsif ($sym) {
            DEBUG("Symbol %s defined", $sym);

            # Create symbol specification
            my $sym_spec = new CTXFryer::FSA::Symbol::Set($sym);

            # Store to DOM tree
            my $iter = { obj => $sym_spec, lo => 1, hi => 1 };

            push(@{$dom_tree[-1]}, $iter);

            $sym = undef;
        }

        # Iteration definition is complete
        elsif ($iter && $iter eq "done") {
            # Adapt last iteration
            my $i = $dom_tree[-1]->[-1];

            $i || FATAL("Nothing to iterate at \"%s\"[%d]", $def, $pos);

            $i->{lo} = $iter_lo;
            $i->{hi} = $iter_hi;

            if (!defined $i->{hi} && $iter_param == \$iter_lo) {
                $i->{hi} = $iter_lo;
            }

            DEBUG("Iteration %d..%s defined", $i->{lo}, defined $i->{hi} ? $i->{hi} : "infinity");

            $iter = undef;
        }
    }

    $this->{_dom_tree} = \@dom_tree;

    return $this;
}


sub fsa($) {
    my $this = shift;

    my $def = $this->definition();

    my $lang = $this->language();

    DEBUG("Constructing FSA equivalent to %s", $this);

    my $dom_tree = $this->{_dom_tree};

    # Join iterations
    my @alt;

    for (my $a = 0; $a < @$dom_tree; ++$a) {
        my $alt = new CTXFryer::FSA("\"$def\" alternative $a");

        $alt->root()->accepts([ $lang ]);

        my $iters = $dom_tree->[$a];

        for (my $i = 0; $i < @$iters; ++$i) {
            my $iter = $iters->[$i];

            my $obj = $iter->{obj};
            my $lo  = $iter->{lo};
            my $hi  = $iter->{hi};

            # Iteration subject
            my $sub;

            # Symbol set iteration
            if (UNIVERSAL::isa($obj, "CTXFryer::FSA::Symbol::Set")) {
                $sub = new CTXFryer::FSA("\"$def\" alternative $a symbol set iteration $i");

                my $branch = new CTXFryer::FSA::Branch;
                my $target = new CTXFryer::FSA::State("\"$def\" alternative $a symbol set iteration $i");

                $target->accepts([ $lang ]);

                $branch->symbol($obj);
                $branch->target($target);

                $sub->root()->branch([ $branch ]);
            }

            # Sub-regex iteration
            elsif (UNIVERSAL::isa($obj, "CTXFryer::FSA::Regex")) {
                $sub = $obj->fsa();
            }

            # Internal error
            else {
                FATAL("Iteration object %s isn't a CTXFryer::FSA::Symbol::Set nor CTXFryer::FSA::Regex", $obj);
            }

            DEBUX(2, "\"$def\" alternative $a iteration $i subject: %s", $sub);

            my $sub_iter = CTXFryer::FSA::iteration($sub, $lo, $hi);

            DEBUX(2, "\"$def\" alternative $a iteration $i (%d..%d): %s", $lo, $hi, $sub_iter);

            $alt = CTXFryer::FSA::concat($alt, $sub_iter);
        }

        DEBUX(1, "\"$def\" alternative $a: %s", $alt);

        push(@alt, $alt);
    }

    # Unite alternatives
    my $fsa = CTXFryer::FSA::union(@alt);

    # Make accepting states terminal unless greedy (default)
    $this->isGreedy() || $fsa->setAcceptStatesTerminal();

    DEBUG("%s equivalent: %s", $this, $fsa);

    return $fsa;
}


sub str($) {
    my $this = shift;

    my $str = ref($this) . "(\"" . $this->definition() . "\", ";

    $str .= "language: " . $this->language() . ", ";
    $str .= "base: "     . $this->base()     . ", ";

    if ($debug && $debug > 1) {
        $str .= ", ";

        local $" = ", ";

        my @map;

        foreach my $char (sort keys %{$this->{_map}}) {
            my $sym = $this->{_map}->{$char};

            my $char_str = CTXFryer::FSA::Symbol::ASCII::printable($char);

            push(@map, "'$char' => $sym");
        }

        $str .= "map: (@map), ";

        my @esc;

        foreach my $char (sort keys %{$this->{_esc}}) {
            my $str = $this->{_esc}->{$char};

            my $printable_str = CTXFryer::FSA::Symbol::ASCII::printable($str);

            push(@esc, "'$char' => \"$printable_str\"");
        }

        $str .= "esc: (@esc), "
    }

    if ($debug) {
        $str .= "DOM tree: ";

        my $dom_tree = $this->{_dom_tree};

        my @alt_str;

        for (my $a = 0; $a < @$dom_tree; ++$a) {
            my $iters = $dom_tree->[$a];

            my @iter_str;

            for (my $i = 0; $i < @$iters; ++$i) {
                my $iter = $iters->[$i];

                my $obj = $iter->{obj};
                my $lo  = $iter->{lo};
                my $hi  = $iter->{hi};

                defined $hi or $hi = "infinity";

                push(@iter_str, "$lo..$hi of $obj");
            }

            local $" = ", ";

            push(@alt_str, scalar(@$iters) . " iterations: (@iter_str)");
        }

        local $" = ", ";

        $str .= scalar(@$dom_tree) . " alternatives: (@alt_str), ";
    }

    $str .= "greedy: " . ($this->isGreedy() ? "yes" : "no") . ")";

    return $str;
}


1;
