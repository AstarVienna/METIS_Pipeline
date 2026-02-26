/* $Id: hdrl_buffer-test.c,v 1.1 2013-10-23 09:13:56 jtaylor Exp $
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
 * $Date: 2013-10-23 09:13:56 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#include "hdrl_buffer.h"

#include <cpl.h>

#include <stdlib.h>
#include <string.h>


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_buffer_test   
            Testing of hdrl_buffer module
 */
/*----------------------------------------------------------------------------*/

cpl_error_code test_basic(void)
{
    hdrl_buffer *buf = hdrl_buffer_new();

    hdrl_buffer_readonly(buf, CPL_TRUE);
    hdrl_buffer_readonly(buf, CPL_FALSE);

    char * p2 = hdrl_buffer_allocate(buf, 10023);
    memset(p2, 1, 10023);

    size_t size = hdrl_buffer_set_malloc_threshold(buf, sizeof(int));
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test(size == 0.);

    hdrl_buffer_readonly(buf, CPL_TRUE);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_buffer_readonly(buf, CPL_FALSE);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_buffer_free(buf, p2);
    hdrl_buffer_delete(buf);

    cpl_msg_info(cpl_func, "test pool <= object");
    buf = hdrl_buffer_new();
    for (size_t i = 0; i < 100; i++) {
        char * p = hdrl_buffer_allocate(buf, 1<<20);
        memset(p, 1, 1<<20);
        hdrl_buffer_free(buf, p);
    }
    hdrl_buffer_delete(buf);

    cpl_msg_info(cpl_func, "test pool > object");
    buf = hdrl_buffer_new();
    for (size_t i = 0; i < 1000; i++) {
        char * p = hdrl_buffer_allocate(buf, 1<<18);
        p[(1<<18) - 1] = 1;
        hdrl_buffer_free(buf, p);
    }

    hdrl_buffer_delete(NULL);

    hdrl_buffer_delete(buf);

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_basic();

    setenv("HDRL_BUFFER_MALLOC", "1", 1);
    test_basic();

    return cpl_test_end(0);
}
