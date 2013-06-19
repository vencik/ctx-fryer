package FSA::Symbol::ASCII;

use strict;
use warnings;

use base qw(FSA::Symbol);

use FSA::Symbol;

use Logging qw(:all);


our @all       = map(chr($_), 0..255);
our @printable = map(chr($_), 32..126);

our %printable = map(($_ => 1), @printable);


sub printable($) {
    my $str = "";

    foreach my $char (split("", shift)) {
        unless ($printable{$char}) {
            $char = sprintf("\\x%02x", ord($char));
        }

        $str .= $char;
    }

    return $str;
}


sub first($) {
    my $class = shift; $class = ref $class || $class;

    return new FSA::Symbol::ASCII($all[0]);
}


sub last($) {
    my $class = shift; $class = ref $class || $class;

    return new FSA::Symbol::ASCII($all[-1]);
}


sub equal($$) {
    my ($ch1, $ch2) = @_;

    return $ch1->{char} eq $ch2->{char};
}


sub less($$) {
    my ($ch1, $ch2) = @_;

    return ord($ch1->{char}) < ord($ch2->{char});
}


sub pred($) {
    my $this = shift;

    my $char = $this->{char};

    $char eq $all[0] && return;

    my $ord = ord($char);

    return new FSA::Symbol::ASCII(chr($ord - 1));
}


sub succ($) {
    my $this = shift;

    my $char = $this->{char};

    $char eq $all[-1] && return;

    my $ord = ord($char);

    return new FSA::Symbol::ASCII(chr($ord + 1));
}


sub distance($$) {
    my ($sym1, $sym2) = @_;

    my $ord1 = ord($sym1->{char});
    my $ord2 = ord($sym2->{char});

    return abs($ord1 - $ord2);
}


sub str($) {
    my $this = shift;

    my $char_str = printable($this->{char});

    return ref($this) . "('$char_str')";
}


sub xmlElementName { "symbol-ascii" }

sub xmlElementAttrs($) {
    my $this = shift;

    my $char_str = printable($this->{char});
    my $ord      = ord($this->{char});
    my $xord     = sprintf("0x%02x", $ord);
    my $is_first = not defined $this->pred();
    my $is_last  = not defined $this->succ();

    return (
        char  => $char_str,
        ord   => $ord,
        xord  => $xord,
        first => $is_first ? "true" : "false",
        last  => $is_last  ? "true" : "false",
    );
}


1;
