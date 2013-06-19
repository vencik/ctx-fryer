# Common base
. command.sh
. paths.sh
. logging.sh

# Default log level
log_level=$LOG_LEVEL_INFO

# Project ID postfix
project_id_postfix=".CTXFryer.`date +%Y%m%d%H%M%S`"


# Basename
this=`basename $0`


# Usage
usage() {
    cat <<HERE
Usage: `$this` <project definition file>

HERE

    exit 1
}


# Modus
make_project=no
test "$this" = "make_project.sh" && make_project=yes


# Project definition file, path and ID
project_file=$1

test -n "$project_file" || usage
test -f "$project_file" || FATAL "$project_file isn't a file"

project_id=`basename "$project_file"`
project_id="$project_id$project_id_postfix"

project_path=`dirname $project_file`
if test -z "$project_path"; then
    project_path=`pwd`
    project_file="$project_path/$project_file"
elif test "$project_path" = '.'; then
    project_path=`pwd`
    project_file="$project_path/$project_file"
elif expr substr "$project_path" 1 1 != '/' >/dev/null; then
    project_path="`pwd`/$project_path"
    project_file="$project_path/$project_file"
fi

DEBUG "Project ID:   $project_id"
DEBUG "Project path: $project_path"
DEBUG "Project file: $project_file"

# Create project
INFO "Creating project $project_id"

project_home="$project_path/$project_id"
mkdir "$project_home" || FATAL "Failed to create project home directory $project_home"

project_file_link="$project_home/def_file"
ln -s "$project_file" "$project_file_link" || FATAL "Failed to create project file link"

tlangs=""
tlangs_test=""
tlangs_clean=""
tlangs_purge=""

tlangs_def=`grammar2tlang.pl "$project_file_link"` \
|| FATAL "Failed to resolve target languages of project"

for tlang in $tlangs_def; do
    tlang_dir_tpl="$tlang_prefix/$tlang/project"

    if test -d "$tlang_dir_tpl"; then
        tlang_dir="$project_home/$tlang"

        DEBUG "Creating target language $tlang directory"
        cp -r "$tlang_dir_tpl" "$tlang_dir" \
        || FATAL "Failed to create target language directory $tlang_dir"

        tlangs="$tlang $tlangs"
        tlangs_test="${tlang}-test $tlangs_test"
        tlangs_clean="${tlang}-clean $tlangs_clean"
        tlangs_purge="${tlang}-purge $tlangs_purge"
    else
        WARN "Sorry, target language $lang isn't supported"
    fi
done

project_doc="$project_home/doc"
mkdir "$project_doc" || FATAL "Failed to create project doc directory $project_doc"

# Create paths make file
cat > "$project_home/Makefile.paths" <<HERE
sh_bin       = $sh_bin
perl_bin     = $perl_bin
perl_lib     = $perl_lib
xml_lib      = $xml_lib
tlang_prefix = $tlang_prefix
HERE

# Create main make file
cat > "$project_home/Makefile" <<HERE
include Makefile.paths

cd   = cd
make = make
rm   = rm -f

# Add --log-position for logging of message position in the code
log_options = --log-level INFO --log-process

grammar2regex    = PERL5LIB=\${PERL5LIB}:\$(perl_lib) \$(perl_bin)/grammar2regex.pl
grammar2tlang    = PERL5LIB=\${PERL5LIB}:\$(perl_lib) \$(perl_bin)/grammar2tlang.pl
regex2fsa        = PERL5LIB=\${PERL5LIB}:\$(perl_lib) \$(perl_bin)/regex2fsa.pl \$(log_options)
grammar2lrparser = PERL5LIB=\${PERL5LIB}:\$(perl_lib) \$(perl_bin)/grammar2lrparser.pl \$(log_options)


.PHONY: all doc $tlangs

all: code documentation report

terminal_symbols_fsa.xml: $project_file_link
	\$(grammar2regex) < \$< | \$(regex2fsa) --output \$@

lr_parser.xml: $project_file_link
	\$(grammar2lrparser) --input "\$<" --output \$@

code: terminal_symbols_fsa.xml lr_parser.xml tlangs

tlangs: $tlangs

$tlangs:
	\$(MAKE) --directory=\$@

test: code $tlangs_test

$tlangs_test:
	for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d test; \
	done

tlangs_clean: $tlangs_clean

$tlangs_clean:
	for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d clean; \
	done

tlangs_purge: $tlangs_purge

$tlangs_purge:
	for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d purge; \
	done

documentation: terminal_symbols_fsa.xml lr_parser.xml
	\$(MAKE) --directory=doc

clean: tlangs_clean
	\$(MAKE) --directory=doc clean

