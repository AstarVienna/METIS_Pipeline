/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
-----------------------------------------------------------------------------*/

#include "hdrl_types.h"
#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include "hdrl_utils.h"
#include "hdrl_fit.h"

#include <cpl.h>
#include <math.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_fit     Fitting
 */
/*----------------------------------------------------------------------------*/

/**@{*/


/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/** @cond PRIVATE */

#if CPL_VERSION_CODE > CPL_VERSION(6, 6, 255)
static cpl_matrix * matrix_product_normal_create(const cpl_matrix * self)
{
    const size_t m       = cpl_matrix_get_nrow(self);
    cpl_matrix * product = cpl_matrix_wrap((cpl_size)m, (cpl_size)m,
                                           cpl_malloc(m * m * sizeof(double)));

    if (cpl_matrix_product_normal(product, self)) {
        cpl_matrix_delete(product);
        product = NULL;
    }

    return product;
}
#else
cpl_matrix * cpl_matrix_product_normal_create(const cpl_matrix * self);
cpl_error_code cpl_matrix_product_transpose(cpl_matrix * self,
                                            const cpl_matrix * ma,
                                            const cpl_matrix * mb);
static cpl_matrix * matrix_product_normal_create(const cpl_matrix * self)
{

    return cpl_matrix_product_normal_create(self);
}
#endif


typedef struct {
    /* input design matrix of fit */
    cpl_matrix * design;
    /* coefficient row matrix */
    cpl_matrix * coef;
    /* covariance matrix of coefficients */
    cpl_matrix * cov;
} hdrl_ls_fit_result;


/* ---------------------------------------------------------------------------*/
/**
 * @brief create least squares fit result structure
 * @return  fit result structure
 */
