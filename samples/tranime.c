/*
 * tranime.c - animate bitmaps with transparent background
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include <shapewin.h>
#include "bmpload.h"

#include "tranime.h"
#include "tranires.h"

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

void    taMessage(PSZ msg)
{
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, msg, ProgramName, 0, MB_OK) ;
}

/*
 * Control Dialog Position
 */

void    dialogAtCenter(HWND hwndDialog)
{
    SWP     posScr ;
    SWP     posDlg ;

    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog,   &posDlg) ;

    posDlg.x = (posScr.cx - posDlg.cx) / 2 ;
    posDlg.y = (posScr.cy - posDlg.cy) / 2 ;

#if 0
    printf("Disktop size %d x %d\n", posScr.cx, posScr.cy) ;
    printf("Dialog  size %d x %d\n", posDlg.cx, posDlg.cy) ;
    printf("Place dialog at ( %d, %d)\n", posDlg.x, posDlg.y) ;
    fflush(stdout) ;
#endif    

    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

void    dialogAtMouse(HWND hwndDialog)
{
    POINTL  pt ;
    SWP     posDlg ;
    SWP     posScr ;

    WinQueryPointerPos(HWND_DESKTOP, &pt)    ;
    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog,   &posDlg) ;

    posDlg.x = pt.x - (posDlg.cx / 10)    ;
    posDlg.y = pt.y - (posDlg.cy * 4 / 5) ;

    if (posDlg.x < 0) {
        posDlg.x = 0 ;
    }
    if (posDlg.y < 0) {
        posDlg.y = 0 ;
    }
    if ((posDlg.x + posDlg.cx) > posScr.cx) {
        posDlg.x = posScr.cx - posDlg.cx ;
    }
    if ((posDlg.y + posDlg.cy) > posScr.cy) {
        posDlg.y = posScr.cy - posDlg.cy ;
    }
    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

/*
 * bitmaps to use
 */

typedef struct _ANIM {
    HDC     hdc  ;
    HPS     hps  ;
    HBITMAP hbm  ;
    SHORT   cx   ;
    SHORT   cy   ;
    HWND    hwnd ;
} ANIREC, *ANIPTR ;

ANIREC  tabAnim[32] = { 0 } ;
int     numAnim = 0 ;
int     stepAnim  = 20  ;
int     delayAnim = 500 ;

#define YLOC    100

