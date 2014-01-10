##### Configuration #####

# Set ctx-fryer install prefix
CTX_FRYER_PREFIX = /home/vencik/test

# Set ctx-fryer-c-rt install prefix
CTX_FRYER_C_RT_PREFIX = $(CTX_FRYER_PREFIX)

##### Configuration end #####
include Makefile.config


CC = gcc -g -Wall -Werror -I$(CTX_FRYER_C_RT_PREFIX)/include -L$(CTX_FRYER_C_RT_PREFIX)/lib
RM = rm -rf

CTX_FRYER_PERLLIB = PERL5LIB=$${PERL5LIB}:$(CTX_FRYER_PREFIX)/share/perl/5.14.2
CTX_FRYER_LDLIB   = LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:$(CTX_FRYER_PREFIX)/lib
CTX_FRYER_PATH    = PATH=$${PATH}:$(CTX_FRYER_PREFIX)/bin

CALC_LDLIB = $(CTX_FRYER_LDLIB):./parser/C/.libs:.


#.PHONY: parser

all: envcheck parser calc check notice

calc: calc.o parser
	$(CC) -L. -L./parser/C/.libs -o $@ $< -lxattr -lparser -lctx-fryer-c-rt

calc.o: calc.c
	$(CC) -c -o $@ $<

parser: Expression_Factor_Term.cfg libxattr.so
	$(CTX_FRYER_PATH) $(CTX_FRYER_LDLIB) $(CTX_FRYER_PERLLIB) ctx-fryer cpro $< $@
	$(CTX_FRYER_PATH) $(CTX_FRYER_LDLIB) $(CTX_FRYER_PERLLIB) $(MAKE) --directory $@
	$(CTX_FRYER_PATH) $(CTX_FRYER_LDLIB) $(CTX_FRYER_PERLLIB) $(MAKE) --directory $@ check

libxattr.so: xattr.o
	$(CC) -shared -o $@ $^

xattr.o: xattr.c
	$(CC) -c -o $@ $<


clean: parser-clean
	$(RM) *.o

parser-clean:
	-@if test -d parser; then \
	    $(MAKE) --directory parser clean; \
	fi

purge: clean
	$(RM) parser
	$(RM) *.so calc

check: calc.sh calc
	$(CALC_LDLIB) ./$<

envcheck:
	$(CTX_FRYER_PATH) which ctx-fryer || $(MAKE) envcheck-error
	$(CTX_FRYER_PERLLIB) perl -MCTXFryer -e '1;' || $(MAKE) envcheck-error

envcheck-error:
	@echo ''
	@echo '=============================================================== '
	@echo 'Environment check failed'
	@echo ''
	@echo 'If you have installed ctx-fryer to a custom prefix,             '
	@echo 'please set PATH, LD_LIBRARY_PATH and PERL5LIB accordingly       '
	@echo '=============================================================== '
	@echo ''
	@false

notice:
	@echo ''
	@echo '=============================================================== '
	@echo 'Test program calc has been successfully built.                  '
	@echo 'Note that the parser libraries are dynamically linked;          '
	@echo 'there is no installation (for simplicity, so far),              '
	@echo 'so if you want to run the calculator, you need to specify       '
	@echo 'paths for the dynamic loader; i.e:                              '
	@echo ''
	@echo '$$ $(CALC_LDLIB) ./calc'
	@echo ''
	@echo 'You might also want to take a look at the documentation built   '
	@echo 'in the ./parser/doc directory.'
	@echo '=============================================================== '
	@echo ''
