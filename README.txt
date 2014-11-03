

	    ShapeWin - Shape (non-rectangular) Window for OS/2        
	    
                    Version 1.02

        Copyright (C) 1998, 1999 Software Research Associates, Inc.

License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

Description

    ShapeWindow is a custom control (widget) to implement shape (non
    rectangular) window for OS/2 PM.  This package also contains 3
    sample programs which uses shape window.

    See 'shapewin.txt' and sample sources how to use shape window 
    control.

    3 sample programs are
    
        trbitmap.exe    display bitmap with transparent background
	tranime.exe     animeation with several bitmaps
	treys.exe       'xeyes' look-a-like

    Usage of these programs are described in
    
        trbitmap.txt
	tranime.txt
	treyes.txt

    Notes that these sample program eat lot of CPU.  It bit heavy on home
    PC (486/66), but not so heavy on company's (working) Pentium 100
    PC.  Anyway, large bitmap, complicated bitmap result slow down your 
    machine.  So before you use large, complicated bitmaps, try little
    bitmaps first.
    
Update Histry

    1.02        speedup
    
        Included speedup patch from Mr.Moeller (ulrich.moeller@rz.hu-berlin.de)
        (He is author of XFolder).  With this patch CPU usage was down to
	half.

    1.01        bugfix & speedup

        Appended some mouse events notify.  Included speedup patch from
	Mr.Suwa (jjsuwa@ibm.net).  This patch make animation lighter,
	CPU usage was down to 80% to 60% on some animation.

    1.01        first release

Known Problems

    'Object Desktop' causes problems, a part of bitmap is not redrawn.

Contant

    Send bug report, questions, requests to

        akira@sra.co.jp

    Newest version will placed on
    
        http://www.sra.co.jp/people/akira/os2/shapewin-e.html
