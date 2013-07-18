rev_date = $(shell date "+%d %B %Y")
ftp_host = ftp.freehostia.com
ftp_root = lopucha.net/ctx-fryer

# Project web
htmldocdir = $(docdir)/HTML

htmldoc_DATA = \
    index.html

%.html: %.txt
	$(ASCIIDOC) -a revdate="$(rev_date)" -a revnumber="$(PACKAGE_VERSION)" $<

# Project web upload
noinst_SCRIPTS = ftp.sh

upload: install
	./ftp.sh --host $(ftp_host) --root $(ftp_root) $(htmldocdir)