purge: tlangs_purge
	\$(MAKE) --directory=doc purge
	\$(rm) \\
	    terminal_symbols_fsa.xml \\
	    lr_parser.xml

report:
	@echo
	@echo '=================================================='
	@echo
	@echo 'Project successfully fried.'
	@echo
	@echo 'File lr_parser.xml contains the parser definition.'
	@echo 'Documentation may be found in doc sub-directory.'
	@echo 'Generated code may be found in respective target'
	@echo 'language sub-directories.'
	@echo
	@echo 'You may now clean-up the project directory by'
	@echo '\$\$ make clean'
	@echo 'This will keep all generated target files while'
	@echo 'removing the build-time mess.'
	@echo 'To run automatic unit tests of the generated code'
	@echo 'you should execute'
	@echo '\$\$ make test'
	@echo '(this will check that all the tests defined in'
	@echo 'TestCase sections of the project def. file pass).'
	@echo 'You may also execute'
	@echo '\$\$ make purge'
	@echo 'to exterminate everything (do that if you change'
	@echo 'contents of the definition file).'
	@echo
	@echo 'Consume while still warm... ;-)'
	@echo
	@echo '=================================================='
	@echo
HERE

# Create documentation make file
cat > "$project_doc/Makefile" <<HERE
include ../Makefile.paths

rm = rm -f

tex2pdf      = latex -interaction nonstopmode -output-format pdf
tex2ps       = latex -interaction nonstopmode
dvi2ps       = dvips
dot2eps      = dot -Teps
fsa2dot      = xsltproc \$(xml_lib)/fsa2dot.xml
trans2dot    = xsltproc \$(xml_lib)/lrparser_trans2dot.xml
reads2dot    = xsltproc \$(xml_lib)/lrparser_reads2dot.xml
includes2dot = xsltproc \$(xml_lib)/lrparser_includes2dot.xml
lookback2dot = xsltproc \$(xml_lib)/lrparser_lookback2dot.xml
lrparser2tex = xsltproc \$(xml_lib)/lrparser2latex.xml

.PHONY: all lr_parser.ps

all: lr_parser.ps

lr_parser.ps: lr_parser.tex                  \
              terminal_symbols_fsa.eps       \
              lr_parser.transition_graph.eps \
              lr_parser.reads.eps            \
              lr_parser.includes.eps         \
              lr_parser.lookback.eps
	\$(rm) lr_parser.toc lr_parser.lof lr_parser.lot
	\$(tex2ps) lr_parser.tex
	\$(tex2ps) lr_parser.tex
	\$(dvi2ps) lr_parser.dvi

terminal_symbols_fsa.eps: terminal_symbols_fsa.dot
	\$(dot2eps) \$< > \$@

terminal_symbols_fsa.dot: ../terminal_symbols_fsa.xml
	\$(fsa2dot) \$< > \$@

lr_parser.transition_graph.eps: lr_parser.transition_graph.dot
	\$(dot2eps) \$< > \$@

lr_parser.transition_graph.dot: ../lr_parser.xml
	\$(trans2dot) \$< > \$@

lr_parser.reads.eps: lr_parser.reads.dot
	\$(dot2eps) \$< > \$@

lr_parser.reads.dot: ../lr_parser.xml
	\$(reads2dot) \$< > \$@

lr_parser.includes.eps: lr_parser.includes.dot
	\$(dot2eps) \$< > \$@

lr_parser.includes.dot: ../lr_parser.xml
	\$(includes2dot) \$< > \$@

lr_parser.lookback.eps: lr_parser.lookback.dot
	\$(dot2eps) \$< > \$@

lr_parser.lookback.dot: ../lr_parser.xml
	\$(lookback2dot) \$< > \$@

lr_parser.tex: ../lr_parser.xml
	\$(lrparser2tex) \$< > \$@

clean:
	\$(rm) \\
	    terminal_symbols_fsa.dot lr_parser.transition_graph.dot \\
	    terminal_symbols_fsa.eps lr_parser.transition_graph.eps \\
	    lr_parser.reads.dot lr_parser.includes.dot lr_parser.lookback.dot \\
	    lr_parser.reads.eps lr_parser.includes.eps lr_parser.lookback.eps \\
	    lr_parser.tex lr_parser.dvi lr_parser.aux lr_parser.log \\
	    lr_parser.toc lr_parser.lof lr_parser.lot

purge: clean
	\$(rm) lr_parser.ps
HERE

# Project creation done
INFO "Project created in sandbox"
INFO "$project_home"

if test "$make_project" = "yes"; then
    ( cd "$project_home"; make all test )
    test $? -eq 0 || FATAL "Project make failed; please report the bug"

    INFO "Project make completed in sandbox"
    INFO "$project_home"
else
    INFO "You may now proceed by changing to the directory and running make"
fi
