#
# Makefile for Shape Window Control & Sample Programs
#

CC	= gcc -c
CFLAGS	= -Zmtd -O2
LD	= gcc
LDFLAGS	= -s -Zmtd
EMXPATH = D:\emx

LIBS	=

#
# Inference Rules
#
.c.o :
	$(CC) $(CFLAGS) $*.c

#
# Target to Build
#

TARGET = trbitmap.exe tranime.exe treyes.exe

all : $(TARGET)

#
# Shape Window Control
#

SHSRCS = shapewin.c
SHOBJS = shapewin.o

shapewin.o : shapewin.c shapewin.h

#
# Bitmap Manipulation Library
#

BLSRCS = bmpload.c
BLOBJS = bmpload.o

bmpload.o : bmpload.c bmpload.h

#
# Display Bitmap with Transparent Background
#

BMSRCS = trbitmap.c
BMOBJS = trbitmap.o
BMLIBS = shapewin.o bmpload.o

trbitmap.exe : $(BMOBJS) $(BMLIBS) trbitmap.def trbitmap.res
	$(LD) $(LDFLAGS) -o trbitmap.exe trbitmap.def trbitmap.res $(BMOBJS) $(BMLIBS) $(LIBS)

trbitmap.res : trbitmap.rc trbitres.h trbitmap.ico
	rc -r -i $(EMXPATH)\include trbitmap.rc

trbitmap.o : trbitmap.c trbitmap.h trbitres.h shapewin.h bmpload.h

#
# Animate Bitmaps with Transparent Background
#

ANSRCS = tranime.c
ANOBJS = tranime.o
ANLIBS = shapewin.o bmpload.o

tranime.exe : $(ANOBJS) $(ANLIBS) tranime.def tranime.res
	$(LD) $(LDFLAGS) -o tranime.exe tranime.def tranime.res $(ANOBJS) $(ANLIBS) $(LIBS)

tranime.res : tranime.rc tranires.h tranime.ico
	rc -r -i $(EMXPATH)\include tranime.rc

tranime.o : tranime.c tranime.h tranires.h shapewin.h bmpload.h

#
# PM-Eyes with transparent background
#

EYSRCS = treyes.c
EYOBJS = treyes.o
EYLIBS = shapewin.o

treyes.exe : $(EYOBJS) $(EYLIBS) treyes.def treyes.res
	$(LD) $(LDFLAGS) -o treyes.exe treyes.def treyes.res $(EYOBJS) $(EYLIBS) $(LIBS)

treyes.res : treyes.rc treyeres.h treyes.ico
	rc -r -i $(EMXPATH)\include treyes.rc

treyes.o : treyes.c treyes.h treyeres.h shapewin.h
