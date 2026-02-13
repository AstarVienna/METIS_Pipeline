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

#include <cpl_test.h>

#include "../hdrl_cat_conf.h"
#include "../hdrl_cat_table.h"
#include "hdrl_types.h"

#define NTEST 10


/*----------------------------------------------------------------------------*/
/**
 * @brief Fill a propertylist with the values that it needed for wcs
 */
/*----------------------------------------------------------------------------*/
void fill_plist(cpl_propertylist *pl)
{
    cpl_propertylist_update_string(pl, "CTYPE1", "RA---TAN" );
    cpl_propertylist_update_string(pl, "CTYPE2", "DEC--TAN" );
    cpl_propertylist_update_double(pl, "CRVAL1",  30.       );
    cpl_propertylist_update_double(pl, "CRVAL2",  12.       );
    cpl_propertylist_update_double(pl, "CRPIX1",  512.      );
    cpl_propertylist_update_double(pl, "CRPIX2",  512.      );
    cpl_propertylist_update_double(pl, "CD1_1",  -1. / 3600.);
    cpl_propertylist_update_double(pl, "CD1_2",   0.        );
    cpl_propertylist_update_double(pl, "CD2_1",   0.        );
    cpl_propertylist_update_double(pl, "CD2_2",   1. / 3600.);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Create a gaussian image
 *
 * @return cpl_image
 */
/*----------------------------------------------------------------------------*/
cpl_image * create_gauss(double dx, double dy, double fwhm)
{
    /* sample gaussian on large grid and sum it down to requested fwhm */
    cpl_size  factor = 16;
    cpl_size  nx     = fwhm * 20;
    cpl_size  ny     = fwhm * 20;
    cpl_size  nnx    = nx   * factor;
    cpl_size  nny    = ny   * factor;
    double    sigma  = fwhm * factor / (2 * sqrt(2 * log(2)));
    cpl_image *g     = cpl_image_new(nnx, nny, CPL_TYPE_DOUBLE);

    dx *= factor;
    dy *= factor;

    cpl_image_fill_gaussian( g,
    		                 nnx / 2 + dx,
							 nny / 2 + dx,
                             2. * CPL_MATH_PI * sigma * sigma,
							 sigma,
							 sigma);

    cpl_image *r  = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    double    *dg = cpl_image_get_data_double(g);
    double    *dr = cpl_image_get_data_double(r);

    for (cpl_size y = 0; y < ny; y++) {
        for (cpl_size x = 0; x < nx; x++) {
            for (cpl_size i = y * factor; i < y * factor + factor; i++) {
                for (cpl_size j = x * factor; j < x * factor + factor; j++) {
                    dr[y * nx + x] += dg[i * nnx + j];
                }
            }
        }
    }

    for (cpl_size y = 0; y < nny; y++) {
        for (cpl_size x = 0; x < nnx; x++) {
            dr[y / factor * nx + x / factor] += dg[y * nx + x];
        }
    }

    cpl_image_divide_scalar(r, factor * factor);

    /* avoiding a background rms of exactly zero */
    cpl_image * noise_bkg = cpl_image_duplicate(r);
    cpl_image_fill_noise_uniform(noise_bkg, -HDRL_EPS_DATA, +HDRL_EPS_DATA);
    cpl_image_add(r, noise_bkg);
    cpl_image_delete(noise_bkg);
    /* Close up */
    cpl_image_delete(g);

    return r;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Check hdrl_catalogue_conf() in various conditions
 *
 * @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_catalogue_hdrl_catalogue_conf_basic(void)
{
	/* Initialize */
    double xpos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double ypos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double norm[] = {1000., 100., 200., 500., 550., 600., 650., 700., 750.,  800.};

    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *im  = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);

    cpl_image_fill_noise_uniform(bkg, -10.,   10. );
    cpl_image_fill_noise_uniform(cnf,  99.9, 100.1);

    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    double tot[NTEST];
    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_image_fill_gaussian(im, xpos[i], ypos[i], norm[i] * norm2, sigma, sigma);
        tot[i] = cpl_image_get_flux(im);
        cpl_image_add(bkg, im);
    }
    cpl_image_delete(im);

    hdrl_casu_fits *inf    = hdrl_casu_fits_wrap(bkg);
    hdrl_casu_fits *inconf = hdrl_casu_fits_wrap(cnf);

    /* Give it a WCS */
    fill_plist(hdrl_casu_fits_get_ehu(inf));

    /* Run casu catalogue. TODO: Check results */
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_catalogue_conf(inf, inconf, 5, 1.5, 0, 5., 1, 64, 6, 3., 1.,
    		HDRL_SATURATION_INIT,res), CPL_ERROR_NONE);
    cpl_test_nonnull(res->catalogue);
    cpl_image_delete(res->segmentation_map);
    cpl_image_delete(res->background);

    /* Check the results. Start by checking the number of rows and columns. Sort the table by X */
    cpl_table *tab = hdrl_casu_tfits_get_table(res->catalogue);
    cpl_test_nonnull(tab);
    cpl_test_eq(cpl_table_get_ncol(tab), NCOLS);
    cpl_test_eq(cpl_table_get_nrow(tab), NTEST);

    cpl_propertylist *pl = cpl_propertylist_new();
    cpl_propertylist_append_bool(pl, "X_coordinate", 0);
    cpl_table_sort(tab, pl);
    cpl_propertylist_delete(pl);

    /* Test the column content of the table */
    int nl;
    for (cpl_size i = 0; i < NTEST; i++) {

        cpl_test_abs(xpos[i], cpl_table_get_double(tab, "X_coordinate",    i, &nl), 0.2);
        cpl_test_abs(ypos[i], cpl_table_get_double(tab, "Y_coordinate",    i, &nl), 0.2);

        double diff =    fabs(cpl_table_get_double(tab, "Aper_flux_5",     i, &nl) - tot[i])
                      /       cpl_table_get_double(tab, "Aper_flux_5_err", i, &nl);
        cpl_test_lt(diff, 1.6);
    }

    /* Clean up */
    hdrl_casu_fits_delete(inf);
    hdrl_casu_fits_delete(inconf);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_free(res);


    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Check background subtraction of hdrl_catalogue_conf() in various conditions
 *
 * @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_catalogue_hdrl_catalogue_conf_backsub(void)
{
    cpl_image *cnf = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image_add_scalar(cnf, 100.);

    cpl_image *im  = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image_fill_noise_uniform(im, -10.,   10. );

    /*Set a region arround the object to zero*/
    for (cpl_size x = 400; x < 600; x++) {
        for (cpl_size y = 400; y < 600; y++) {
            cpl_image_set(im, x, y, 0.);
        }
    }

    /* Insert an tophat object */
    for (cpl_size x = 500; x < 505; x++) {
        for (cpl_size y = 500; y < 505; y++) {
            cpl_image_set(im, x, y, 4990.);
        }
    }

    cpl_image_add_scalar(im, 10.);
    hdrl_casu_fits *inf    = hdrl_casu_fits_wrap( im);
    hdrl_casu_fits *inconf = hdrl_casu_fits_wrap(cnf);

    /* Run casu catalogue */
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_catalogue_conf(inf, inconf, 5, 3., 0, 1., 0, 16, 7, 3., 1.,
    		HDRL_SATURATION_INIT,res), CPL_ERROR_NONE);
    cpl_table *tab = hdrl_casu_tfits_get_table(res->catalogue);
    cpl_test_nonnull(tab);

    /* Check the results. */
    cpl_test_eq(cpl_table_get_nrow(tab), 1);

    /* Test the column content of the table */
    int nl;
    cpl_test_abs(5000., cpl_table_get_double(tab, "Aper_flux_1", 0, &nl), 0.2);

    /* Clean up */
    hdrl_casu_fits_delete(inf);
    hdrl_casu_fits_delete(inconf);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);
    cpl_free(res);


    return cpl_error_get_code();
}

