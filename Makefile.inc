all:
	for dir in $(dirs); do \
		$(MAKE) --directory=$$dir $@; \
	done

test: all
	for dir in $(dirs); do \
		$(MAKE) --directory=$$dir $@; \
	done

install: all
	for dir in $(dirs); do \
		$(MAKE) --directory=$$dir $@; \
	done

clean:
	for dir in $(dirs); do \
		$(MAKE) --directory=$$dir $@; \
	done

purge: clean
	for dir in $(dirs); do \
		$(MAKE) --directory=$$dir $@; \
	done
