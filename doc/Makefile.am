ftp_host = ftp.freehostia.com
ftp_root = lopucha.net/ctx-fryer

# Project web
htmldocdir = $(docdir)/HTML

htmldoc_DATA = \
    index.html

index.html: index.txt
	$(ASCIIDOC) -a revdate="`date '+%d %B %Y'`" -a revnumber="$(PACKAGE_VERSION)" $<

# Project web upload
noinst_SCRIPTS = ftp.sh

upload: install
	./ftp.sh --host $(ftp_host) --root $(ftp_root) $(htmldocdir)