cpl_error_code test_gaussians(double dx, double dy, double fwhm)
{
    /* Generate a field with some stars and a confidence map */
    cpl_image *im  = create_gauss(dx, dy, fwhm);
    hdrl_casu_fits *inf = hdrl_casu_fits_wrap(im);

    /* Give it a WCS */
    fill_plist(hdrl_casu_fits_get_ehu(inf));

    /* Run casu catalogue. TODO: Check results */
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_catalogue_conf(inf, NULL, 5, 2.5, 0, fwhm, 1, fwhm * 3, 6, 3., 1.,
    		HDRL_SATURATION_INIT, res), CPL_ERROR_NONE);
    cpl_test_nonnull(res->catalogue);
    cpl_image_delete(res->segmentation_map);
    cpl_image_delete(res->background);

    /* Check the results. Start by checking the number of rows and columns. Sort the table by X */
    cpl_table *tab = hdrl_casu_tfits_get_table(res->catalogue);
    cpl_test_nonnull(tab);
    cpl_test_eq(cpl_table_get_ncol(tab), NCOLS);
    cpl_test_eq(cpl_table_get_nrow(tab), 1    );

    /* Test the column content of the table */
    int nl;
    cpl_test_abs(fwhm, cpl_table_get_double(tab, "FWHM", 0, &nl), 0.006);

    /* Clean up */
    hdrl_casu_fits_delete(inf);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_free(res);


    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Unit tests of hdrl_catalogue_conf in the catalogue module
 */
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_catalogue_hdrl_catalogue_conf_basic();
    hdrl_catalogue_hdrl_catalogue_conf_backsub();

    test_gaussians(.5, .5, 3.);
    test_gaussians(.0, .5, 4.);
    test_gaussians(.5, .5, 5.);
    test_gaussians(.8, .5, 6.);
    test_gaussians(.1, .2, 7.);

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
