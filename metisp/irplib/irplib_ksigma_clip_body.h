/* $Id: irplib_ksigma_clip_body.h,v 1.1 2011-11-02 13:18:28 amodigli Exp $
 *
 * This file is part of the irplib package 
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: amodigli $
 * $Date: 2011-11-02 13:18:28 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#define TYPE_ADD(a) CONCAT2X(a, CPL_TYPE)

static cpl_error_code
TYPE_ADD(irplib_ksigma_clip)(const CPL_TYPE  * pi,
			     int               llx,
			     int               lly,
			     int               urx,
			     int               ury,
			     int               nx,
			     double            var_sum,
			     int               npixs,
			     double            kappa,
			     int               nclip,
			     double            tolerance,
			     double          * mean,
			     double          * stdev)
{
    int    pos0 = (llx - 1) + (lly - 1) * nx; /* 1st pixel to process */
    double nb   = (double) npixs;             /* Non-bad pixels in window */

    double lo_cut    = *mean - kappa * (*stdev);
    double hi_cut    = *mean + kappa * (*stdev);
    
    double lo_cut_p  = lo_cut;
    double hi_cut_p  = hi_cut;

    double c_mean = 0;  /* Set to zero in case loop body not executed. */
    double c_stdev = 0; /* Set to zero in case loop body not executed. */

    int    iclip;

    for (iclip = 0; iclip < nclip; iclip++) {
        int pos = pos0;
        int i, j;
        double c_var_sum;

	c_var_sum = var_sum;
	c_mean    = *mean;
	c_stdev   = *stdev;
	nb        = npixs;
	
        for (j = lly - 1; j < ury; j++, pos += (nx - urx + llx - 1)) {
            for (i = llx - 1; i < urx; i++, pos++) {
                if (pi[pos] > hi_cut || pi[pos] < lo_cut) {
                    const double delta = (double)pi[pos] - c_mean;

                    c_var_sum  -= nb * delta * delta / (nb - 1.0);
                    c_mean     -= delta / (nb - 1.0);
                    nb          = nb - 1.0;
                }
            }
        }

	if (nb == 1.0 || c_var_sum < 0.0) {
	    cpl_msg_error(cpl_func, "Iteration %d: Too many pixels were "
			  "removed. This may cause unexpected behaviour. "
			  "Please set a lower number of iterations "
                          "or increase the value of kappa\n", iclip);
	    cpl_error_set(cpl_func, CPL_ERROR_DIVISION_BY_ZERO);
	} else {
	    c_stdev = sqrt(c_var_sum / (nb - 1.0));
	}

	lo_cut = c_mean - kappa * c_stdev;
	hi_cut = c_mean + kappa * c_stdev;
	
        if(fabs(lo_cut - lo_cut_p) < tolerance &&
           fabs(hi_cut - hi_cut_p) < tolerance) {
            break;
	} else {
	    lo_cut_p = lo_cut;
	    hi_cut_p = hi_cut;
	}
    }

    *mean  = c_mean;
    *stdev = c_stdev;

    return cpl_error_get_code();
}
