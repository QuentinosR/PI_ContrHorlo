HEADERSDIR=/usr/include
BINDIR=bin
INCLUDEDIR=include

LIB_NAME=libsvgige.so
SO_NAME=$(LIB_NAME).2.4
REAL_NAME=$(SO_NAME).7
SHARED_LIB=$(BINDIR)/$(REAL_NAME)

all:
	@echo USAGE:
	@echo Run \"make install\" as root!
	@echo Next run \"make\" in ./console

install:
	rm -rf $(HEADERSDIR)/libsvgige
	mkdir $(HEADERSDIR)/libsvgige
	cp $(INCLUDEDIR)/svgige.h $(HEADERSDIR)/libsvgige
	chmod 644 $(HEADERSDIR)/libsvgige/svgige.h
	cp $(SHARED_LIB) /usr/lib/
	ln -f -s $(SO_NAME) /usr/lib/$(LIB_NAME)
	chmod 644 /usr/lib/$(REAL_NAME)
	ldconfig

uninstall:
	rm -rf $(HEADERSDIR)/libsvgige
	rm /usr/lib/$(LIB_NAME)*
	ldconfig


clean:
	cd console ; $(MAKE) clean ; cd ..
