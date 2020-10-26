/*
 * trbitmap.c - display bitmap with transparent background
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include <shapewin.h>
#include "bmpload.h"

#include "trbitmap.h"
#include "trbitres.h"

/*
 * Uses two window for control transparent bitmap
 */

HWND    hwndFrame = NULLHANDLE ;    /* Invisible Frame Window   */
HWND    hwndShape = NULLHANDLE ;    /* Shape Window for Bitmap  */

/*
 * Bitmap to Display
 */

PUCHAR  pszBitmap = NULL       ;    /* Bitmap File Name     */

HDC     hdcBitmap = NULLHANDLE ;    /* Memory DC for Bitmap */
HPS     hpsBitmap = NULLHANDLE ;    /* Memory PS for Bitmap */
HBITMAP hbmBitmap = NULLHANDLE ;    /* Bitmap Handle        */

/*
 * myname - adjust and save program names
 */

UCHAR   ProgramPath[256] ;
UCHAR   ProgramName[256] ;

static  void    myname(PSZ me)
{
    PUCHAR  p, last ;

    /*
     * full pathname of program
     */

    for (p = me, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last != NULL) {
        strcpy(ProgramPath, me) ;
    } else if (DosSearchPath(7, "PATH", me, ProgramPath, 256) != 0) {
        strcpy(ProgramPath, me) ;
    }

    /*
     * basename of program
     */

    for (p = ProgramPath, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last == NULL) {
        strcpy(ProgramName, ProgramPath) ;
    } else {
        strcpy(ProgramName, &last[1]) ;
    }
    if ((p = strrchr(ProgramName, '.')) != NULL) {
        *p = '\0' ;
    }
}

/*
 * Error Notify
 */

void    trMessage(PSZ msg)
{
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, msg, ProgramName, 0, MB_OK) ;
}

/*
 * load/free Bitmap
 */

static  void    loadBitmap(HAB hab)
{
    SIZEL   siz ;
    
    hdcBitmap = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    hpsBitmap = GpiCreatePS(hab, hdcBitmap, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;

    if (hdcBitmap == NULLHANDLE || hpsBitmap == NULLHANDLE) {
        return ;
    }
    hbmBitmap = bitmapLoadFile(hab, hpsBitmap, pszBitmap) ;
}

static  void    freeBitmap(void)
{
    if (hbmBitmap != NULLHANDLE) {
        GpiDeleteBitmap(hbmBitmap) ;
    }
    if (hpsBitmap != NULLHANDLE) {
        GpiDestroyPS(hpsBitmap) ;
    }
    if (hdcBitmap != NULLHANDLE) {
        DevCloseDC(hdcBitmap) ;
    }
}

/*
 * createFrame - create frame window
 */

static  PFNWP   pfnFrame ;
static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;

#ifndef WS_TOPMOST
#define WS_TOPMOST  0x00200000
#endif

static  void    createFrame(HAB hab, BOOL ontop)
{
    FRAMECDATA  fcd ;
    ULONG       style = 0  ;
    UCHAR       title[256] ;

    memset(&fcd, 0, sizeof(fcd)) ;
    fcd.cb = sizeof(fcd) ;
    fcd.flCreateFlags = (FCF_TASKLIST | FCF_ICON) ;
    fcd.hmodResources = NULLHANDLE  ;
    fcd.idResources   = ID_TRBITMAP ;   

    sprintf(title, "%s [%s]", ProgramName, pszBitmap) ;
    
    if (ontop) {
        style |= WS_TOPMOST ;
    }

    hwndFrame = WinCreateWindow(
            HWND_DESKTOP,           /* Parent window handle     */
            WC_FRAME,               /* Frame Window Class       */
            title,                  /* as Title                 */
            style,                  /* Window Style             */
            0, 0, 0, 0,             /* Position & size          */
            NULLHANDLE,             /* Owner Window             */
            HWND_TOP,               /* Z-Order                  */
            0,                      /* Window ID                */
            &fcd,                   /* Control Data             */
            NULL) ;                 /* Presentation Parameter   */

    if (hwndFrame == NULLHANDLE) {
        return ;
    }

    pfnFrame = WinSubclassWindow(hwndFrame, procFrame) ;

    WinSendMsg(hwndFrame, WM_SETICON, 
        MPFROMP(WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_TRBITMAP)), NULL) ;
}

