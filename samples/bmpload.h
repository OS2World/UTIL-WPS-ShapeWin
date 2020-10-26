/*
 * bmpload.h - load bitmap from File
 */

#ifndef _BMPLOAD_H
#define _BMPLOAD_H

/*
 * load bitmap from file
 */
 
extern  BOOL    debugBmpLoadFile ;

HBITMAP     bitmapLoadFile(HAB hab, HPS hps, PUCHAR fname) ;

#endif  /* _BMPLOAD_H */
