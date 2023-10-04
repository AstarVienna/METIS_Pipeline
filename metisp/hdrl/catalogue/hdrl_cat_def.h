/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HDRL_CATALOGUE_DEF_H
#define HDRL_CATALOGUE_DEF_H


#include <cpl.h>
#include <math.h>


/*** DEFINES ***/

/* must not be followed by a semicolon! gcc on mac has omp
 * but it doesn't work for nontrivial cases as libc lacks allocate */
#if defined (_OPENMP) && !defined( __APPLE__)
#define HDRL_OMP(x) _Pragma (#x)
#else
#define HDRL_OMP(x)
#endif

/* Catalogue generation parameters */
#define IMNUM           200    /* Maximum number of images to be deblended */
#define NPAR            16     /* Number of parameters in a basic results array */
#define NRADS           13     /* Number of RANDS  (RADII)  */
#define NAREAL          8      /* Number of areals profiles */

/* MFLAG values used for tracking the quality of individual pixels */
#define MF_CLEANPIX     0      /*  */
#define MF_OBJPIX       1      /*  */
#define MF_SATURATED    2      /*  */
#define MF_ZEROCONF     3      /*  */
#define MF_STUPID_VALUE 4      /*  */
#define MF_POSSIBLEOBJ  5      /*  */


/*** DATA TYPES ***/

typedef struct {
    cpl_size      x;              /* */
    cpl_size      y;              /* */
    double        z;              /* */
    double        zsm;            /* */
    cpl_size      iobj;           /* */
} plstruct;

typedef struct {

    cpl_size      areal[NAREAL]; /* height above thresh of areal-prof cuts   */

    cpl_size      lsiz;          /* size of a line                           */
    cpl_size      csiz;          /* size of a column                         */
    cpl_size      maxip;         /* max no. of parents ever used.            */
    cpl_size      maxbl;         /* size of pixel-storage block stack        */
    cpl_size      maxpa;         /* size of parent-stack.                    */
    cpl_size      ipnop;         /* parent-number-of-pixels, min size of image */
    cpl_size      nimages;       /* count of images                          */
    cpl_size      ipstack;       /* parent-name stack pointer                */
    cpl_size      ibstack;       /* pixel-block name stack pointer           */
    double        thresh;        /* threshold for image detection            */
    double        background;    /* background value                         */
    double        sigma;         /* median background sigma                  */
    cpl_size      multiply;      /* smoothing multiplication                 */
    double        xintmin;       /* minimum intensity for consideration      */
    cpl_size      mulpix;        /* minimum size for considering multiple images */
    double        areal_offset;  /* offset in areal profile levels           */
    double        fconst;        /* Normalisation constant for areal profiles */
    double        saturation;    /* saturation level from background analysis */
    cpl_size      icrowd;        /* true if deblending routine is to be used */

    cpl_size      *blink;        /* block-link array                         */
    cpl_size      *bstack;       /* stack of pixel names                     */

    struct {                     /* Image control block array                */
        cpl_size  first;         /* link to first data block                 */
        cpl_size  last;          /* current last block                       */
        cpl_size  pnop;          /* Parent no. pixels (-1 = inactive)        */
        cpl_size  growing;
        cpl_size  touch;         /* 0 = does not touch an edge               */
        cpl_size  pnbp;          /* Parent no of bad pixels                  */
    } *parent;

    cpl_size      *pstack;       /* stack of parent names                    */
    plstruct      *plessey;      /* x,y,i storage array                      */
    cpl_size      *lastline;     /* Parents on last line                     */

    cpl_image     *inframe;      /* Pointer to original image data frame     */
    cpl_image     *conframe;     /* Pointer to original confidence map frame */
    double        *indata;       /* Pointer to original image data           */
    double        *confdata;     /* Pointer to original confidence map data  */
    unsigned char *mflag;        /* Pointer to mflag array for tracking merges */
    cpl_mask      *opmask;       /* Object pixel mask                        */
    double        rcore;         /* Core radius for aperture photometry      */
    double        filtfwhm;      /* FWHM of smoothing kernel in detection algorithm */
    plstruct      *plarray;      /* Plessey structure workspace for passing data to various processing routines */
    cpl_size      npl;           /* Size of the above                        */
    cpl_size      npl_pix;       /* Number of pixels in the above structure  */
    double        fwhm;          /* Value of the seeing                      */

    struct {
        cpl_size  nbx;           /* X dimension of background map            */
        cpl_size  nby;           /* Y dimension of background map            */
        cpl_size  nbsize;        /* Size of a side of background map cell    */
        double    **bvals;       /* Pointer to background map                */
    } backmap;
} ap_t;

typedef struct {
    double        x;             /* x position                               */
    double        y;             /* y position                               */
    double        total;         /* total integrated intensity               */
    cpl_size      area;          /* image area in pixels                     */
    double        peak;          /* peak image intensity above sky           */
    double        xx;            /* 2nd moment x                             */
    double        xy;            /* 2nd moment cross term                    */
    double        yy;            /* 2nd moment y                             */
    double        ecc;           /* Eccentricity                             */
    cpl_size      areal[NAREAL]; /* areal profile of image                   */
} apmCat_t;


/* cpl_image functions */
typedef struct {
    cpl_image        *image;             /*  */
    cpl_propertylist *ehu;               /*  */
} hdrl_casu_fits;

/* cpl_table functions */
typedef struct {
    cpl_table        *table;             /*  */
    cpl_propertylist *ehu;               /*  */
} hdrl_casu_tfits;


#define HDRL_SATURATION_INIT INFINITY

typedef enum {
    HDRL_CATALOGUE_BKG          = 1,
    HDRL_CATALOGUE_SEGMAP       = 2,
    HDRL_CATALOGUE_CAT_COMPLETE = 4,
    HDRL_CATALOGUE_ALL          = 7
} hdrl_catalogue_options;

/* Catalogue structure */
typedef struct {
    hdrl_casu_tfits  *catalogue;         /* cpl_table and property list */
    cpl_image        *segmentation_map;  /*  */
    cpl_image        *background;        /*  */
} hdrl_casu_result;


#endif /* HDRL_CATALOGUE_DEF_H */
