# We shall log
. "$ctx_fryer__path/ctx-fryer-paths.sh"
. "$ctx_fryer__path/ctx-fryer-logging.sh"

# Create project
ctx_fryer__create_project () {
    # Project definition file
    project_file="$1"

    test -n "$project_file" || FATAL "Project definition file not specified"
    test -n "$project_file" || FATAL "$project_file doesn't exist"
    test -f "$project_file" || FATAL "$project_file isn't a file"

    # Project ID
    project_id=`basename "$project_file"`
    project_id=`echo "$project_id" | sed -e 's/\.grammar$//'`

    # Project directory
    project_dir="$2"
    test -n "$project_dir" || project_dir="./$project_id.`date +%Y%m%d%H%M%S`"

    echo_project_dir="no"
    test "$3" = "echo_project_dir" && echo_project_dir="yes"

    DEBUG "Project ID:   $project_id"
    DEBUG "Project file: $project_file"
    DEBUG "Project dir:  $project_dir"

    # Create project
    INFO "Creating project $project_id"

    mkdir -p "$project_dir" || FATAL "Failed to create project directory $project_dir"

    cp -s "$project_file" "$project_dir/def_file" || FATAL "Failed to copy project file"
    project_file="$project_dir/def_file"

    tlangs=""
    tlangs_test=""
    tlangs_clean=""
    tlangs_purge=""
    tlangs_hint=1

    tlangs_def=`ctx-fryer-cfg2tlang "$project_file"` \
    || FATAL "Failed to resolve target languages of project"

    for tlang in $tlangs_def; do
        tlang_dir_tpl="$tlang_lib/$tlang"

        if test -d "$tlang_dir_tpl"; then
            tlang_dir="$project_dir/$tlang"

            DEBUG "Creating target language $tlang directory"
            cp -r "$tlang_dir_tpl" "$tlang_dir" \
            || FATAL "Failed to create target language directory $tlang_dir"

            tlangs="$tlang $tlangs"
            tlangs_test="${tlang}-test $tlangs_test"
            tlangs_clean="${tlang}-clean $tlangs_clean"
            tlangs_purge="${tlang}-purge $tlangs_purge"
        else
            WARN "Sorry, target language $tlang isn't supported (yet)"

            if test $tlangs_hint -gt 0; then
                WARN "It could be, though; perhaps you'd like to add the support?"
                WARN "All that's needed is to write XSLT transform of the parser"
                WARN "XML description to $tlang data structures and implement"
                WARN "the run-time support (i.e. the push-down automaton operating"
                WARN "over the parser tables)"
                WARN "See the development documentation if you're interested"
            fi
        fi
    done

    project_doc="$project_dir/doc"
    mkdir "$project_doc" || FATAL "Failed to create project doc directory $project_doc"

    # Create main make file
    cat > "$project_dir/Makefile" <<HERE
cd   = cd
make = make
rm   = rm -f

# Add --log-process  for logging process name and PID
# Add --log-position for logging message position in the code
log_options = --log-level INFO

grammar2regex    = ctx-fryer-cfg2re
grammar2tlang    = ctx-fryer-cfg2tlang
regex2fsa        = ctx-fryer-re2fsa \$(log_options)
grammar2lrparser = ctx-fryer-cfg2parser \$(log_options)


.PHONY: all doc $tlangs

all: code documentation report

terminal_symbols_fsa.xml: def_file
	\$(grammar2regex) < \$< | \$(regex2fsa) --output \$@

lr_parser.xml: def_file
	\$(grammar2lrparser) --input "\$<" --output \$@

code: terminal_symbols_fsa.xml lr_parser.xml tlangs

tlangs: tlangs_prepare tlangs_configure $tlangs

tlangs_prepare:
	@for d in $tlangs; do \
	    if test -f \$\$d/prepare.sh; then \
	        ( cd \$\$d; sh ./prepare.sh ) || break; \
	    fi \
	done

tlangs_configure:
	@for d in $tlangs; do \
	    if test -x \$\$d/configure; then \
	        ( cd \$\$d; ./configure --with-include="${prefix}/include" ) || break; \
	    fi \
	done

$tlangs:
	\$(MAKE) --directory=\$@

test: code tlangs_test

tlangs_test:
	@for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d test || break; \
	done

tlangs_clean:
	@for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d clean || break; \
	done

tlangs_purge:
	@for d in $tlangs; do \
	    \$(MAKE) --directory=\$\$d purge || break; \
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
rm = rm -f

tex2pdf      = latex -interaction nonstopmode -output-format pdf
tex2ps       = latex -interaction nonstopmode
dvi2ps       = dvips
dot2eps      = dot -Teps
fsa2dot      = xsltproc ${xml_lib}/fsa2dot.xml
trans2dot    = xsltproc ${xml_lib}/lrparser_trans2dot.xml
reads2dot    = xsltproc ${xml_lib}/lrparser_reads2dot.xml
includes2dot = xsltproc ${xml_lib}/lrparser_includes2dot.xml
lookback2dot = xsltproc ${xml_lib}/lrparser_lookback2dot.xml
lrparser2tex = xsltproc ${xml_lib}/lrparser2latex.xml

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
    INFO "Project created in $project_dir"
    INFO "You may now proceed by changing to the directory and running make"

    test "$echo_project_dir" = "yes" && echo "$project_dir"
}


# Build project
ctx_fryer__build_project () {
    project_dir=`ctx_fryer__create_project $* "echo_project_dir"`

    ( cd "$project_dir"; make all test )
    test $? -eq 0 || FATAL "Project build failed; please report the bug"

    INFO "Project buid completed in $project_dir"
}
