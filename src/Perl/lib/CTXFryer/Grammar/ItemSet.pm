package CTXFryer::Grammar::ItemSet;

use strict;
use warnings;

use base qw(CTXFryer::Signed);

use CTXFryer::Grammar::Item;

use CTXFryer::Signed;

use CTXFryer::Serialised qw(:xml);


sub _closure(@) {
    my @closure;

    my %item_tab;
    my @nt_2_expand;

    # Add kernel itself, first
    foreach my $item (@_) {
        my $signature = $item->signature();

        exists $item_tab{$signature}
        and FATAL("INTERNAL ERROR: %s appears multiple times in %s",
                  $item, \@_);

        push(@closure, $item);
        $item_tab{$signature} = 1;

        my $symbol = $item->symbol();

        UNIVERSAL::isa($symbol, "CTXFryer::Grammar::NonTerminal")
        and push(@nt_2_expand, $symbol);
    }

    # Expand dotted non-terminals
    while (@nt_2_expand) {
        my $nt = shift(@nt_2_expand);

        my $grammar = $nt->grammar();

        # For all rules expanding the non-terminal...
        foreach my $rule_number ($nt->LHSrules()) {
            my $rule = $grammar->rule($rule_number);

            my $item = new CTXFryer::Grammar::Item($rule, 0);

            my $signature = $item->signature();

            # ... add item with dot position 0 (unless already there)
            exists $item_tab{$signature} && next;

            push(@closure, $item);

            $item_tab{$signature} = 1;

            # ... and expand non-terminal on position 0 (if it's there)
            my $symbol = $item->symbol();

            UNIVERSAL::isa($symbol, "CTXFryer::Grammar::NonTerminal")
            and push(@nt_2_expand, $symbol);
        }
    }

    return @closure;
}


sub new($@) {
    my $class = shift; $class = ref $class || $class;

    my @kernel = @_;

    # Arguments sanity check
    foreach my $item (@kernel) {
        UNIVERSAL::isa($item, "CTXFryer::Grammar::Item")
        or FATAL("INTERNAL ERROR: %s isn't an item", $item);
    }

    # Compute minimal closure of the kernel
    my @closure = _closure(@kernel);

    # Assemble item set signature
    my $signature = join(",", map($_->signature(),
                         sort { $a <=> $b } @closure));

    # Create kernel map for easy containment checks
    my %kernel_map;

    for (my $i = 0; $i < @kernel; ++$i) {
        my $item = $kernel[$i];

        $kernel_map{$item->signature()} = $i;
    }

    # Create item map for easy containment checks
    my %item_map;

    for (my $i = 0; $i < @closure; ++$i) {
        my $item = $closure[$i];

        $item_map{$item->signature()} = $i;
    }

    # Create list of final items
    my @final_items;

    for (my $i = 0; $i < @closure; ++$i) {
        my $item = $closure[$i];

        $item->isFinal() && push(@final_items, $i);
    }

    my $this = $class->SUPER::new($signature);

    $this->{_kernel}      = \@kernel;
    $this->{_kernel_map}  = \%kernel_map;
    $this->{_items}       = \@closure;
    $this->{_item_map}    = \%item_map;
    $this->{_final_items} = \@final_items;

    return bless($this, $class);
}


sub kernel($) {
    my $this = shift;

    my $kernel = $this->{_kernel};

    return @$kernel;
}


sub items($) {
    my $this = shift;

    my $items = $this->{_items};

    return @$items;
}


sub _contains($$@) {
    my $this = shift;
    my $map  = shift;

    foreach my $item (@_) {
        # Sanity check
        UNIVERSAL::isa($item, "CTXFryer::Grammar::Item")
        or FATAL("INTERNAL ERROR: %s isn't a grammar item", $item);

        exists $map->{$item->signature()} || return;
    }

    return 1;
}


sub kernelContains($@) {
    my $this = shift;

    my $map = $this->{_kernel_map};

    return $this->_contains($map, @_);
}


sub contains($@) {
    my $this = shift;

    my $map = $this->{_item_map};

    return $this->_contains($map, @_);
}


sub finalItems($) {
    my $this = shift;

    my @items = $this->items();

    return map($items[$_], @{$this->{_final_items}});
}


sub xmlElementName { "item-set" }

sub xmlChildren($) {
    my $this = shift;

    my @items = $this->items();
    my @kernel;

    for (my $i = 0; $i < @items; ++$i) {
        my $item = $items[$i];

        $this->kernelContains($item) && push(@kernel, $i);
    }

    my $items_child = xmlNewElement(
        "items",
        {},
        new CTXFryer::List(@items)
    );

    my $kernel_child = xmlNewElement(
        "kernel",
        {},
        new CTXFryer::List(@kernel)
    );

    return (
        $items_child,
        $kernel_child,
    );
}


1;
