# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.2 1997/04/28 01:10:06 oliver Exp $
# Makefile for AMUL
#

SHELL = /bin/sh

BINS = smuglcom smugl
DIRS = lib smuglcom smugl
SUBDIRS = $(DIRS)

release:
	@touch ./include/*.rev
	@make all

dep depend:
	for d in $(SUBDIRS) ; do :>$$d/Makefile.dep ; done
	for d in $(SUBDIRS) ; do $(MAKE) -C $$d $@ ; done

tidy:
	for d in $(SUBDIRS) ; do $(MAKE) -C $$d $@ ; done

fix-tidy:
	for d in $(SUBDIRS) ; do $(MAKE) -C $$d $@ ; done