/*
 * createShape - create shape window
 */

static  void    createShape(HAB hab, BOOL ontop)
{
    BITMAPINFOHEADER2   bmi ;
    SHAPEWIN    shpctrl ;
    ULONG       style = 0 ;
    
    if (hwndFrame == NULLHANDLE) {
        return ;
    }
    if (hpsBitmap == NULLHANDLE || hbmBitmap == NULLHANDLE) {
        return ;
    }
    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmBitmap, &bmi) ;
    
    /*
     * Register Window Class
     */
     
    WinRegisterClass(hab, ShapeWinName, ShapeWinProc, 0L, sizeof(PVOID)) ;

    /*
     * Create Image Window
     */

    shpctrl.cx = bmi.cx ;
    shpctrl.cy = bmi.cy ;
    shpctrl.hpsDraw = hpsBitmap ;
    shpctrl.hpsMask = hpsBitmap ;
    
    if (ontop) {
        style |= WS_TOPMOST ;
    }

    hwndShape = WinCreateWindow(
            HWND_DESKTOP,           /* Parent Window    */
            ShapeWinName,           /* Window Class     */
	    NULL,                   /* Window Text      */
	    style,                  /* Window Style     */
	    0, 0, 0, 0,             /* Pos & Size       */
	    hwndFrame,              /* Owner Window     */
	    HWND_TOP,               /* Z-Order          */
	    0,                      /* Window ID        */
	    &shpctrl,               /* Control Data     */
	    NULL) ;                 /* Pres. Param.     */
    
    if (hwndShape == NULLHANDLE) {
        return ;
    }
}

/*
 * placeStartup - set startup position
 */

static  void    placeStartup(void)
{
    POINTL  pt ;
    SWP     swpScreen ;
    BITMAPINFOHEADER2   bmi ;
    SHORT   x, y ;
    
    if (hbmBitmap == NULLHANDLE || hwndFrame == NULLHANDLE || hwndShape == NULLHANDLE) {
        return ;
    }
    
    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmBitmap, &bmi) ;

    WinQueryPointerPos(HWND_DESKTOP, &pt) ;
    WinQueryWindowPos(HWND_DESKTOP, &swpScreen) ;

    x = pt.x ;
    y = pt.y ;
    
    if ((x + bmi.cx) > swpScreen.cx) {
        x = swpScreen.cx - bmi.cx ;
    }
    if ((y + bmi.cy) > swpScreen.cy) {
        y = swpScreen.cy - bmi.cy ;
    }
    
    WinSetWindowPos(hwndFrame, NULLHANDLE, 
            x, y, bmi.cx, bmi.cy, (SWP_MOVE | SWP_SIZE | SWP_HIDE)) ;
    WinSetWindowPos(hwndShape, NULLHANDLE, 
            x, y, bmi.cx, bmi.cy, (SWP_MOVE | SWP_SIZE | SWP_SHOW)) ;
}

/*
 * context menu
 */

static  HWND    hwndPopup = NULLHANDLE ;    /* context menu */
 
static  void    contextMenu(void)
{
    POINTL  pt   ;
    ULONG   opts ;
    
    if (hwndPopup == NULLHANDLE) {
        hwndPopup = WinLoadMenu(hwndFrame, NULLHANDLE, IDM_POPUP) ;
    }
    if (hwndPopup == NULLHANDLE) {
        printf("failed to load opup menu\n") ;
        return ;
    }
    
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;

    opts = PU_HCONSTRAIN | PU_VCONSTRAIN |
	     PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 ;

    WinPopupMenu(HWND_DESKTOP, hwndFrame, hwndPopup, 
                                pt.x, pt.y, IDM_EXIT, opts) ;
}
 
