/*
 * bmpload.c - bitmap library, load bitmap from file
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INCL_PM
#include <os2.h>

#include "bmpload.h"

BOOL    debugBmpLoadFile = FALSE ;

/*
 * loadData, freeData - load/dispose bitmap data from file
 */

static  PUCHAR  loadData(PUCHAR fname)
{
    struct stat st  ;
    PUCHAR      p   ;
    FILE        *fp ;
    PUCHAR      fn = "loadData" ;

    /*
     * query file size and prepare buffer
     */

    if (stat(fname, &st) != 0) {
        if (debugBmpLoadFile) {
            fprintf(stderr, "%s - failed to stat %s\n", fn, fname) ;
        }
        return NULL ;
    }

    if (debugBmpLoadFile) {
        fprintf(stderr, "%s - %s %d bytes\n", fn, fname, st.st_size) ;
    }

    if ((p = malloc(st.st_size)) == NULL) {
        if (debugBmpLoadFile) {
	    fprintf(stderr, "%s - failed to alloc %d\n", fn, st.st_size) ;
        }
        return NULL ;
    }

    /*
     * open file and read into buffer
     */

    if ((fp = fopen(fname, "rb")) == NULL) {
        if (debugBmpLoadFile) {
            fprintf(stderr, "%s - failed to open %s\n", fn, fname) ;
        }
        free(p) ;
	return NULL ;
    }
    fread(p, 1, st.st_size, fp) ;
    fclose(fp) ;

    /*
     * check if file was really bitmap file
     */

    if (p[0] != 'B' || p[1] != 'M') {
        if (debugBmpLoadFile) {
	    fprintf(stderr, "%s - %s is not bitmap file\n", fname) ;
        }
	free(p) ;
	return NULL ;
    }
    return p ;
}

static  void    freeData(PUCHAR data)
{
    if (data) {
        free(data) ;
    }
}

/*
 * bitmapLoadFile - load bitmap from file
 */

HBITMAP bitmapLoadFile(HAB hab, HPS hps, PUCHAR fname)
{
    PUCHAR  pbm ;
    HBITMAP hbm ;
    PBITMAPFILEHEADER2  pbfh ;
    PUCHAR  fn = "bitmapLoadFile" ;

    /*
     * load bitmap data from file
     */

    if ((pbm = loadData(fname)) == NULL) {
        if (debugBmpLoadFile) {
            fprintf(stderr, "%s - failed to load %s\n", fn, fname) ;
        }
        return NULLHANDLE ;
    }
    pbfh = (PBITMAPFILEHEADER2) pbm ;

    /*
     * create bitmap from loaded data
     */

    hbm = GpiCreateBitmap(hps, &pbfh->bmp2, CBM_INIT,
                (pbm + pbfh->offBits), (PBITMAPINFO2) &pbfh->bmp2) ;

    if (hbm == NULLHANDLE) {
        if (debugBmpLoadFile) {
	    fprintf(stderr, "%s - failed to create bitmap\n", fn)  ;
        }
        freeData(pbm) ;
	return NULLHANDLE ;
    }
    
    GpiSetBitmap(hps, hbm) ;
    freeData(pbm) ;
    
    return hbm ;
}

