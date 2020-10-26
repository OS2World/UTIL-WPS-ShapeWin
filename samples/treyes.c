/*
 * treyes.c - eyes watches mouse pointer with shape window
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INCL_PM
#include <os2.h>

#include <shapewin.h>

#include "treyes.h"
#include "treyeres.h"

/*
 * Uses two window for control transparent bitmap
 */

HWND    hwndFrame = NULLHANDLE ;    /* Invisible Frame Window       */
HWND    hwndShape = NULLHANDLE ;    /* Shape Window for Eye Image   */

/*
 * Uses Memory DC/PS and Bitmap to manage eye image
 */

HDC     hdcMem = NULLHANDLE ;
HPS     hpsMem = NULLHANDLE ;
HBITMAP hbmMem = NULLHANDLE ;

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
 * initPosSize - initial Size & Positions from argument (or default)
 */

static  void    initPosSize(int ac, char *av[], PSWP swp)
{
    SWP     swpScr ;
    POINTL  ptCur  ;
    int     i      ;

    TRACE("InitPosSize\n") ;
    
    /*
     * set default size, position to pointer and 10th of screen size
     */

    WinQueryPointerPos(HWND_DESKTOP, &ptCur) ;
    WinQueryWindowPos(HWND_DESKTOP, &swpScr) ;
    
    swp->x = ptCur.x ;
    swp->y = ptCur.y ;
    swp->cx = swpScr.cx / 8 ;
    swp->cy = swpScr.cy / 8 ;

    /*
     * change them if option specified
     */

    for (i = 1 ; i < ac ; i++) {
        if (av[i][0] != '-') {
	    continue ;
	}
	switch (av[i][1]) {
	case 'w' :
	case 'W' :
	    if (av[i][2] != '\0') {
	        swp->cx = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        swp->cx = atoi(av[i+=1]) ;
	    }
	    break ;
	case 'h' :
	case 'H' :
	    if (av[i][2] != '\0') {
	        swp->cy = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        swp->cy = atoi(av[i+=1]) ;
	    }
	    break ;
	case 'x' :
	case 'X' :
	    if (av[i][2] != '\0') {
	        swp->x = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        swp->x = atoi(av[i+=1]) ;
	    }
	    break ;
	case 'y' :
	case 'Y' :
	    if (av[i][2] != '\0') {
	        swp->y = atoi(&av[i][2]) ;
	    } else if ((i + 1) < ac) {
	        swp->y = atoi(av[i+=1]) ;
	    }
	    break ;
	}
    }
    
    /*
     * adjust position to fit screen
     */
    
    if (swp->cx > swpScr.cx) {
        swp->cx = swpScr.cx ;
    }
    if (swp->cy > swpScr.cy) {
        swp->cy = swpScr.cy ;
    }
    if ((swp->x + swp->cx) > swpScr.cx) {
        swp->x = swpScr.cx - swp->cx ;
    }
    if ((swp->y + swp->cy) > swpScr.cy) {
        swp->y = swpScr.cy - swp->cy ;
    }
    
    TRACE("initPosSize pos %d %d, size %d, %d\n", swp->x, swp->y, swp->cx, swp->cy) ;
}

/*
 * Uses Memory DC/PS to manage eye image bitmap
 */

