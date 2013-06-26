package CTXFryer::Serialised;

use strict;
use warnings;

use CTXFryer::Logging qw(:all);

use CTXFryer qw(:all);

use XML::LibXML;

use overload (
    '""' => "str",
    );

use vars qw(@ISA @EXPORT_OK %EXPORT_TAGS);

require Exporter;


@ISA = qw(Exporter);

@EXPORT_OK = qw(
    xmlNewElement
    );

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
    xml => [ "xmlNewElement" ],
    );


sub str($) {
    my $this = shift;

    FATAL("Class %s doesn't define method str", ref($this));
}


sub xmlElementName($) {
    my $this = shift;

    FATAL("Class %s doesn't define method xmlElementName", ref($this));
}


sub xmlElementAttrs($) {
    # By default, there are no attributes
    return;
}


sub xmlChildren($) {
    # By default, there are no children
    return;
}


sub xmlElement($);


sub xmlNewElement($$@) {
    my $name  = shift;
    my $attrs = shift;

    # Create new element
    my $element = new XML::LibXML::Element($name);

    # Set attributes
    foreach my $attr (sort { $a cmp $b } keys %$attrs) {
        my $val = $attrs->{$attr};

        $element->setAttribute($attr, $val);
    }

    # Add children
    foreach my $child (@_) {
        # Descend recursively
        if (UNIVERSAL::isa($child, "CTXFryer::Serialised")) {
            $element->addChild($child->xmlElement());
        }

        # Node pre-created
        elsif (UNIVERSAL::isa($child, "XML::LibXML::Node")) {
            $element->addChild($child);
        }

        # DOM creation failure: can't serialise a reference
        elsif (ref($child)) {
            FATAL("Non-scalar %s (an %s) can't be represented in XML",
                  $child, ref($child));
        }

        # Append text node for primitive scalars
        else {
            $element->appendTextNode($child);
        }
    }

    return $element;
}


sub xmlElement($) {
    my $this = shift;

    my $name     = $this->xmlElementName();
    my %attrs    = $this->xmlElementAttrs();
    my @children = $this->xmlChildren();

    return xmlNewElement($name, \%attrs, @children);
}


sub xml($) {
    my $this = shift;

    my $element = $this->xmlElement();

    # Namespaces
    $element->setAttributeNS("http://www.w3.org/2000/xmlns/", "xmlns:ctx-fryer", "ctx-fryer");
    $element->setAttributeNS("http://www.w3.org/2000/xmlns/", "xmlns:math",      "math");

    my $doc = new XML::LibXML::Document;

    $doc->setURI($xml_uri);
    $doc->setDocumentElement($element);

    return $doc->serialize($xml_indent_mode);
}


1;
