

/*
 * This file is part of the irplib package
 * Copyright (C) 2002, 2003 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 USA
 */

/*
 * $Author: jtaylor $
 * $Id: irplib_hist.c,v 1.8 2013-07-04 12:10:12 jtaylor Exp $
 * $Date: 2013-07-04 12:10:12 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "irplib_hist.h"

struct _irplib_hist_
{
    unsigned long * bins;
    unsigned long   nbins;
    double          start;
    double          range;
};

/*
 * Create a new empty histogram
 */

irplib_hist *
irplib_hist_new(void)
{
    return (irplib_hist *) cpl_calloc(1, sizeof(irplib_hist));
}

/*
 * Delete a histogram
 */

void
irplib_hist_delete(irplib_hist * d)
{
    if (d == NULL)
	return;

    if (d -> bins)
	cpl_free(d -> bins);

    cpl_free(d);
}

/*
 * Initialise a histogram with user-defined values
 */

cpl_error_code
irplib_hist_init(irplib_hist   * hist,
                 unsigned long   nbins,
                 double          start,
                 double          range)
{
    /* Test the entries */
    cpl_ensure_code(hist         != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(nbins        >  0,    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(range        >  0,    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(hist -> bins == NULL, CPL_ERROR_ILLEGAL_INPUT);

    /* Initialise the histogram structure */
    hist -> bins  = (unsigned long *) cpl_calloc(nbins, sizeof(unsigned long));
    hist -> nbins = nbins;
    hist -> start = start;
    hist -> range = range;

    return cpl_error_get_code();
}

/*
 * Return the value of a histogram bin.
 * An uninitialised histogram is considered an illegal input.
 */

unsigned long
irplib_hist_get_value(const irplib_hist * hist,
                      const unsigned long binpos)
{
    cpl_ensure(hist         != NULL,          CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hist -> bins != NULL,          CPL_ERROR_ILLEGAL_INPUT, 0);
    cpl_ensure(binpos       <  hist -> nbins, CPL_ERROR_ILLEGAL_INPUT, 0);

    return hist -> bins[binpos];
}

/*
 * Return the number of bins in the histogram.
 */

unsigned long
irplib_hist_get_nbins(const irplib_hist * hist)
{
    cpl_ensure(hist != NULL, CPL_ERROR_NULL_INPUT, 0);

    return hist -> nbins;
}

/*
 * Return the binwidth of the histogram.
 */

double
irplib_hist_get_bin_size(const irplib_hist * hist)
{
    cpl_ensure(hist         != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hist -> bins != NULL, CPL_ERROR_ILLEGAL_INPUT, 0);

    return hist -> range / (double)(hist -> nbins - 2);
}

/*
 * Return the range covered by the histogram.
 */

double
irplib_hist_get_range(const irplib_hist * hist)
{
    cpl_ensure(hist != NULL, CPL_ERROR_NULL_INPUT, 0);

    return hist -> range;
}

/*
 * Return the real value corresponding
 * to the inferior limit of the histogram..
 */

double
irplib_hist_get_start(const irplib_hist * hist)
{
    cpl_ensure(hist         != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hist -> bins != NULL, CPL_ERROR_ILLEGAL_INPUT, 0);

    return hist -> start;
}

/*
 * Fill a histogram for an image.
 * If the histogram is uninitialised,
 * the function initialises it with default values.
 */

cpl_error_code
irplib_hist_fill(irplib_hist     * hist,
                 const cpl_image * image)
{
    double           binwidth = 1.0;
    int              nsamples;
    int              i;
    const float    * data = 0;
    const cpl_binary*  bpm_data = 0;
    const cpl_mask* bpm = 0;

    /* Test the entries */
    cpl_ensure_code(hist  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(image != NULL, CPL_ERROR_NULL_INPUT);

    if (hist -> bins == NULL) {
	const double        hstart = cpl_image_get_min(image);
	const double        hrange = cpl_image_get_max(image) - hstart;
	cpl_error_code      error;

	/*
         * Whichever function that computes an optimal binwidth
         * should be introduced inside this if-statement, here.
         */

	/* 2 extra-bins for possible out-of-range values */
	const unsigned long nbins  = (unsigned long) (hrange / binwidth) + 2;

	error = irplib_hist_init(hist, nbins, hstart, hrange);
	cpl_ensure_code(!error, error);
    } else {
	cpl_ensure_code(hist -> range > 0, CPL_ERROR_ILLEGAL_INPUT);

	/* 2 bins reserved for possible out-of-range values */
        binwidth = hist -> range / (double)(hist -> nbins - 2);
    }

    nsamples = cpl_image_get_size_x(image) * cpl_image_get_size_y(image);
    data     = cpl_image_get_data_float_const(image);
    bpm 	 = cpl_image_get_bpm_const(image);
    if (bpm)
    {
    	bpm_data 	 = cpl_mask_get_data_const(bpm); // bad pixel mask
    }

    for (i = 0; i < nsamples; i++)
    {
    	int pos = 0;
        if(bpm_data && bpm_data[i] != CPL_BINARY_0)
    	{
    		continue;
    	}
		pos = (int)((data[i] - hist -> start) / binwidth);
		if (pos <  0)
		{
			hist -> bins[0]++;
		} else if ((unsigned long) pos >= (hist -> nbins - 2))
		{
			hist -> bins[hist -> nbins - 1]++;
		} else
		{
			hist -> bins[pos + 1]++;
		}
    }

    return cpl_error_get_code();
}

/*
 * Compute the maximum of a histogram.
 * Return: the maximum value.
 * The parameter max_where is a pointer to the position
 * of the maximum in the histogram.
 */

unsigned long
irplib_hist_get_max(const irplib_hist * hist,
                    unsigned long     * maxpos)
{
    unsigned long max = 0;
    unsigned long ui;

    cpl_ensure(hist         != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(maxpos       != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hist -> bins != NULL, CPL_ERROR_ILLEGAL_INPUT, 0);

    for(ui = 0; ui < hist -> nbins; ui++) {
	double c_value = irplib_hist_get_value(hist, ui);
	if(c_value > max) {
	    max     = c_value;
	    *maxpos = ui;
	}
    }

    return max;
}

/*
 * Cast a histogram into a table with a single column named "HIST"
 */

cpl_table *
irplib_hist_cast_table(const irplib_hist * hist)
{
    cpl_table      * table;
    cpl_error_code   error;

    cpl_ensure(hist         != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(hist -> bins != NULL, CPL_ERROR_ILLEGAL_INPUT, NULL);

    table = cpl_table_new(hist -> nbins);

    error = cpl_table_new_column(table, "HIST", CPL_TYPE_INT);
    cpl_ensure(!error, error, NULL);

    error = cpl_table_copy_data_int(table, "HIST", (int *)(hist -> bins));
    cpl_ensure(!error, error, NULL);

    return table;
}

/*
 * Collapse the histogram: add the values of all bins.
 * Used now only for debugging purposes.
 */

cpl_error_code
irplib_hist_collapse(irplib_hist * hist,
                     unsigned long new_nbins)
{
    unsigned long   ui, nuj;
    unsigned long * old_bins;
    unsigned long   old_nbins;
    double          collapse_rate;
    cpl_error_code  error;
    unsigned long   rest;

    cpl_ensure_code(hist         != NULL,          CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(hist -> bins != NULL,          CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(new_nbins    >  0,             CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(new_nbins    <= hist -> nbins, CPL_ERROR_ILLEGAL_INPUT);

    old_bins  = hist -> bins;
    old_nbins = hist -> nbins;

    hist -> bins = NULL;
    error = irplib_hist_init(hist, new_nbins, hist -> start, hist -> range);
    cpl_ensure_code(!error, error);

    collapse_rate = (double) (old_nbins - 2) / (double) (new_nbins - 2);

    /* The out-of-range values are not affected by the collapsing operation */
    hist -> bins[0]             = old_bins[0];
    hist -> bins[new_nbins - 1] = old_bins[old_nbins - 1];

    rest = 0;
    nuj  = 1;

    for (ui = 1; ui < new_nbins - 1; ui++) {
	unsigned long uj;
	const double  up  = collapse_rate *  ui;

	hist -> bins[ui] += rest;

	for (uj = nuj; uj < (unsigned long) up + 1; uj++)
	    hist -> bins[ui] +=  old_bins[uj];

	rest = (unsigned long)(up - (unsigned long) up) * old_bins[uj];
	hist -> bins[ui] += rest;

	rest = old_bins[uj] - rest;
	nuj = uj + 1;
    }

    cpl_free(old_bins);

    return cpl_error_get_code();
}
