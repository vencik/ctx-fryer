package Math::Digraph;

use strict;
use warnings;

use base qw(Math::Couple);

use Math::Set;
use Math::Relation::Binary;

use Logging qw(:all);


sub new($$$) {
    my $class = shift; $class = ref $class || $class;

    my ($vertices, $edges) = @_;

    UNIVERSAL::isa($vertices, "Math::Set")
    or FATAL("INTERNAL ERROR: %s isn't a set", $vertices);

    UNIVERSAL::isa($edges, "Math::Relation::Binary")
    or FATAL("INTERNAL ERROR: %s isn't a binary relation", $edges);

    # Sanity check
    foreach my $couple ($edges->items()) {
        UNIVERSAL::isa($couple, "Math::Couple")
        or FATAL("INTERNAL ERROR: %s isn't a couple", $couple);

        foreach my $vertex ($couple->item()) {
            $vertices->contains($vertex)
            or FATAL("INTERNAL ERROR: Vertex set %s is inconsistent with edge set %s",
                     $vertices, $edges);
        }
    }

    my $this = $class->SUPER::new($vertices, $edges);

    return bless($this, $class);
}


sub vertices($) {
    my $this = shift;

    return $this->first();
}


sub edges($) {
    my $this = shift;

    return $this->second();
}


sub _determineSCC_traverse($$$$$$$$$@);

sub _determineSCC_traverse($$$$$$$$$@) {
    my $vertex   = shift;
    my $edges    = shift;
    my $stack    = shift;
    my $depths   = shift;
    my $infinity = shift;
    my $sccs     = shift;

    DEBUX(1, "Vertex: %s, stack: %s", $vertex, $stack);

    my $vertex_entering_callback     = shift;
    my $vertex_backtracking_callback = shift;
    my $vertex_finalising_callback   = shift;
    my @vertex_processing_cb_args    = @_;

    my $vertex_key   = Math::Set::itemKey($vertex);
    my $vertex_depth = \$depths->{$vertex_key};

    defined $$vertex_depth
    or FATAL("INTERNAL ERROR: vertex %s depth isn't defined; depths: %s",
             $vertex, $depths);

    0 == $$vertex_depth || return $$vertex_depth;

    # Enter vertex
    push(@$stack, $vertex);

    my $initial_vertex_depth = $$vertex_depth = scalar(@$stack);

    if (defined $vertex_entering_callback) {
        &$vertex_entering_callback(
            $vertex,
            @vertex_processing_cb_args);
    }

    # Recurse via edges
    my $vertex_edges_cut = $edges->cut($vertex);

    if (defined $vertex_edges_cut) {
        foreach my $vertex_edge_target_1plet ($vertex_edges_cut->items()) {
            my $vertex_edge_target = $vertex_edge_target_1plet->item(0);

            my $vertex_edge_target_depth = _determineSCC_traverse(
                $vertex_edge_target,
                $edges, $stack, $depths, $infinity, $sccs,
                $vertex_entering_callback,
                $vertex_backtracking_callback,
                $vertex_finalising_callback,
                @vertex_processing_cb_args);

            if ($vertex_edge_target_depth < $$vertex_depth) {
                $$vertex_depth = $vertex_edge_target_depth;
            }

            if (defined $vertex_backtracking_callback) {
                &$vertex_backtracking_callback(
                    $vertex, $vertex_edge_target,
                    @vertex_processing_cb_args);
            }
        }
    }

    # Finalise traversal
    if ($$vertex_depth == $initial_vertex_depth) {
        DEBUX(1, "Vertex %s is root of a SCC", $vertex);

        my @scc;
        my $popped_vertex_key;

        do {
            my $popped_vertex = pop(@$stack);

            # Sanity check
            defined $popped_vertex
            or FATAL("INTERNAL ERROR: no vertex on stack");

            $popped_vertex_key = Math::Set::itemKey($popped_vertex);

            $depths->{$popped_vertex_key} = $infinity;

            push(@scc, $popped_vertex);

            if (defined $vertex_finalising_callback) {
                &$vertex_finalising_callback(
                    $vertex, $popped_vertex,
                    @vertex_processing_cb_args);
            }

        } while ($popped_vertex_key ne $vertex_key);

        push(@$sccs, new Math::Set(@scc));
    }

    return $$vertex_depth;
}


sub determineSCC($@) {
    my $this = shift;

    my @sccs;

    my $vertex_entering_callback     = shift;
    my $vertex_backtracking_callback = shift;
    my $vertex_finalising_callback   = shift;
    my @vertex_processing_cb_args    = @_;

    my $vertices = $this->vertices();
    my $edges    = $this->edges();

    my @stack;
    my @vertices = $vertices->items();
    my %depths   = map((Math::Set::itemKey($_) => 0), @vertices);
    my $infinity = scalar(@vertices) + 1;

    DEBUG("Vertices: %s", \@vertices);

    foreach my $vertex (@vertices) {
        _determineSCC_traverse(
            $vertex,
            $edges, \@stack, \%depths, $infinity, \@sccs,
            $vertex_entering_callback,
            $vertex_backtracking_callback,
            $vertex_finalising_callback,
            @vertex_processing_cb_args);
    }

    # Sanity check
    @stack && FATAL("INTERNAL ERROR: Stack not empty: %s", \@stack);

    return @sccs;
}


1;
