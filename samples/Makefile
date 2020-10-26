#
# Makefile for Shape Window Control & Sample Programs
#

CC	 = gcc -c
CFLAGS   = -O2 -Zmtd
LD	 = gcc
LDFLAGS	 = -s -Zmtd
EMXPATH  = D:\emx

#
# Target to Build
#

SAMPLES = trbitmap.exe tranime.exe treyes.exe

all : shapewin.dll shapewin.a $(SAMPLES) 

samples : $(SAMPLES) force
	cp -i trbitmap.exe ..
	cp -i tranime.exe ..
	cp -i treyes.exe ..

clean : force
	rm -f *.o *.obj *.dll *.lib *.a *.exe *.res

#
# Library to Test
#

shapewin.dll : ../shapewin.dll
	cp ../shapewin.dll .

shapewin.a   : ../shapewin.a
	cp ../shapewin.a .

#
# Bitmap Manipulation Library
#

BLSRCS = bmpload.c
BLOBJS = bmpload.o

bmpload.o : bmpload.c bmpload.h
	$(CC) $(CFLAGS) bmpload.c

#
# Display Bitmap with Transparent Background
#

BMSRCS = trbitmap.c
BMOBJS = trbitmap.o
BMLIBS = bmpload.o shapewin.a

trbitmap.exe : $(BMOBJS) $(BMLIBS) trbitmap.def trbitmap.res
	$(LD) $(LDFLAGS) -o trbitmap.exe trbitmap.def trbitmap.res $(BMOBJS) $(BMLIBS)

trbitmap.res : trbitmap.rc trbitres.h trbitmap.ico
	rc -r -i $(EMXPATH)\include trbitmap.rc

trbitmap.o : trbitmap.c trbitmap.h trbitres.h bmpload.h
	$(CC) $(CFLAGS) trbitmap.c

#
# Animate Bitmaps with Transparent Background
#

ANSRCS = tranime.c
ANOBJS = tranime.o
ANLIBS = bmpload.o shapewin.a

tranime.exe : $(ANOBJS) $(ANLIBS) tranime.def tranime.res
	$(LD) $(LDFLAGS) -o tranime.exe tranime.def tranime.res $(ANOBJS) $(ANLIBS)

tranime.res : tranime.rc tranires.h tranime.ico
	rc -r -i $(EMXPATH)\include tranime.rc

tranime.o : tranime.c tranime.h tranires.h bmpload.h
	$(CC) $(CFLAGS) tranime.c

#
# PM-Eyes with transparent background
#

EYSRCS = treyes.c
EYOBJS = treyes.o
EYLIBS = shapewin.a

treyes.exe : $(EYOBJS) $(EYLIBS) treyes.def treyes.res
	$(LD) $(LDFLAGS) -o treyes.exe treyes.def treyes.res $(EYOBJS) $(EYLIBS)

treyes.res : treyes.rc treyeres.h treyes.ico
	rc -r -i $(EMXPATH)\include treyes.rc

treyes.o : treyes.c treyes.h treyeres.h
	$(CC) $(CFLAGS) treyes.c

#

force :