static  BOOL    loadOne(HAB hab, PUCHAR name, ANIPTR p)
{
    SIZEL   siz ;
    BITMAPINFOHEADER2   bmi ;
    
    p->hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    p->hps = GpiCreatePS(hab, p->hdc, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;
    p->hbm = bitmapLoadFile(hab, p->hps, name) ;
    
    if (p->hdc == NULLHANDLE || p->hps == NULLHANDLE ||p->hbm == NULLHANDLE) {
        if (p->hbm != NULLHANDLE) GpiDeleteBitmap(p->hbm) ;
	if (p->hps != NULLHANDLE) GpiDestroyPS(p->hps) ;
	if (p->hdc != NULLHANDLE) DevCloseDC(p->hdc) ;
	return FALSE ;
    }

    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(p->hbm, &bmi) ;

    p->cx = bmi.cx ;
    p->cy = bmi.cy ;
    
    return TRUE ;
}

void    loadBitmaps(HAB hab, int ac, char *av[])
{
    int     i ;
    
    for (i = 1 ; i < ac && numAnim < 32 ; i++) {
        if (av[i][0] != '-') {
            if (loadOne(hab, av[i], &tabAnim[numAnim]) == TRUE) {
                numAnim += 1 ;
	    }
        } else {
	    if (av[i][1] == 'd' || av[i][1] == 'D') {
	        if (av[i][2] != '\0') {
		    delayAnim = atoi(&av[i][2]) ;
		} else if ((i + 1) < ac) {
		    delayAnim = atoi(av[i+=1]) ;
		}
	    }
	    if (av[i][1] == 's' || av[i][1] == 'S') {
	        if (av[i][2] != '\0') {
		    stepAnim = atoi(&av[i][2]) ;
		} else if ((i + 1) < ac) {
		    stepAnim = atoi(av[i+=1]) ;
		}
	    }
        }
    }
}

void    freeBitmaps(void)
{
    int     i ;
    
    for (i = 0 ; i < numAnim ; i++) {
        if (tabAnim[i].hwnd != NULLHANDLE) {
	    WinDestroyWindow(tabAnim[i].hwnd) ;
	}
        GpiDeleteBitmap(tabAnim[i].hbm) ;
	GpiDestroyPS(tabAnim[i].hps) ;
	DevCloseDC(tabAnim[i].hdc) ;
    }
}

/*
 * dialog procedure for animation control
 */

static  SWP     swpScreen ;
static  int     curBitmap = 0 ;
static  int     nxtBitmap = 0 ;
static  int     curLocate = 0 ;
static  int     nxtLocate = 0 ;

MRESULT EXPENTRY    procControl(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    int         i       ;
    ANIPTR      pAnim   ;
    SHAPEWIN    shpctrl ;
    PSWP        pswp    ;
    UCHAR   buff[128] ;    

    switch(msg) {
    
    case WM_INITDLG :
        dialogAtCenter(hwnd) ;
	WinSendMsg(hwnd, WM_SETICON, 
	    MPFROMP(WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_TRANIME)), NULL) ;
        WinQueryWindowPos(HWND_DESKTOP, &swpScreen) ;
	
	sprintf(buff, "%d", stepAnim) ;
        WinSetWindowText(WinWindowFromID(hwnd, IDC_STEP), buff) ;
	sprintf(buff, "%d", delayAnim) ;
        WinSetWindowText(WinWindowFromID(hwnd, IDC_DELAY), buff) ;
	
        for (i = 0, pAnim = tabAnim ; i < numAnim ; i++, pAnim++) {

	    shpctrl.cx = pAnim->cx ;
	    shpctrl.cy = pAnim->cy ;
	    shpctrl.hpsDraw = pAnim->hps ;
	    shpctrl.hpsMask = pAnim->hps ;
	    
	    pAnim->hwnd = WinCreateWindow(
                HWND_DESKTOP,           /* Parent Window    */
                ShapeWinName,           /* Window Class     */
                NULL,                   /* Window Text      */
	        0,                      /* Window Style     */
	        0, 0, 0, 0,             /* Pos & Size       */
	        hwnd,                   /* Owner Window     */
	        HWND_TOP,               /* Z-Order          */
	        0,                      /* Window ID        */
	        &shpctrl,               /* Control Data     */
	        NULL) ;                 /* Pres. Param.     */
	}
        return (MRESULT) 0 ;

    case WM_CLOSE :
        WinDismissDlg(hwnd, TRUE) ;
	return (MRESULT) 0 ;

    case WM_DESTROY :
        return (MRESULT) 0 ;
        
    case WM_WINDOWPOSCHANGED :
        pswp = (PSWP) PVOIDFROMMP(mp1) ;
	if (pswp->fl & SWP_ZORDER) {
            for (i = 0, pAnim = tabAnim ; i < numAnim ; i++, pAnim++) {
        	WinSetWindowPos(pAnim->hwnd, 
		    pswp->hwndInsertBehind, 0, 0, 0, 0, SWP_ZORDER) ;
            }
	}
        return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
    
    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
	case IDC_EXIT :
	    WinPostMsg(hwnd, WM_CLOSE, NULL, NULL) ;
	    return (MRESULT) 0 ;

	case IDC_RUN :
            WinQueryWindowText(WinWindowFromID(hwnd, IDC_STEP), 64, buff) ;
	    stepAnim  = atoi(buff) ;
            WinQueryWindowText(WinWindowFromID(hwnd, IDC_DELAY), 64, buff) ;
	    delayAnim = atoi(buff) ;

            if (delayAnim == 0) {
	        taMessage("no delay specified") ;
		return (MRESULT) 0 ;
	    }

	    pAnim = &tabAnim[curBitmap] ;
	    WinSetWindowPos(pAnim->hwnd, NULLHANDLE, 
	            curLocate, YLOC, 0, 0, (SWP_MOVE | SWP_SHOW)) ;
            WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, delayAnim) ;
	    return (MRESULT) 0 ;

        case IDC_STOP :
	    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1) ;
	    pAnim = &tabAnim[curBitmap] ;
            WinShowWindow(pAnim->hwnd, FALSE) ;
	    return (MRESULT) 0 ;
	}
	return (MRESULT) 0 ;

    case WM_TIMER :
        if ((curBitmap + 1) < numAnim) {
	    nxtBitmap = curBitmap + 1 ;
	} else {
	    nxtBitmap = 0 ;
	}
	if ((curLocate + stepAnim) >= swpScreen.cx) {
	    nxtLocate = 0 ;
	} else if ((curLocate + stepAnim) < 0) {
	    nxtLocate = swpScreen.cx ;
	} else {
	    nxtLocate = curLocate + stepAnim ;
	}
        pAnim = &tabAnim[curBitmap] ;
	WinShowWindow(pAnim->hwnd, FALSE) ;
        pAnim = &tabAnim[nxtBitmap] ;
	WinSetWindowPos(pAnim->hwnd, NULLHANDLE,
	            nxtLocate, YLOC, 0, 0, (SWP_MOVE | SWP_SHOW)) ;
	curLocate = nxtLocate ;
	curBitmap = nxtBitmap ;
        return (MRESULT) 0 ;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
}

/*
 * main - program start here
 */

int     main(int ac, char *av[])
{
    HAB     hab ;
    HMQ     hmq ;
    SIZEL   siz ;

    myname(av[0]) ;
    _wildcard(&ac, &av) ;
    
    hab = WinInitialize(0) ;
    hmq = WinCreateMsgQueue(hab, 0) ;

    WinRegisterClass(hab, ShapeWinName, ShapeWinProc, 0L, sizeof(PVOID)) ;

    loadBitmaps(hab, ac, av) ;
    
    if (numAnim == 0) {
        taMessage("no animating bitmaps") ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }

    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, 
                procControl, NULLHANDLE, ID_CONTROL, NULL) ;
        
    freeBitmaps() ;
    
    WinDestroyMsgQueue(hmq) ;
    WinTerminate(hab) ;
    return 0 ;
}