/*
 * procFrame - sub-classed frame window procedure for transparent bitmap
 *
 *      bitmap window send messages ti its owner (frame).  process those
 *      messages to perform bitmap display
 */

static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSWP    pswp ;
    SHORT   x, y ;
    
    TRACE("frame %08x %08x %08x\n", msg, mp1, mp2) ;
    
    switch (msg) {
        
    case WM_ADJUSTWINDOWPOS :
        pswp = (PSWP) PVOIDFROMMP(mp1) ;
	WinSetWindowPos(hwndShape, pswp->hwndInsertBehind,
        	    pswp->x, pswp->y, pswp->cx, pswp->cy, pswp->fl) ;
	pswp->fl &= ~SWP_SHOW ;
	pswp->fl |=  SWP_HIDE ;
	return (*pfnFrame) (hwnd, msg, mp1, mp2) ;

    case WM_SINGLESELECT :
        WinSetWindowPos(hwndShape, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER) ;
	return (MRESULT) 0 ;
	            
    case WM_BEGINDRAG :
        WinSendMsg(hwndFrame, WM_TRACKFRAME, 
	        MPFROMSHORT(TF_MOVE | TF_SETPOINTERPOS), NULL) ;
        return (MRESULT) 0 ;

    case WM_CONTEXTMENU :
        TRACE("WM_CONEXTMENU\n") ;
	contextMenu() ;
        return (MRESULT) 0 ;
	
    case WM_COMMAND :
        if (SHORT1FROMMP(mp1) == IDM_EXIT) {
	    WinPostMsg(hwnd, WM_CLOSE, NULL, NULL) ;
	    return (MRESULT) 0 ;
        }
	return (MRESULT) 0 ;
    }
    return (*pfnFrame) (hwnd, msg, mp1, mp2) ;
}
 
/*
 * main - program start here
 */

int     main(int ac, char *av[])
{
    HAB     hab  ;
    HMQ     hmq  ;
    QMSG    qmsg ;
    int     i    ;
    BOOL    ontop = FALSE ;
    UCHAR   msg[256] ;

    myname(av[0]) ;
    _wildcard(&ac, &av) ;

    hab = WinInitialize(0) ;
    hmq = WinCreateMsgQueue(hab, 0) ;
    
    /*
     * load given bitmap
     */
     
    for (i = 1, pszBitmap = NULL ; i < ac ; i++) {
        if (stricmp(av[i], "-ontop") == 0) {
	    ontop = TRUE ;
	} else if (pszBitmap == NULL) {
	    pszBitmap = av[i] ;
	}
    }
    if (pszBitmap == NULL) {
        sprintf(msg, "usage : %s bitmap-file", av[0]) ;
        trMessage(msg) ;
        freeBitmap() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    loadBitmap(hab) ;
    
    if (hbmBitmap == NULLHANDLE) {
        trMessage("failed to load bitmap") ;
        freeBitmap() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    /*
     * Start Window Processing
     */

    createFrame(hab, ontop) ;
    createShape(hab, ontop) ;

    if (hwndFrame == NULLHANDLE || hwndShape == NULLHANDLE) {    
        trMessage("failed to create windows") ;
	if (hwndFrame != NULLHANDLE) WinDestroyWindow(hwndFrame) ;
	if (hwndShape != NULLHANDLE) WinDestroyWindow(hwndShape) ;
        freeBitmap() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    placeStartup() ;
    
    while (WinGetMsg(hab, &qmsg, 0, 0, 0)) {
        WinDispatchMsg(hab, &qmsg) ;
    }

    /*
     * dispose resources
     */
    
    WinDestroyWindow(hwndFrame)  ;
    WinDestroyWindow(hwndShape) ;

    freeBitmap() ;
    
    WinDestroyMsgQueue(hmq) ;
    WinTerminate(hab) ;
    
    return 0 ; 
}
