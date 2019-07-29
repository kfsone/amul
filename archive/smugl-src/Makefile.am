# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.2 1997/04/28 01:10:06 oliver Exp $
# Makefile for AMUL
#

SHELL = /bin/sh

BINS = smuglcom smugl
DIRS = lib smuglcom smugl

all:
	@for i in $(DIRS) ; do cd $$i ; make ; cd .. ; done

depend:
	@echo "Making dependencies..."
	@for i in $(DIRS) ; do cd $$i ; :>Makefile.dep ; echo " - $$i" ; make depend ; cd .. ; done

tags:
	@echo "Making tags files..."
	@for i in $(DIRS) ; do cd $$i ; echo " - $$i" ; rm -f *TAGS* ; etags *.cc *.hpp ; cd .. ; done

clean:
	@for i in $(DIRS) ; do cd $$i ; make clean ; cd .. ; done

distclean:
	@for i in $(DIRS) ; do cd $$i ; make distclean ; cd .. ; done
	rm -f confdefs.h *.cache *.log *.status Makefile core *.core \
		h/Makefile h/config.h
	rm -rf test/Data/*

install:
	@for i in $(BINS) ; do cd $$i ; make install ; cd .. ; done

install-strip:
	@for i in $(BINS) ; do cd $$i ; make install-strip ; cd .. ; done
	
release:
	@touch h/*.rev
	@make all
