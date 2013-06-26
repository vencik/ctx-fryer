package CTXFryer::ProjectDef::Grammar::Rules;

use strict;
use warnings;

use base qw(CTXFryer::Base);

use CTXFryer::ProjectDef::Grammar::Rule;

use CTXFryer::Logging qw(:all);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        root   => undef,
        list   => [],
        map    => {},
        nt_map => {},
    };

    return bless($this, $class);
}


sub root($) {
    my $this = shift;

    return $this->{root};
}


sub all($) {
    my $this = shift;

    return @{$this->{list}};
}


sub nonTerminals($) {
    my $this = shift;

    return keys %{$this->{nt_map}};
}


sub no($$) {
    my ($this, $no) = @_;

    $no || FATAL("Rules are numbered starting with 1 (so that grammar augmentation may be done)");

    return $this->{list}->[--$no];
}


sub id($@) {
    my ($this, $id) = @_;

    if (defined $id) {
        my $no = $this->{map}->{$id};

        $no || return;

        return $this->no($no);
    }

    my @ids;

    while (my ($id, $no) = each %{$this->{map}}) {
        $ids[$no] = $id;
    }

    return @ids;
}


sub id2no($@) {
    my $this = shift;

    my @nos = map($this->{map}->{$_}, @_);

    wantarray && return @nos;

    1 == @_ && return $nos[0];

    return \@nos;
}


sub parse($$$) {
    my ($this, $line, $pos) = @_;

    my $ret = 0;

    # Rule definition
    if ($line =~ /^\s*(([A-Za-z_]\w*)\s*:\s*)?([A-Za-z_]\w*)\s*=>\s*(.*)\s*$/) {
        my $no    = 1 + @{$this->{list}};
        my $id    = $2 || $no;
        my $left  = $3;
        my @right = split(/\s+/, $4);

        # First non-terminals ever spotted is considered root
        $this->{root} ||= $left;

        # Rules left sides are the non-terminals
        $this->{nt_map}->{$left} = 1;

        # Rule IDs must be unique
        if (exists $this->{map}->{$id}) {
            ERROR("Rule ID %s reuse at %s", $id, $pos);

            ++$ret;
        }
        else {
            $this->{map}->{$id} = $no;

            my $rule = new CTXFryer::ProjectDef::Grammar::Rule(
                           number => $no,
                           lhs    => $left,
                           rhs    => [@right],
                           );

            push(@{$this->{list}}, $rule);
        }
    }

    # Parse error
    else {
        ERROR("Grammar rule definition syntax error at %s", $pos);

        $ret = 1;
    }

    return $ret;
}


1;