/* ---------------------------------------------------------------------------*/
static hdrl_ls_fit_result * hdrl_ls_fit_result_create(void)
{
    return cpl_calloc(1, sizeof(hdrl_ls_fit_result));
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete least squares fit result structure
 * @param fit result structure, may be NULL
 */
/* ---------------------------------------------------------------------------*/
static void hdrl_ls_fit_result_delete(hdrl_ls_fit_result * r)
{
    if (r == NULL)
        return;
    cpl_matrix_delete(r->design);
    cpl_matrix_delete(r->coef);
    cpl_matrix_delete(r->cov);
    cpl_free(r);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get fitted values from a least squares fit result
 * @param r fit result structure
 * @return vector containing the fitted values
 */
/* ---------------------------------------------------------------------------*/
static cpl_vector * hdrl_ls_fit_result_get_fitted_values(
      const hdrl_ls_fit_result * r)
{
    cpl_matrix * fvalues = cpl_matrix_product_create(r->design, r->coef);
    cpl_vector * res = cpl_vector_wrap(cpl_matrix_get_nrow(fvalues),
                                       cpl_matrix_get_data(fvalues));
    cpl_matrix_unwrap(fvalues);
    return res;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get fit residuals from a least squares fit result
 * @param r    fit result structure
 * @param data data which from which the fitted values are subtracted
 * @return vector of residuals
 */
/* ---------------------------------------------------------------------------*/
static cpl_vector * hdrl_ls_fit_result_get_residuals(
      const hdrl_ls_fit_result * r,
      const cpl_vector * data)
{
    cpl_vector * fval = hdrl_ls_fit_result_get_fitted_values(r);
    cpl_vector * res = cpl_vector_duplicate(data);
    cpl_vector_subtract(res, fval);
    cpl_vector_delete(fval);
    return res;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief get squared chi of a least squares fit result
 * @param r      fit result structure
 * @param data   data of the fit
 * @param errors errors of the fit
 * @return  squared chi statistic of the fit
 *
 * the chi square is computed as:
 * sum_i(1/sigma_i^2 * residual_i ^ 2)
 */
/* ---------------------------------------------------------------------------*/
static double hdrl_ls_fit_result_get_chi2(
      const hdrl_ls_fit_result * r,
      const cpl_vector * data,
      cpl_vector * errors)
{
    cpl_vector * fval = hdrl_ls_fit_result_get_residuals(r, data);
    /* mse = sum(sqrt(weights) * residuals ** 2) / df */
    cpl_vector_divide(fval, errors);
    cpl_vector_multiply(fval, fval);
    double mswd = cpl_vector_get_sum(fval);

    cpl_vector_delete(fval);
    return mswd;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get degrees of freedom of a least squares fit
 * @param r fit result structure
 * @return degrees of freedom
 */
/* ---------------------------------------------------------------------------*/
static cpl_size hdrl_ls_fit_result_get_residual_dof(const hdrl_ls_fit_result * r)
{
    return cpl_matrix_get_nrow(r->design) - cpl_matrix_get_ncol(r->design);
}

/* ---------------------------------------------------------------------------*/
/**
* @brief generic 1d vandermonde matrix
*
* @param sample  sampling positions
* @param degree  degree of polynomial
* @param func    function evaluating polynomials from [0, degree] at
*                sampling point
* @return matrix containing the vandermonde matrix
*/
/* ---------------------------------------------------------------------------*/
static cpl_matrix * vander1d(
          const cpl_vector * sample,
          cpl_size           degree,
          void (*func)(double, double *, size_t))
{
    const size_t nr = cpl_vector_get_size(sample);
    const size_t nc = degree + 1;
    cpl_matrix * V = cpl_matrix_new(nr, nc);
    double * v = cpl_matrix_get_data(V);
    const double * d = cpl_vector_get_data_const(sample);
    for (size_t i = 0; i < nr; i++) {
        func(d[i], &v[i*nc], nc);
    }

    return V;
}

static void polynomial(double x, double * p, size_t ncoefs)
{
    p[0] = 1.;
    for (size_t i = 1; i < ncoefs; i++) {
        p[i] = pow(x, i);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get vandermonde matrix for a 1d polynomial
 * @param sample  sampling positions
 * @param degree  degree of polynomial
 * @return matrix containing the vandermonde matrix
 */
/* ---------------------------------------------------------------------------*/
static cpl_matrix * polyvander1d(
        const cpl_vector * sample,
        cpl_size           degree)
{
    return vander1d(sample, degree, &polynomial);
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief perform a least squares fit
 * @param design  design matrix
 * @param values  data to fit
 * @param errors  errors of data
 * @return fit result structure
 *         must be deleted with hdrl_ls_fit_result_delete()
 */
/* ---------------------------------------------------------------------------*/
static hdrl_ls_fit_result * fit(
         const cpl_matrix * design,
         const cpl_vector * values,
         const cpl_vector * errors)
{
    hdrl_ls_fit_result * r = hdrl_ls_fit_result_create();
    r->design = cpl_matrix_duplicate(design);
    if (errors) {
        assert(cpl_matrix_get_nrow(design) == cpl_vector_get_size(errors));
        /* weight response and design */
        cpl_vector * vrhs = cpl_vector_duplicate(errors);
        cpl_vector_power(vrhs, -1);
        cpl_matrix * wdesign = cpl_matrix_duplicate(design);
        for (size_t i = 0; i < (size_t)cpl_vector_get_size(errors); i++) {
            double w = cpl_vector_get(vrhs, i);
            for (size_t j = 0; j < (size_t)cpl_matrix_get_ncol(wdesign); j++) {
                cpl_matrix_set(wdesign, i, j,
                               cpl_matrix_get(wdesign, i, j) * w);
            }
        }

        cpl_vector_multiply(vrhs, values);
        cpl_matrix * rhs = cpl_matrix_wrap(cpl_vector_get_size(vrhs), 1,
                                           cpl_vector_get_data(vrhs));

        /* solve Ax = b */
        /* cpl_matrix_solve_normal(design, rhs) + covariance */
        {
            cpl_matrix * At  = cpl_matrix_transpose_create(wdesign);
            cpl_matrix * AtA = matrix_product_normal_create(At);

            /* RRt = AtA */
            cpl_matrix_decomp_chol(AtA);
            /* solve for pseudo inverse: (RRt)P=At*/
            cpl_matrix_solve_chol(AtA, At);
            /* compute solution to system Ax=b -> x=Pb */
            r->coef = cpl_matrix_product_create(At, rhs);
            /* compute covariance matrix cov(b) = PPt */
            r->cov = cpl_matrix_new(cpl_matrix_get_ncol(At),
                                    cpl_matrix_get_ncol(At));
            cpl_matrix_product_transpose(r->cov, At, At);

            cpl_matrix_delete(At);
            cpl_matrix_delete(AtA);
        }

        cpl_matrix_unwrap(rhs);
        cpl_vector_delete(vrhs);
        cpl_matrix_delete(wdesign);
    }
    else {
        cpl_vector * vrhs = cpl_vector_duplicate(values);
        cpl_matrix * rhs = cpl_matrix_wrap(cpl_vector_get_size(vrhs), 1,
                                           cpl_vector_get_data(vrhs));
        r->coef = cpl_matrix_solve_normal(design, rhs);
        cpl_matrix_unwrap(rhs);
        cpl_vector_delete(vrhs);
    }
    return r;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief perform 1d polynomial least squares fit
 *
 * @param sample  sampling points
 * @param values  values to fit
 * @param errors  errors to fit
 * @param degree  degree of polynomial to fit
 * @return fit result structure
 *         must be deleted with hdrl_ls_fit_result_delete()
 * @see fit, polyvander1d
 */
/* ---------------------------------------------------------------------------*/
static hdrl_ls_fit_result * polyfit1d(
         const cpl_vector * sample,
         const cpl_vector * values,
         const cpl_vector * errors,
         int degree)
{
    cpl_matrix * design = polyvander1d(sample, degree);
    hdrl_ls_fit_result * r = fit(design, values, errors);
    cpl_matrix_delete(design);
    return r;
}
/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief weighted least squares polynomial fit of each pixel of a imagelist
 *
 * @param list      imagelist to fit, the 1/errors^2 are used as the weights of
 *                  the fit
 * @param samplepos vector of sample position of each image in the list
 * @param degree    degree of the fit starting from 0
 * @param coef      output coefficient hdrl_imagelist, the data contains the
 *                  coefficient the error contains the diagonal element of the
 *                  covariance matrix
 * @param chi2      output double cpl_image, contains the chi2 of the fit
 * @param dof       output double cpl_image, contains the degrees of freedom of
 *                  the residuals
 *
 *  @note the errors only need to be relative correct, if the are wrong by a
 *        constant the real errors of the data points can be estimated by
 *        multiplying the squared errors with chi2/dof
 *        The fitting method uses normal equation so the function should not be
 *        used for badly conditioned data.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_fit_polynomial_imagelist(const hdrl_imagelist * list,
                              const cpl_vector * samplepos,
                              const int degree,
                              hdrl_imagelist ** coef,
                              cpl_image ** chi2,
                              cpl_image ** dof)
{
    cpl_ensure_code(degree >= 0, CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(list && samplepos && coef, CPL_ERROR_NULL_INPUT);
    // TODO test
    cpl_ensure_code(cpl_vector_get_size(samplepos) ==
                    hdrl_imagelist_get_size(list),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_vector_get_size(samplepos) ==
                    hdrl_imagelist_get_size(list),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_imagelist_get_size(list) > 0,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_imagelist_get_size(list) >= degree + 1,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    intptr_t nx = hdrl_imagelist_get_size_x(list);
    intptr_t ny = hdrl_imagelist_get_size_y(list);
    size_t noz = degree + 1;

    /* make sure image has a mask to avoid creation race later */
    if (coef) {
    	*coef = hdrl_imagelist_new();
    }
    if (chi2) {
        *chi2 = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_get_bpm(*chi2);
    }
    if (dof) {
        *dof = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_get_bpm(*dof);
    }
    for (size_t z = 0; z < noz; z++) {
        hdrl_image * img = hdrl_image_new(nx, ny);
        hdrl_image_get_mask(img);
        hdrl_imagelist_set(*coef, img, z);
    }
    cpl_imagelist * datal, *errorl;
    if (hdrl_imagelist_to_cplwrap(list, &datal, &errorl)) {
        goto fail;
    }
    HDRL_OMP(omp parallel shared(coef, chi2, dof))
    {
        hdrl_vector_cache * cache =
            hdrl_vector_cache_new(cpl_imagelist_get_size(datal), nx * 2);
        /* copy to store bad pixel cleaned positions in */
        cpl_vector * nsamppos = cpl_vector_duplicate(samplepos);
        HDRL_OMP(omp for)
        for (intptr_t y = 0; y < ny; y++) {
                cpl_vector * datav[nx];
                cpl_vector * errsv[nx];
                hdrl_imagelist_to_vector_row(datal, y + 1, datav, cache);
                hdrl_imagelist_to_vector_row(errorl, y + 1, errsv, cache);
                for (intptr_t x = 0; x < nx; x++) {
                    /* all bad or less good than fit degrees */
                    cpl_vector * data = datav[x];
                    cpl_vector * errs = errsv[x];
                    if (data == NULL || (size_t)cpl_vector_get_size(data) < noz) {
                        for (size_t z = 0; z < noz; z++) {
                            hdrl_image * oimg = hdrl_imagelist_get(*coef, z);
                            hdrl_image_set_pixel(oimg, x + 1, y + 1,
                                                 (hdrl_value){NAN, NAN});
                            hdrl_image_reject(oimg, x + 1, y + 1);
                        }
                        if (chi2) {
                            cpl_image_set(*chi2, x + 1, y + 1, NAN);
                            cpl_image_reject(*chi2, x + 1, y + 1);
                        }
                        if (dof) {
                            int n = data ? cpl_vector_get_size(data) - noz : -noz;
                            cpl_image_set(*dof, x + 1, y + 1, n);
                            cpl_image_reject(*dof, x + 1, y + 1);
                        }
                        hdrl_cplvector_delete_to_cache(cache, data);
                        hdrl_cplvector_delete_to_cache(cache, errs);
                        continue;
                    }

                    hdrl_ls_fit_result * r;

                    /* remove bad pixels from sample positions and fit */
                    if (cpl_vector_get_size(data) != cpl_vector_get_size(samplepos)) {
                        size_t j = 0;
                        cpl_vector_set_size(nsamppos, cpl_vector_get_size(data));
                        for (size_t i = 0; i < (size_t)hdrl_imagelist_get_size(list); i++) {
                            hdrl_image * img = hdrl_imagelist_get(list, i);
                            if (hdrl_image_is_rejected(img, x + 1, y + 1))
                                continue;
                            cpl_vector_set(nsamppos, j++, cpl_vector_get(samplepos, i));
                        }

                        r = polyfit1d(nsamppos, data, errs, degree);
                    }
                    else {
                        r = polyfit1d(samplepos, data, errs, degree);
                    }

                    // TODO handle failure
                    for (size_t z = 0; z < noz; z++) {
                        hdrl_image * oimg = hdrl_imagelist_get(*coef, z);
                        hdrl_image_set_pixel(oimg, x + 1, y + 1,
                                             (hdrl_value){cpl_matrix_get(r->coef, z, 0),
                                             sqrt(cpl_matrix_get(r->cov, z, z))});
                    }
                    if (chi2) {
                        cpl_image_set(*chi2, x + 1, y + 1,
                                      hdrl_ls_fit_result_get_chi2(r, data, errs));
                    }
                    if (dof) {
                        cpl_image_set(*dof, x + 1, y + 1,
                                      hdrl_ls_fit_result_get_residual_dof(r));
                    }
                    hdrl_ls_fit_result_delete(r);
                    hdrl_cplvector_delete_to_cache(cache, data);
                    hdrl_cplvector_delete_to_cache(cache, errs);
                }
            }
        hdrl_vector_cache_delete(cache);
        cpl_vector_delete(nsamppos);
    }

    cpl_imagelist_unwrap(datal);
    cpl_imagelist_unwrap(errorl);

    return cpl_error_get_code();
fail:
    hdrl_imagelist_delete(*coef);
    *coef = NULL;
    if (chi2) {
        cpl_image_delete(*chi2);
        *chi2 = NULL;
    }
    if (dof) {
        cpl_image_delete(*dof);
        *dof = NULL;
    }
    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief weighted least squares polynomial fit of each pixel of a imagelist
 *
 * @param list      imagelist to fit, the 1/errors^2 are used as the weights of
 *                  the fit
 * @param samplepos each slice of pixels of this imagelist form the sample
 *                  position vector
 * @param degree    degree of the fit starting from 0
 * @param coef      output coefficient hdrl_imagelist, the data contains the
 *                  coefficient the error contains the diagonal element of the
 *                  covariance matrix
 * @param chi2      output double cpl_image, contains the chi2 of the fit
 * @param dof       output double cpl_image, contains the degrees of freedom of
 *                  the residuals
 * @see hdrl_fit_polynomial_imagelist
 *
 * Similar to hdrl_fit_polynomial_imagelist except the sample positions for
 * each line of pixels per image is taken from the slice of pixels of the
 * samplepos imagelist
 *
 *  @note the errors only need to be relative correct, if the are wrong by a
 *        constant the real errors of the data points can be estimated by
 *        multiplying the squared errors with chi2/dof
 *        The fitting method uses normal equation so the function should not be
 *        used for badly conditioned data.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_fit_polynomial_imagelist2(const hdrl_imagelist * list,
                               const cpl_imagelist * samplepos,
                               const int degree,
                               hdrl_imagelist ** coef,
                               cpl_image ** chi2,
                               cpl_image ** dof)
{
    cpl_ensure_code(degree >= 0, CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(list && samplepos && coef, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(samplepos) ==
                    hdrl_imagelist_get_size(list),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(samplepos) ==
                    hdrl_imagelist_get_size(list),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_imagelist_get_size(list) > 0,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_imagelist_get_size(list) >= degree + 1,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_image_get_size_x(hdrl_imagelist_get_const(list, 0)) ==
                    cpl_image_get_size_x(cpl_imagelist_get_const(samplepos, 0)),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(hdrl_image_get_size_y(hdrl_imagelist_get_const(list, 0)) ==
                    cpl_image_get_size_y(cpl_imagelist_get_const(samplepos, 0)),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    intptr_t nx = hdrl_imagelist_get_size_x(list);
    intptr_t ny = hdrl_imagelist_get_size_y(list);
    size_t noz = degree + 1;

    /* make sure image has a mask to avoid creation race later */
    if(coef) {
    	*coef = hdrl_imagelist_new();
    }
    if (chi2) {
        *chi2 = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_get_bpm(*chi2);
    }
    if (dof) {
        *dof = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_get_bpm(*dof);
    }
    for (size_t z = 0; z < noz; z++) {
        hdrl_image * img = hdrl_image_new(nx, ny);
        hdrl_image_get_mask(img);
        hdrl_imagelist_set(*coef, img, z);
    }
    cpl_imagelist * datal, *errorl;
    if (hdrl_imagelist_to_cplwrap(list, &datal, &errorl)) {
        goto fail;
    }
    HDRL_OMP(omp parallel shared(coef, chi2, dof))
    {
        hdrl_vector_cache * cache =
            hdrl_vector_cache_new(cpl_imagelist_get_size(datal), nx * 3);
        HDRL_OMP(omp for)
        for (intptr_t y = 0; y < ny; y++) {
            cpl_vector * datav[nx];
            cpl_vector * errsv[nx];
            cpl_vector * samplev[nx];
            hdrl_imagelist_to_vector_row(datal, y + 1, datav, cache);
            hdrl_imagelist_to_vector_row(errorl, y + 1, errsv, cache);
            hdrl_imagelist_to_vector_row(samplepos, y + 1, samplev, cache);
            for (intptr_t x = 0; x < nx; x++) {
                cpl_vector * data = datav[x];
                cpl_vector * errs = errsv[x];
                cpl_vector * samp = samplev[x];
                /* all bad or less good than fit degrees */
                if (data == NULL || samp == NULL ||
                    (size_t)cpl_vector_get_size(data) < noz ||
                    (size_t)cpl_vector_get_size(samp) < noz) {
                    for (size_t z = 0; z < noz; z++) {
                        hdrl_image * oimg = hdrl_imagelist_get(*coef, z);
                        hdrl_image_set_pixel(oimg, x + 1, y + 1,
                                             (hdrl_value){NAN, NAN});
                        hdrl_image_reject(oimg, x + 1, y + 1);
                    }
                    if (chi2) {
                        cpl_image_set(*chi2, x + 1, y + 1, NAN);
                        cpl_image_reject(*chi2, x + 1, y + 1);
                    }
                    if (dof) {
                        int n = data ? cpl_vector_get_size(data) - noz : -noz;
                        cpl_image_set(*dof, x + 1, y + 1, n);
                        cpl_image_reject(*dof, x + 1, y + 1);
                    }
                    hdrl_cplvector_delete_to_cache(cache, data);
                    hdrl_cplvector_delete_to_cache(cache, errs);
                    hdrl_cplvector_delete_to_cache(cache, samp);
                    continue;
                }

                /* remove bad pixels from vectors */
                if (cpl_vector_get_size(data) != hdrl_imagelist_get_size(list) ||
                    cpl_vector_get_size(samp) != hdrl_imagelist_get_size(list)) {
                    size_t j = 0;
                    for (size_t i = 0; i < (size_t)hdrl_imagelist_get_size(list); i++) {
                        int dump;
                        hdrl_image * himg = hdrl_imagelist_get(list, i);
                        const cpl_image * img = cpl_imagelist_get_const(samplepos, i);
                        /* if any entry is bad skip */
                        if (hdrl_image_is_rejected(himg, x + 1, y + 1) ||
                            cpl_image_is_rejected(img, x + 1, y + 1))
                            continue;
                        /* refill vector with non bad pixels in order */
                        hdrl_value val = hdrl_image_get_pixel(himg, x + 1, y + 1, NULL);
                        cpl_vector_set(data, j, val.data);
                        cpl_vector_set(errs, j, val.error);
                        cpl_vector_set(samp, j, cpl_image_get(img, x + 1, y + 1, &dump));
                        j++;
                    }
                    cpl_vector_set_size(data, j);
                    cpl_vector_set_size(errs, j);
                    cpl_vector_set_size(samp, j);
                }

                hdrl_ls_fit_result * r = polyfit1d(samp, data, errs, degree);
                // TODO handle failure
                for (size_t z = 0; z < noz; z++) {
                    hdrl_image * oimg = hdrl_imagelist_get(*coef, z);
                    hdrl_image_set_pixel(oimg, x + 1, y + 1,
                                         (hdrl_value){cpl_matrix_get(r->coef, z, 0),
                                         sqrt(cpl_matrix_get(r->cov, z, z))});
                }
                if (chi2) {
                    cpl_image_set(*chi2, x + 1, y + 1,
                                  hdrl_ls_fit_result_get_chi2(r, data, errs));
                }
                if (dof) {
                    cpl_image_set(*dof, x + 1, y + 1,
                                  hdrl_ls_fit_result_get_residual_dof(r));
                }
                hdrl_ls_fit_result_delete(r);
                hdrl_cplvector_delete_to_cache(cache, data);
                hdrl_cplvector_delete_to_cache(cache, errs);
                hdrl_cplvector_delete_to_cache(cache, samp);
            }
        }
        hdrl_vector_cache_delete(cache);
    }

    cpl_imagelist_unwrap(datal);
    cpl_imagelist_unwrap(errorl);

    return cpl_error_get_code();
fail:
    hdrl_imagelist_delete(*coef);
    *coef = NULL;
    if (chi2) {
        cpl_image_delete(*chi2);
        *chi2 = NULL;
    }
    if (dof) {
        cpl_image_delete(*dof);
        *dof = NULL;
    }
    return cpl_error_get_code();
}
/**@}*/

