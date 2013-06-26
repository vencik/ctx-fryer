package CTXFryer::LRParser::Action;

use strict;
use warnings;

use base qw(CTXFryer::Serialised);


sub new($) {
    my $class = shift; $class = ref $class || $class;

    my $this = {
        _is_valid => 1,
    };

    return bless($this, $class);
}


sub isValid($) {
    my $this = shift;

    return $this->{_is_valid};
}


sub invalidate($) {
    my $this = shift;

    $this->{_is_valid} = undef;
}


sub str($@) {
    my $this  = shift;
    my @attrs = grep(defined, @_);

    my $str = ref($this);

    if (!$this->isValid()) {
        unshift(@attrs, "invalidated");
    }

    @attrs and $str .= '(' . join(", ", @attrs) . ')';

    return $str;
}


sub xmlElementAttrs($) {
    my $this = shift;

    return (
        "is-valid" => $this->isValid() ? "true" : "false",
    );
}


1;
