package CTXFryer::ProjectDef::MetaInfo;

use strict;
use warnings;

use base qw(CTXFryer::Base);

use CTXFryer::Logging qw(:all);


use constant META_SCOPE        => 0;
use constant DESCRIPTION_SCOPE => 1;
use constant TLANGS_SCOPE      => 2;


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        label   => undef,
        author  => undef,
        descr   => undef,
        tlangs  => [],
        scope   => META_SCOPE,
    };

    return bless($this, $class);
}


sub label($) {
    my $this = shift;

    return $this->{label};
}


sub author($) {
    my $this = shift;

    return $this->{author};
}


sub description($) {
    my $this = shift;

    return $this->{descr};
}


sub targetLanguages($) {
    my $this = shift;

    return @{$this->{tlangs}};
}


sub parse($$$) {
    my ($this, $line, $pos) = @_;

    my $ret   = 0;
    my $scope = $this->{scope};

    # Label
    if (META_SCOPE == $scope && $line =~ /^\s*label\s*=\s*(.*)\s*$/i) {
        defined $this->{label}
        and WARN("Redefinition of label at %s", $pos);

        $this->{label} = $1;
    }

    # Author
    elsif (META_SCOPE == $scope && $line =~ /^\s*author\s*=\s*(.*)\s*$/i) {
        defined $this->{author}
        and WARN("Redefinition of author at %s", $pos);

        $this->{author} = $1;
    }

    # Description start
    elsif (META_SCOPE == $scope && $line =~ /^\s*Description\s*$/i) {
        $this->{scope} = DESCRIPTION_SCOPE;
        $this->{descr} = "";
    }

    # Description end
    elsif (DESCRIPTION_SCOPE == $scope && $line =~ /^\s*DescriptionEnd\s*$/i) {
        $this->{scope} = META_SCOPE;
    }

    # Description line
    elsif (DESCRIPTION_SCOPE == $scope) {
        $line =~ s/^\s*//;
        $line =~ s/\s*$//;

        $this->{descr} .= $line . "\n";
    }

    # Target languages
    elsif (META_SCOPE == $scope && $line =~ /^\s*TargetLanguages\s*$/i) {
        $this->{scope} = TLANGS_SCOPE;
    }

    # Target languages end
    elsif (TLANGS_SCOPE == $scope && $line =~ /^\s*TargetLanguagesEnd\s*$/i) {
        $this->{scope} = META_SCOPE;
    }

    # Accumulate target language(s)
    elsif (TLANGS_SCOPE == $scope) {
        push(@{$this->{tlangs}}, grep($_, split(/\s+/, $line)));
    }

    # Parse error
    else {
        ERROR("Syntax error at %s", $pos);

        $ret = 1;
    }

    return $ret;
}


1;