static  void    createMemorySpace(HAB hab)
{
    SIZEL   siz ;
    
    TRACE("createMemorySpace\n") ;
    
    hdcMem = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    hpsMem = GpiCreatePS(hab, hdcMem, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;

    if (hdcMem == NULLHANDLE || hpsMem == NULLHANDLE) {
        if (hdcMem != NULLHANDLE) DevCloseDC(hdcMem)   ;
	if (hpsMem != NULLHANDLE) GpiDestroyPS(hpsMem) ;
	hdcMem = NULLHANDLE ;
	hpsMem = NULLHANDLE ;
	TRACE("createMemorySpace - failed\n") ;
    }
}

static  void    disposeMemorySpace(void)
{
    TRACE("disposeMemorySpace\n") ;
    
    if (hpsMem != NULLHANDLE) {
        if (hbmMem != NULLHANDLE) {
	    GpiSetBitmap(hpsMem, NULLHANDLE) ;
	    GpiDeleteBitmap(hbmMem) ;
	}
        GpiDestroyPS(hpsMem) ;
    }
    if (hdcMem != NULLHANDLE) {
        DevCloseDC(hdcMem) ;
    }
    hpsMem = NULLHANDLE ;
    hdcMem = NULLHANDLE ;
}

/*
 * draw eye image on memory PS
 */

#define SIZE_DIV        16
#define SIZE_OUT        15
#define SIZE_INN        13
#define SIZE_EYE        4

#define COLOR_BLACK     CLR_BLACK
#define COLOR_WHITE     CLR_WHITE
#define COLOR_BACK      CLR_BLUE        /* not BLACK nor WHITE  */

static  SIZEL   drawSize ;

static  void    drawBack(int cx, int cy)
{
    POINTL  pt1, pt2 ;
    ARCPARAMS   arcParam ;
    
    TRACE("drawBack %d x %d\n", cx, cy) ;

    drawSize.cx = cx ;
    drawSize.cy = cy ;
    
    if (hpsMem == NULLHANDLE) {
        TRACE("drawBack - no Memory PS\n") ;
        return ;
    }
    
    /*
     * Fill Background
     */
     
    GpiSetColor(hpsMem, COLOR_BACK) ;
    pt1.x = 0 ;
    pt1.y = 0 ;
    GpiMove(hpsMem, &pt1) ;
    pt2.x = drawSize.cx ;
    pt2.y = drawSize.cy ;
    GpiBox(hpsMem, DRO_FILL, &pt2, 0, 0) ;

    /*
     * Calc Centers
     */
    
    pt1.y = pt2.y = drawSize.cy / 2 ;
    pt1.x = (drawSize.cx * 1) / 4 ;
    pt2.x = (drawSize.cx * 3) / 4 ;
    
    /*
     * Draw Outer (BLACK) circle
     */
    
    arcParam.lP = (drawSize.cx / 4) * SIZE_OUT / SIZE_DIV ;
    arcParam.lQ = (drawSize.cy / 2) * SIZE_OUT / SIZE_DIV ;
    arcParam.lR = 0 ;
    arcParam.lS = 0 ;
    GpiSetArcParams(hpsMem, &arcParam) ;
    
    GpiSetColor(hpsMem, COLOR_BLACK) ;
    GpiMove(hpsMem, &pt1) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;
    GpiMove(hpsMem, &pt2) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;
}

static  void    drawBallCalcPos(PPOINTL base, PPOINTL ptr, PPOINTL eye)
{
    int     ix, iy, rad ;
    double  r ;
    
    ix = (drawSize.cx / 4) * (SIZE_INN - SIZE_EYE) / SIZE_DIV ;
    iy = (drawSize.cy / 2) * (SIZE_INN - SIZE_EYE) / SIZE_DIV ;
    rad = (ix < iy) ? ix : iy ;
    TRACE("CalcPos rad %d\n", rad) ;

    ix = ptr->x - base->x ;
    iy = ptr->y - base->y ;
    
    if (((ix *  ix) + (iy * iy)) <= (rad * rad)) {
        eye->x = ptr->x ;
	eye->y = ptr->y ;
    } else {
        r = atan2((double) ix, (double) iy) ;
	eye->x = base->x + (LONG) (sin(r) * (double) rad) ;
	eye->y = base->y + (LONG) (cos(r) * (double) rad) ;
    }
    TRACE("CalcPos pos %d %d\n", eye->x, eye->y) ;
}

static  POINTL  ptLastC = { -1, -1 } ;
static  POINTL  ptLastR = { 0, 0 } ;
static  POINTL  ptLastL = { 0, 0 } ;

static  void    drawBall(BOOL restrict)
{
    ARCPARAMS   arcParam ;
    POINTL      pt, ptCR, ptCL, ptER, ptEL ;
    RECTL       rect ;
    
    TRACE("drawBall\n") ;
    
    if (hpsMem == NULLHANDLE) {
        TRACE("drawBall - no Memory PS\n") ;
        return ;
    }
    
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;
    WinMapWindowPoints(HWND_DESKTOP, hwndFrame, &pt, 1) ;

    if (pt.x == ptLastC.x && pt.y == ptLastC.y) {
        TRACE("drawBall - not changed\n") ;
        return ;
    }
    
    ptCL.y = ptCR.y = drawSize.cy / 2 ;
    ptCL.x = (drawSize.cx * 1) / 4 ;
    ptCR.x = (drawSize.cx * 3) / 4 ;

    /*
     * Draw Inner Circle to clear last eye ball
     */
     
    arcParam.lP = (drawSize.cx / 4) * SIZE_INN / SIZE_DIV ;
    arcParam.lQ = (drawSize.cy / 2) * SIZE_INN / SIZE_DIV ;
    arcParam.lR = 0 ;
    arcParam.lS = 0 ;
    GpiSetArcParams(hpsMem, &arcParam) ;

    GpiSetColor(hpsMem, COLOR_WHITE) ;
    GpiMove(hpsMem, &ptCL) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;
    GpiMove(hpsMem, &ptCR) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;

    if (restrict) { 
        rect.xLeft   = ptCL.x - (drawSize.cx / 4) * SIZE_INN / SIZE_DIV ;
        rect.xRight  = ptCR.x + (drawSize.cx / 4) * SIZE_INN / SIZE_DIV ;
        rect.yTop    = ptCL.y + (drawSize.cy / 2) * SIZE_INN / SIZE_DIV ;
        rect.yBottom = ptCL.y - (drawSize.cy / 2) * SIZE_INN / SIZE_DIV ;
    }
    
    /*
     * draw eye ball
     */
     
    arcParam.lP = (drawSize.cx / 4) * SIZE_EYE / SIZE_DIV ;
    arcParam.lQ = (drawSize.cy / 2) * SIZE_EYE / SIZE_DIV ;
    arcParam.lR = 0 ;
    arcParam.lS = 0 ;
    GpiSetArcParams(hpsMem, &arcParam) ;
    
    drawBallCalcPos(&ptCL, &pt, &ptEL) ;
    drawBallCalcPos(&ptCR, &pt, &ptER) ;
    
    GpiSetColor(hpsMem, COLOR_BLACK) ;
    GpiMove(hpsMem, &ptEL) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;
    GpiMove(hpsMem, &ptER) ;
    GpiFullArc(hpsMem, DRO_FILL, (FIXED) 0x10000) ;

    ptLastC = pt   ;
    ptLastL = ptEL ;
    ptLastR = ptER ;
    
    if (hwndShape != NULLHANDLE) {
        if (restrict) {
            WinSendMsg(hwndShape, SHAPEWIN_MSG_UPDATE, MPFROMP(&rect), NULL) ;
        } else {
            WinSendMsg(hwndShape, SHAPEWIN_MSG_UPDATE, NULL, NULL) ;
        }
    }
}

/*
 * bitmap & window to display eye image
 */

static  void    adjustImage(PSWP swp)
{
    BITMAPINFOHEADER2   bmi ;
    HBITMAP     hbm  ;
    HWND        hwnd ;
    SHAPEWIN    shpctrl ;
    
    TRACE("adjustImage\n") ;
    
    if (hdcMem == NULLHANDLE || hpsMem == NULLHANDLE) {
        TRACE("adjustImage - no Memory PS\n") ;
        return ;
    }
    if (hwndFrame == NULLHANDLE) {
        TRACE("adjustImage - no Frame Window\n") ;
        return ;
    }

    /*
     * if bitmap already exist and having same size, use it
     */

    if (hbmMem != NULLHANDLE) {
        bmi.cbFix = sizeof(bmi) ;
	GpiQueryBitmapInfoHeader(hbmMem, &bmi) ;

	if (bmi.cx == swp->cx && bmi.cy == swp->cy) {
	    return ;
	}
	GpiSetBitmap(hpsMem, NULLHANDLE) ;  /* unlink from Memory PS */
    }
    
    /*
     * create new bitmap of given size
     */
    
    memset(&bmi, 0, sizeof(bmi)) ;
    bmi.cbFix = sizeof(bmi) ;
    bmi.cx = swp->cx ;
    bmi.cy = swp->cy ;
    bmi.cPlanes       = 1  ;
    bmi.cBitCount     = 24 ;
    bmi.ulCompression = 0  ;
    bmi.cclrUsed      = 0  ;
    bmi.cclrImportant = 0  ;

    hbm = GpiCreateBitmap(hpsMem, &bmi, 0, NULL, NULL) ;
    GpiSetBitmap(hpsMem, hbm) ;
    
    /*
     * credate new window with new bitmap
     */

    if (hbm == NULLHANDLE) {
        TRACE("adjustImage - failed to create Bitmap\n") ;
        return ;
    }

    drawBack(swp->cx, swp->cy) ;
    drawBall(FALSE) ;

    shpctrl.cx = swp->cx ;
    shpctrl.cy = swp->cy ;
    shpctrl.hpsDraw = hpsMem ;
    shpctrl.hpsMask = hpsMem ;
    
    hwnd = WinCreateWindow(
            HWND_DESKTOP,           /* Parent Window    */
            "ShapeWin",             /* Window Class     */
	    NULL,                   /* Window Text      */
	    0,                      /* Window Style     */
	    swp->x,  swp->y,
	    swp->cx, swp->cy,
	    hwndFrame,              /* Owner Window     */
	    HWND_TOP,               /* Z-Order          */
	    0,                      /* Window ID        */
	    &shpctrl,               /* Control Data     */
	    NULL) ;                 /* Pres. Param.     */

    if (hwnd == NULLHANDLE) {
        TRACE("adjustImage - failed to create window\n") ;
        return ;
    }
    
    WinShowWindow(hwnd, TRUE) ;

    if (hwndShape != NULLHANDLE) {
        WinShowWindow(hwndShape, FALSE) ;
        WinDestroyWindow(hwndShape) ;
    }
    hwndShape = hwnd ;

    if (hbmMem != NULLHANDLE) {
        GpiDeleteBitmap(hbmMem) ;
    }
    hbmMem = hbm ;
}

/*
 * create/disposeWindow - create/dispose Window and related resources
 */

static  PFNWP   pfnFrame ;
static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;

static  void    createWindow(HAB hab, PSWP swp)
{
    FRAMECDATA  fcd ;

    TRACE("createWindow\n") ;
    
    if (hab == NULLHANDLE || swp == NULL) {
        TRACE("createWindow - bad parameters\n") ;
        return ;
    }
    
    memset(&fcd, 0, sizeof(fcd)) ;
    fcd.cb = sizeof(fcd) ;
    fcd.flCreateFlags = (FCF_TASKLIST | FCF_ICON) ;
    fcd.hmodResources = NULLHANDLE ;
    fcd.idResources   = ID_TREYES  ;   

    hwndFrame = WinCreateWindow(
            HWND_DESKTOP,           /* Parent window handle     */
            WC_FRAME,               /* Frame Window Class       */
            ProgramName,            /* as Title                 */
            0,                      /* Window Style             */
            swp->x, swp->y,
	    swp->cx, swp->cy,
            NULLHANDLE,             /* Owner Window             */
            HWND_TOP,               /* Z-Order                  */
            0,                      /* Window ID                */
            &fcd,                   /* Control Data             */
            NULL) ;                 /* Presentation Parameter   */

    if (hwndFrame == NULLHANDLE) {
        TRACE("createWindow - failed to create frmae window\n") ;
        return ;
    }

    pfnFrame = WinSubclassWindow(hwndFrame, procFrame) ;

    WinSendMsg(hwndFrame, WM_SETICON, 
        MPFROMP(WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_TREYES)), NULL) ;

    WinRegisterClass(hab, "ShapeWin", ShapeWinProc, 0L, sizeof(PVOID)) ;

    /*
     * adjust Bitmap/Window size to frame size
     */
     
    adjustImage(swp) ;
}

