SUBDIRS = \
	aranym \
	bitplane \
	16_bit \
	uaegfx \
	saga \
	$(empty)

all clean install::
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

