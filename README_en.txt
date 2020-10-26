

	    ShapeWindow - Shape (non-rectangular) Window for OS/2        
	    
                    Version 2.00, Dec, 2001

        Copyright (C) 1998-2001 Software Research Associates, Inc.

License

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 
    59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
    
    GNU Lesser General Public License   LGPL_en.txt english
                                        LGPL_ja.txt japanese

    Bitmaps in 'bitmaps' directory are converted (to OS/2 bitmap) from
    Mr. Watanabe's (mailto:go@denpa.org) products under his permission.
    You can find original pictures in
        
	XMulti  (http://www.denpa.org/~go/xmulti)
	XMascot (http://cclub-flying.dsl.gr.jp/products/xmascot/)
                (http://cclub-flying.dsl.gr.jp/products/xmascot/dokyu2.html)

    Those characters are derived from 
        ToHeart                 by Leaf (http://leaf.aquaplus.co.jp/)
	Dokyusei2 (AKA nanpa2)  by Elf  (http://www.elf-game.co.jp/)
    and they hold rights on them. They, both, permit derived work (such
    as these bitmaps) distributed as hobby materials (no for commercial).

Description

    ShapeWindow is a custom control (widget) to implement shape (non
    rectangular) window for OS/2 PM.  This package also contains 3
    sample programs which uses shape window.

    See 'shapewin_en.txt' and sample sources how to use shape window 
    control.

    3 sample programs are:
    
        trbitmap.exe    display bitmap with transparent background
	tranime.exe     animeation with several bitmaps
	treys.exe       'xeyes' look-a-like

    You can try these programs as below.
    
    trbitmap bitmaps\misa.bmp
    
        Shows 'MISA' bitmap on your desktop. You can move picture with
        dragging right mouse button.  To stop it, click right mouse 
        button and select'exit'.  Move it over another window and see
        effect of shape window.

    tranime bitmaps\multi*.bmp
    
        It shows control dialog.  'start' starts animation, 'stop' stops
        animation, and 'exit' to exit program.  On start, 'MULTI'
        sweeps your desktop.

    treyes
    
        It shows eyes on your desktop.  Eye ball tracks your mouse 
        movement.  You can move eyes with draggin right mouse button.  
        You can bring up popup menu by clicking right mouse button, and 
	you can move, resize, stop from it.  When selected 'size', you
        see thin border around eyes, and move mouse to edge make resize
        on that edge.
    
    Detailed usages of these programs are desribed in manual pages in 
    'samples' directory.
    
        trbitmap_en.txt
	tranime_en.txt
	treyes_en.txt

    Notes that these sample program eat lot of CPU.  It bit heavy on 
    home PC (486/66), but not so heavy on company's (older) Pentium 100 
    PC, and almost no problems on current Pentium 800 PC.  Anyway, large
    bitmap, complicated bitmap result slow down your machine.  So before
    you use large, complicated bitmaps, try little bitmaps first.
    
Update Histry

    2.00        changed license, bug fix
    
        Make library into DLL, and changed license from GPL to LGPL.  
	I hope more 'nanjaku' prorgrams for OS/2.

	Included bug fix from Masami Kitamura <masamik@attglobal.net>.
        It fixes hpsMask (other than hpsDraw) works well.

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

    If shape window is overlapped with Java 1.3's window, it causes
    system hang.  No problem with Java 1.1.8.
    
    With some display drivers, image on shape window becomes broken. GPI
    operation seems depend on display drivers.

    'Object Desktop' causes problems, a part of bitmap is not redrawn.

Contant

    Send bug report, questions, requests to

        akira@sra.co.jp

    Newest version will placed on
    
        http://www.sra.co.jp/people/akira/os2/shapewin-e.html
