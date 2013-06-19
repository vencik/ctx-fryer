package CTXFryer;

use strict;
use warnings;

use vars qw(
    @ISA
    @EXPORT_OK
    %EXPORT_TAGS
    $VERSION
    $header
    $xml_indent_mode
    $xml_uri
    $xml_element_class
    );

require Exporter;


@ISA = qw(Exporter);

@EXPORT_OK = qw(
    $VERSION
    $header
    $xml_indent_mode
    $xml_uri
    $xml_element_class
    );

%EXPORT_TAGS = (
    all => [ @EXPORT_OK ],
    xml => [ $xml_indent_mode, $xml_uri, $xml_element_class ],
    );


$VERSION = 0.1;

$header = <<HERE;
CTX-fryer version $VERSION
The context-free languages titanium pan ;-)
Author: Vaclav Krpec <vencik\@razdva.cz>
HERE


# Indentation mode:
# 0 means "as is"
# 1 means "add ignorabe white spaces to make the doc. readable"
# 2 means "as 1 and add leading and trailing line breakes to text nodes"
$xml_indent_mode   = 2;
$xml_uri           = "http://www.ctx-fryer.org/";
$xml_element_class = "ctx-fryer";


1;
