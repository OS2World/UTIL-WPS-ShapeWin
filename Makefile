#
# Makefile for Shape Window Control & Sample Programs
#

#
# Directories to install
#

prefix = @prefix@
bindir = $(prefix)/bin
dlldir = $(prefix)/dll
libdir = $(prefix)/lib
hdrdir = $(prefix)/include

#
# Tools to build DLL and Import Library
#

CC     = gcc
CFLAGS = -O2 -Zomf -Zcrtdll -Zmt

.SUFFIXES: .obj .o

#
# Build Shape Window Control
#

all : library samples

library : shapewin.dll shapewin.a shapewin.lib
samples : trbitmap.exe tranime.exe treyes.exe

SHDEFS = shapewin.def
SHSRCS = shapewin.c
SHOBJS = shapewin.obj

shapewin.obj : shapewin.c shapewin.h
	$(CC) -c $(CFLAGS)  shapewin.c

shapewin.dll : $(SHOBJS) $(SHDEFS)
	$(CC) -Zdll $(CFLAGS) -o $@ $(SHOBJS) shapewin.def

shapewin.lib : $(SHDEFS)
	rm -f shapewin.lib
	emximp -o shapewin.lib shapewin.def

shapewin.a : $(SHDEFS)
	rm -f shapewin.a
	emximp -o shapewin.a shapewin.def

#
# Sample Programs
#

samples : force
	(cd samples && $(MAKE) samples && cd ..)

#
# Install to the System
#

install : library force
	cp -i shapewin.h   $(hdrdir)
	cp -i shapewin.dll $(dlldir)
	cp -i shapewin.lib $(libdir)
	cp -i shapewin.a   $(libdir)

#
# Cleanup
#
clean : force
	rm -f *.obj *.lib *.a *.dll core
	(cd samples && $(MAKE) clean && cd ..)

#
# force to do
#
force :