static  void    disposeWindow(void)
{
    TRACE("disposeWindow\n") ;
    
    if (hwndShape != NULLHANDLE) {
        WinDestroyWindow(hwndShape) ;
    }
    if (hwndFrame != NULLHANDLE) {
        WinDestroyWindow(hwndFrame) ;
    }
    hwndShape = NULLHANDLE ;
    hwndFrame = NULLHANDLE ;
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
        TRACE("failed to load opup menu\n") ;
        return ;
    }
    
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;

    opts = PU_POSITIONONITEM | PU_HCONSTRAIN | PU_VCONSTRAIN |
                 PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 ;

    WinPopupMenu(HWND_DESKTOP, hwndFrame, hwndPopup, 
                                pt.x, pt.y, IDM_MOVE, opts) ;
}
 
/*
 * procFrame - sub-classed frame window procedure for eye
 *
 *      bitmap window send messages ti its owner (frame).  process those
 *      messages to perform transparent background
 */

static  BOOL    fsResize = FALSE ;
static  POINTL  ptResize ;

static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    SWP     swp  ;
    PSWP    pswp ;
    SHORT   x, y ;
    USHORT  fs   ;
    POINTL  pt   ;
    
    TRACE("frame %08x %08x %08x\n", msg, mp1, mp2) ;
    
    switch (msg) {
        
    case WM_ADJUSTWINDOWPOS :
        pswp = (PSWP) PVOIDFROMMP(mp1) ;
        if (pswp->fl & SWP_SIZE) {
            adjustImage(pswp) ;
        }
	WinSetWindowPos(hwndShape, pswp->hwndInsertBehind,
        	    pswp->x, pswp->y, pswp->cx, pswp->cy, pswp->fl) ;
	pswp->fl &= ~SWP_SHOW ;
	pswp->fl |=  SWP_HIDE ;
	return (*pfnFrame) (hwnd, msg, mp1, mp2) ;
	
    case WM_BEGINDRAG :
        fs = TF_MOVE | TF_SETPOINTERPOS ;
        WinSendMsg(hwndFrame, WM_TRACKFRAME, MPFROMSHORT(fs), NULL) ;
        return (MRESULT) 0 ;

    case WM_CONTEXTMENU :
        TRACE("WM_CONEXTMENU\n") ;
	contextMenu() ;
        return (MRESULT) 0 ;
	
    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
        case IDM_MOVE :
	    fs = TF_MOVE | TF_SETPOINTERPOS ;
            WinSendMsg(hwndFrame, WM_TRACKFRAME, MPFROMSHORT(fs), NULL) ;
	    return (MRESULT) 0 ;
        case IDM_SIZE :
	    fsResize = TRUE ;
	    WinSetCapture(HWND_DESKTOP, hwnd) ;
	    WinQueryPointerPos(HWND_DESKTOP, &ptResize) ;
	    return (MRESULT) 0 ;
	case IDM_EXIT :
	    WinShowWindow(hwnd, FALSE) ;
	    WinPostMsg(hwnd, WM_CLOSE, NULL, NULL) ;
	    return (MRESULT) 0 ;
        }
        return (MRESULT) 0 ;

    case WM_MOUSEMOVE  :
        if (fsResize) {
	    WinQueryPointerPos(HWND_DESKTOP, &pt)   ;
	    if (pt.x != ptResize.x || pt.y != ptResize.y) {
                WinSetCapture(HWND_DESKTOP, NULLHANDLE) ;
	        fsResize = FALSE ;
	        fs = TF_SETPOINTERPOS ;
                if (pt.x < ptResize.x) {
		    fs |= TF_LEFT ;
		} else if (pt.x > ptResize.x) {
		    fs |= TF_RIGHT ;
		}
		if (pt.y < ptResize.y) {
		    fs |= TF_BOTTOM ;
		} else if (pt.y > ptResize.y) {
		    fs |= TF_TOP ;
		}
                WinSendMsg(hwndFrame, WM_TRACKFRAME, MPFROMSHORT(fs), NULL) ;
		return (MRESULT) 0 ;
	    }
        }
	break ;
	
    case WM_TIMER :
        drawBall(TRUE) ;
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
    SWP     swp  ;

    myname(av[0]) ;

    hab = WinInitialize(0) ;
    hmq = WinCreateMsgQueue(hab, 0) ;

    initPosSize(ac, av, &swp) ;
    
    createMemorySpace(hab)  ;
    createWindow(hab, &swp) ;

    if (hwndFrame == NULLHANDLE) {
        trMessage("failed to create bitmap/windows") ;
        disposeMemorySpace()    ; 
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    
    WinStartTimer(hab, hwndFrame, 1, 200) ;
    WinShowWindow(hwndFrame, TRUE) ;

    while (WinGetMsg(hab, &qmsg, 0, 0, 0)) {
        WinDispatchMsg(hab, &qmsg) ;
    }

    disposeWindow()      ;
    disposeMemorySpace() ;
    
    WinDestroyMsgQueue(hmq) ;
    WinTerminate(hab) ;
    
    return 0 ; 
}
