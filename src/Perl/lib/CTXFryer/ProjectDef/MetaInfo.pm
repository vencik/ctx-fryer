package CTXFryer::ProjectDef::MetaInfo;

use strict;
use warnings;

use base qw(CTXFryer::Base);

use CTXFryer::Logging qw(:all);


use constant META_SCOPE         => 0;
use constant DESCRIPTION_SCOPE  => 1;
use constant TLANGS_SCOPE       => 2;
use constant TLANG_CONFIG_SCOPE => 3;


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my $def_path = shift || $ENV{PWD};
    my $pwd_path = shift || $ENV{PWD};

    my $this = {
        label        => undef,
        author       => undef,
        descr        => undef,
        tlangs       => [],
        tlang_config => {},
        tlang        => undef,
        scope        => META_SCOPE,
        def_path     => $def_path,
        pwd_path     => $pwd_path,
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


sub targetLanguageConfig($@) {
    my $this = shift;

    if (@_) {
        my $tlang = shift;

        my $config = $this->{tlang_config}->{$tlang};

        return $config ? @$config : ();
    }

    return $this->{tlang_config};
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

    # Target language configure arguments
    elsif (META_SCOPE == $scope && $line =~ /^\s*TargetLanguage(\w+)Configuration\s*$/i) {
        $this->{scope} = TLANG_CONFIG_SCOPE;

        $this->{tlang} = $1;
        $this->{tlang_config}->{$1} = [];
    }

    # Target language configure arguments end
    elsif (TLANG_CONFIG_SCOPE == $scope && $line =~ /^\s*TargetLanguageConfigurationEnd\s*$/i) {
        $this->{scope} = META_SCOPE;

        $this->{tlang} = undef;
    }

    # Accumulate target language configure arguments
    elsif (TLANG_CONFIG_SCOPE == $scope) {
        my $tlang = $this->{tlang};

        my @conf = grep($_, split(/\s+/, $line));

        # Formal placeholders substitutions
        my $def_path = $this->{def_path};
        my $pwd_path = $this->{pwd_path};

        foreach (@conf) {
            s/\$PWD(\W)/$pwd_path$1/g;
            s/\$PWD$/$pwd_path/;
            s/\$\{PWD\}/$pwd_path/g;
            s/\$DEF_PATH(\W)/$def_path$1/g;
            s/\$DEF_PATH$/$def_path/;
            s/\$\{DEF_PATH\}/$def_path/g;
        }

        push(@{$this->{tlang_config}->{$tlang}}, @conf);
    }

    # Parse error
    else {
        ERROR("Syntax error at %s", $pos);

        $ret = 1;
    }

    return $ret;
}


1;
