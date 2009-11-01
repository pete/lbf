AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = lbf$(EXEEXT)
lbf_SOURCES = lbf.c lbf.h
include_HEADERS = lbf.h

always:

default: hello

hello: lbf$(EXEEXT) always
	./lbf$(EXEEXT) bf/hello_world.bf
