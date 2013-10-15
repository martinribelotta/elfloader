#TYPE?=host
TYPE?=target

all:
	$(MAKE) -f Makefile.$(TYPE).mk all

clean:
	$(MAKE) -f Makefile.$(TYPE).mk clean

debug:
	$(MAKE) -f Makefile.$(TYPE).mk debug
