# Project webhosting
ftp_host = ftp.freehostia.com
ftp_root = lopucha.net/ctx-fryer


# Project web
html_css_dir = styles
html_js_dir  = scripts
html_img_dir = images
html_ico_dir = images/icons

htmldocdir = $(docdir)/HTML
htmlcssdir = $(htmldocdir)/$(html_css_dir)
htmljsdir  = $(htmldocdir)/$(html_js_dir)
htmlimgdir = $(htmldocdir)/$(html_img_dir)
htmlicodir = $(htmldocdir)/$(html_ico_dir)

htmldoc_DATA = \
    changelog.html \
    devel_guide.html \
    index.html

htmlcss_DATA = \
    $(html_css_dir)/asciidoc.css \
    $(html_css_dir)/toc2.css \
    $(html_css_dir)/ctx-fryer.css \
    $(html_css_dir)/ctx-fryer-toc.css

htmljs_DATA = \
    $(html_js_dir)/asciidoc.js \
    $(html_js_dir)/ctx-fryer.js

htmlimg_DATA = \
    $(html_img_dir)/ctx-fryer.png

htmlico_DATA = \
    $(html_ico_dir)/caution.png \
    $(html_ico_dir)/example.png \
    $(html_ico_dir)/home.png \
    $(html_ico_dir)/important.png \
    $(html_ico_dir)/next.png \
    $(html_ico_dir)/note.png \
    $(html_ico_dir)/prev.png \
    $(html_ico_dir)/tip.png \
    $(html_ico_dir)/up.png \
    $(html_ico_dir)/warning.png

.txt.html:
	$(ASCIIDOC) \
	    -a revdate="`date '+%d %B %Y'`" \
	    -a revnumber="$(PACKAGE_VERSION)" \
	    -a max-width=55em \
	    -a toc2 \
	    -a toc-title="" \
	    -a toclevels=3 \
	    -a icons \
	    -a icondir=$(html_ico_dir) \
	    -a linkcss \
	    -a stylesdir=$(html_css_dir) \
	    -a scriptsdir=$(html_js_dir) \
	    -a theme=ctx-fryer \
	    -a stylesheet=ctx-fryer-toc.css \
	    -o $@ $<

clean-local:
	-rm *.html


# Project web upload
noinst_SCRIPTS = ftp.sh

upload: install
	./ftp.sh --host $(ftp_host) --root $(ftp_root) $(htmldocdir)
