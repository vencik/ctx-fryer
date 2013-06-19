# Path to shell scripts
sh_bin=`dirname "$0"`

# Path to install prefix
prefix=`echo "$sh_bin" | sed -e 's/src\/sh$//'`

# Path to Perl scripts and modules
perl_bin="$prefix/src/perl"
perl_lib="$prefix/src/perl/lib"

# Path to XML sheets
xml_lib="$prefix/src/xml"

# Path to target language specific templates
tlang_prefix="$prefix/tlang"


export PATH="$PATH:$sh_bin:$perl_bin"
export PERL5LIB="$PERL5LIB:$perl_lib"
