/* $Id: hdrl_parameter-test.c,v 1.1 2013-10-01 13:26:32 jtaylor Exp $
 *
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-01 13:26:32 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h"

#include <cpl.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_parameter-test   Testing of the HDRL parameter
 */
/*----------------------------------------------------------------------------*/


/* define sigma clipping parameter object */
typedef struct {
    HDRL_PARAMETER_HEAD;
    double kappa_low;
    double kappa_high;
    int niter;
} hdrl_parameter_sigclip;

/* define type metadata */
static hdrl_parameter_typeobj hdrl_parameter_sigclip_type = {
    HDRL_PARAMETER_COLLAPSE_SIGCLIP,/* type */
    (hdrl_alloc *)&cpl_malloc,      /* fp_alloc */
    (hdrl_free *)&cpl_free,         /* fp_free */
    NULL,                           /* fp_destroy */
    sizeof(hdrl_parameter_sigclip), /* obj_size */
};

/* define initializing constructor */
static hdrl_parameter * hdrl_parameter_sigclip_create(double kappa, int niter)
{
    hdrl_parameter_sigclip * p = (hdrl_parameter_sigclip *)
       hdrl_parameter_new(&hdrl_parameter_sigclip_type);
    p->kappa_low = kappa;
    p->kappa_high = kappa;
    p->niter = niter;
    return (hdrl_parameter *)p;
}

static int hdrl_parameter_is_sigclip(hdrl_parameter *self)
{
    return hdrl_parameter_check_type(self, &hdrl_parameter_sigclip_type);
}

/* public */
static double hdrl_parameter_sigclip_get_kappa(hdrl_parameter *p)
{
	cpl_test(hdrl_parameter_is_sigclip(p));

    hdrl_parameter_sigclip * sp = (hdrl_parameter_sigclip *)p;
    return sp->kappa_low;
}



typedef struct {
    HDRL_PARAMETER_HEAD;
    int hbox_size;
    hdrl_parameter * collapse;
} hdrl_parameter_overscan;

static void hdrl_parameter_overscan_destroy(hdrl_parameter *);

static hdrl_parameter_typeobj hdrl_parameter_overscan_type = {
    HDRL_PARAMETER_OVERSCAN,                       /* type */
    (hdrl_alloc *)&cpl_malloc,                     /* fp_alloc */
    (hdrl_free *)&cpl_free,                        /* fp_free */
    (hdrl_free *)&hdrl_parameter_overscan_destroy, /* fp_destroy */
    sizeof(hdrl_parameter_overscan),               /* obj_size */
};

static hdrl_parameter *
hdrl_parameter_overscan_create(int hbox_size, hdrl_parameter *collapse)
{
    hdrl_parameter_overscan * p =
        (hdrl_parameter_overscan *)hdrl_parameter_new(&hdrl_parameter_overscan_type);
    p->hbox_size = hbox_size;
    p->collapse = collapse;
    return (hdrl_parameter *)p;
}

static int hdrl_parameter_is_overscan(hdrl_parameter *p)
{
    return hdrl_parameter_check_type(p, &hdrl_parameter_overscan_type);
}

static void hdrl_parameter_overscan_destroy(hdrl_parameter *p)
{
	cpl_test(hdrl_parameter_is_overscan(p));
    hdrl_parameter_destroy(((hdrl_parameter_overscan*)p)->collapse);
    hdrl_parameter_get_type(p)->fp_free(p);
}

static double hdrl_parameter_overscan_get_hbox_size(hdrl_parameter *p)
{
	cpl_test(hdrl_parameter_is_overscan(p));

    hdrl_parameter_overscan * sp = (hdrl_parameter_overscan *)p;
    return sp->hbox_size;
}

static hdrl_parameter * hdrl_parameter_overscan_get_collapse(hdrl_parameter *p)
{
    cpl_test(hdrl_parameter_is_overscan(p));

    hdrl_parameter_overscan * sp = (hdrl_parameter_overscan *)p;
    return sp->collapse;
}

static void test_parameters(hdrl_parameter *p)
{
    hdrl_parameter * collapse = hdrl_parameter_overscan_get_collapse(p);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(collapse);

    cpl_test(hdrl_parameter_is_sigclip(collapse));
    cpl_test(!hdrl_parameter_is_overscan(collapse));

    cpl_test(!hdrl_parameter_is_sigclip(p));
    cpl_test(hdrl_parameter_is_overscan(p));

    cpl_test_eq(0, hdrl_parameter_overscan_get_hbox_size(p));

    cpl_test_eq(2, hdrl_parameter_sigclip_get_kappa(collapse));
    cpl_test_eq(2, ((hdrl_parameter_sigclip*)collapse)->kappa_low);
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of parameter module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    /* Test null */
    hdrl_parameter_delete(NULL);
    hdrl_parameter_destroy(NULL);
    cpl_test_error(CPL_ERROR_NONE);

    /* Test sigclip_create */
    hdrl_parameter *collapse = hdrl_parameter_sigclip_create(2, 3);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(collapse);

    /* Test get kappa */
    double kappa = hdrl_parameter_sigclip_get_kappa(collapse);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(kappa, 2);

    /* Test create overscan */
    hdrl_parameter *osp = hdrl_parameter_overscan_create(0, collapse);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(osp);

    /* More tests over the parameters */
    test_parameters(osp);
    cpl_test_error(CPL_ERROR_NONE);

    /* Test get size */
    double size = hdrl_parameter_overscan_get_hbox_size(osp);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(size, 0);

    /* Test get collapse */
    hdrl_parameter *test = hdrl_parameter_overscan_get_collapse(osp);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(test);

    /* Test destroy existing parameter */
    hdrl_parameter_destroy(osp);
    cpl_test_error(CPL_ERROR_NONE);

    /* Test delete parameter */
    collapse = hdrl_parameter_sigclip_create(2, 3);
    osp = hdrl_parameter_overscan_create(0, collapse);
    hdrl_parameter_delete(osp);
    hdrl_parameter_delete(collapse);
    cpl_test_error(CPL_ERROR_NONE);


    return cpl_test_end(0);
}

