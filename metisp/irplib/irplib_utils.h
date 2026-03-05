/* $Id: irplib_utils.h,v 1.60 2013-08-21 14:55:14 cgarcia Exp $
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
 * $Author: cgarcia $
 * $Date: 2013-08-21 14:55:14 $
 * $Revision: 1.60 $
 * $Name: not supported by cvs2svn $
 * $Log: not supported by cvs2svn $
 * Revision 1.59  2013/03/15 09:06:06  jtaylor
 * add irplib_aligned_{[mc]alloc,free}
 *
 * allow portable allocation of aligned memory for vectorization
 *
 * Revision 1.58  2013/03/15 09:05:28  jtaylor
 * move isnan and isinf to header so it is inlineable and use gcc builtin for better performance
 *
 * Revision 1.57  2013/02/27 16:02:02  jtaylor
 * add diagnostic pragma macros
 *
 * Revision 1.56  2012/08/06 06:14:18  llundin
 * irplib_errorstate_warning(): Replaced by cpl_errorstate_dump_one_warning() from CPL 6.X
 *
 * Revision 1.55  2011/06/01 06:47:56  llundin
 * skip_if_lt(): Fix previous edits switch of A and B in error message
 *
 * Revision 1.54  2011/05/26 08:08:56  llundin
 * skip_if_lt(): Support printf-style error message, name-space protect temporary variables
 *
 * Revision 1.53  2011/05/09 07:51:18  llundin
 * irplib_dfs_save_image_(): Modified from cpl_dfs_save_image(). irplib_dfs_save_image(): Use irplib_dfs_save_image_()
 *
 * Revision 1.52  2010/03/23 07:57:59  kmirny
 * DFS08552, Documentation for irplib_frameset_sort
 *
 * Revision 1.51  2009/12/16 14:59:30  cgarcia
 * Avoid name clash with index function
 *
 * Revision 1.50  2009/08/17 15:10:16  kmirny
 *
 * DFS07454 DFS07437
 *
 */

#ifndef IRPLIB_UTILS_H
#define IRPLIB_UTILS_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <math.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

/* Concatenate two macro arguments */
#define IRPLIB_CONCAT(a,b) a ## _ ## b
#define IRPLIB_CONCAT2X(a,b) IRPLIB_CONCAT(a,b)

/*----------------------------------------------------------------------------*/
/*
  @brief   Swap two double values
  @param   A   The 1st double to swap
  @param   B   The 2nd double to swap
  @note A and B are evaluated twice, so side-effects should be avoided
*/
/*----------------------------------------------------------------------------*/
#define IRPLIB_SWAP_DOUBLE(A, B) do {           \
        const double irplib_swap_double=(A);    \
        (A)=(B);                                \
        (B)=irplib_swap_double;                 \
    } while (0)

#define IRPLIB_XSTRINGIFY CPL_XSTRINGIFY
#define IRPLIB_STRINGIFY  CPL_STRINGIFY

#if !defined __GNUC__ && !defined __inline__
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#define __inline__ inline
#else
#define __inline__
#endif
#endif

#define IRPLIB_DIAG_PRAGMA_PUSH_IGN CPL_DIAG_PRAGMA_PUSH_IGN
#define IRPLIB_DIAG_PRAGMA_PUSH_ERR CPL_DIAG_PRAGMA_PUSH_ERR
#define IRPLIB_DIAG_PRAGMA_POP      CPL_DIAG_PRAGMA_POP

/* FIXME: Remove when no longer used by any irplib-based pipelines */
/* Useful for debugging */
#define irplib_trace()  do if (cpl_error_get_code()) {                  \
        cpl_msg_debug(cpl_func, __FILE__ " at line "                    \
                      CPL_STRINGIFY(__LINE__) ": ERROR '%s' at %s",     \
                      cpl_error_get_message(), cpl_error_get_where());  \
    } else {                                                            \
        cpl_msg_debug(cpl_func, __FILE__ " at line "                    \
                      CPL_STRINGIFY(__LINE__) ": OK");                  \
    } while (0)
 
#define irplib_error_recover(ESTATE, ...)                       \
    do if (!cpl_errorstate_is_equal(ESTATE)) {                  \
        cpl_msg_warning(cpl_func, __VA_ARGS__);                 \
        cpl_msg_indent_more();                                  \
        cpl_errorstate_dump(ESTATE, CPL_FALSE,                  \
                            cpl_errorstate_dump_one_warning);   \
        cpl_msg_indent_less();                                  \
        cpl_errorstate_set(ESTATE);                             \
    } while (0)



/*----------------------------------------------------------------------------*/
/*
  @brief Declare a function suitable for use with irplib_dfs_table_convert()
  @param  table_set_row    The name of the function to declare
  @see irplib_dfs_table_convert(), irplib_table_read_from_frameset()

*/
/*----------------------------------------------------------------------------*/
#define IRPLIB_UTIL_SET_ROW(table_set_row)                      \
    cpl_boolean table_set_row(cpl_table *,                      \
                              const char *,                     \
                              int,                              \
                              const cpl_frame *,                \
                              const cpl_parameterlist *)


/*----------------------------------------------------------------------------*/
/*
  @brief Declare a function suitable for use with irplib_dfs_table_convert()
  @param  table_check    The name of the function to declare
  @see irplib_dfs_table_convert()

*/
/*----------------------------------------------------------------------------*/
#define IRPLIB_UTIL_CHECK(table_check)                          \
    cpl_error_code table_check(cpl_table *,                     \
                               const cpl_frameset *,            \
                               const cpl_parameterlist *)


/*----------------------------------------------------------------------------*/
/*
  @brief   Conditional skip to the (unqiue) return point of the function
  @param   CONDITION    The condition to check
  @see cpl_error_ensure()

  skip_if() takes one argument, which is a logical expression.
  If the logical expression is false skip_if() takes no action and
  program execution continues.
  If the logical expression is true this indicates an error. In this case
  skip_if() will set the location of the error to the point where it
  was invoked in the recipe code (unless the error location is already in the
  recipe code). If no error code had been set, then skip_if() will set one.
  Finally, skip_if() causes program execution to skip to the macro 'end_skip'.
  The macro end_skip is located towards the end of the function, after
  which all resource deallocation and the function return is located.

  The use of skip_if() assumes the following coding practice:
  1) Pointers used for dynamically allocated memory that they "own" shall always
     point to either NULL or to allocated memory (including CPL-objects).
  2) Such pointers may not be reused to point to memory whose deallocation
     requires calls to different functions.
  3) Pointers of type FILE should be set NULL when not pointing to an open
     stream and their closing calls (fclose(), freopen(), etc.) following the
     'end_skip' should be guarded against such NULL pointers.

  Error checking with skip_if() is encouraged due to the following advantages:
  1) It ensures that a CPL-error code is set.
  2) It ensures that the location of the error in the _recipe_ code is noted.
  3) The error checking may be confined to a single concise line.
  4) It is not necessary to replicate memory deallocation for every error
     condition.
  5) If more extensive error reporting/handling is required it is not precluded
     by the use of skip_if().
  6) It allows for a single point of function return.
  7) It allows for optional, uniformly formatted debugging/tracing information
     at each macro invocation.

  The implementation of skip_if() uses a goto/label construction.
  According to Kerningham & Ritchie, The C Programming Language, 2nd edition,
  Section 3.8:
  "This organization is handy if the error-handling code is non-trivial,
  and if errors can occur in several places."

  The use of goto for any other purpose should be avoided.

*/
/*----------------------------------------------------------------------------*/
#define skip_if(CONDITION)                                                     \
    do {                                                                       \
        cpl_error_ensure(!cpl_error_get_code(), cpl_error_get_code(),          \
                         goto cleanup, "Propagating a pre-existing error");    \
        cpl_error_ensure(!(CONDITION), cpl_error_get_code(),                   \
                         goto cleanup, "Propagating error");\
    } while (0)

/*----------------------------------------------------------------------------*/
/*
  @brief   Skip if A != B
  @param   A   The 1st double to compare
  @param   B   The 2nd double to compare
  @param   MSG A printf-style error message, 1st arg should be a string literal
  @see skip_if()
  @note A and B are evaluated exactly once

  If no CPL error is set, sets CPL_ERROR_DATA_NOT_FOUND on failure
*/
/*----------------------------------------------------------------------------*/
#define skip_if_ne(A, B, ...)                                                  \
    do {                                                                       \
        /* Name-space protected one-time only evaluation */                    \
        const double irplib_utils_a = (double)(A);                             \
        const double irplib_utils_b = (double)(B);                             \
                                                                               \
        cpl_error_ensure(!cpl_error_get_code(), cpl_error_get_code(),          \
                         goto cleanup, "Propagating a pre-existing error");    \
        if (irplib_utils_a != irplib_utils_b) {                                 \
            char * irplib_utils_msg = cpl_sprintf(__VA_ARGS__);                \
            (void)cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,    \
                                        "Need %g (not %g) %s",        \
                                        irplib_utils_b, irplib_utils_a,        \
                                        irplib_utils_msg);                     \
            cpl_free(irplib_utils_msg);                                        \
            goto cleanup;                                                      \
        }                                                                      \
    } while (0)

/*----------------------------------------------------------------------------*/
/*
  @brief   Skip if A < B
  @param   A   The 1st double to compare
  @param   B   The 2nd double to compare
  @param   MSG A printf-style error message, 1st arg should be a string literal
  @see skip_if()
  @note A and B are evaluated exactly once

  If no CPL error is set, sets CPL_ERROR_DATA_NOT_FOUND on failure
*/
/*----------------------------------------------------------------------------*/
#define skip_if_lt(A, B, ...)                                                  \
    do {                                                                       \
        /* Name-space protected one-time only evaluation */                    \
        const double irplib_utils_a = (double)(A);                             \
        const double irplib_utils_b = (double)(B);                             \
                                                                               \
        cpl_error_ensure(!cpl_error_get_code(), cpl_error_get_code(),          \
                         goto cleanup, "Propagating a pre-existing error");    \
        if (irplib_utils_a < irplib_utils_b) {                                 \
            char * irplib_utils_msg = cpl_sprintf(__VA_ARGS__);                \
            (void)cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,    \
                                        "Need at least %g (not %g) %s",        \
                                        irplib_utils_b, irplib_utils_a,        \
                                        irplib_utils_msg);                     \
            cpl_free(irplib_utils_msg);                                        \
            goto cleanup;                                                      \
        }                                                                      \
    } while (0)

/*----------------------------------------------------------------------------*/
/*
  @brief   Conditional skip on coding bug
  @param   CONDITION    The condition to check
  @see skip_if()
  @note unlike assert() this check cannot be disabled
 */
/*----------------------------------------------------------------------------*/
#define bug_if(CONDITION)                                                      \
    do {                                                                       \
        cpl_error_ensure(!cpl_error_get_code(), cpl_error_get_code(),          \
                         goto cleanup, "Propagating an unexpected error, "     \
                         "please report to " PACKAGE_BUGREPORT);               \
        cpl_error_ensure(!(CONDITION), CPL_ERROR_UNSPECIFIED,                  \
                         goto cleanup, "Internal error, please report to "     \
                         PACKAGE_BUGREPORT);                                   \
    } while (0)

/*----------------------------------------------------------------------------*/
/*
  @brief   Conditional skip with error creation
  @param   CONDITION    The condition to check
  @param   ERROR        The error code to set
  @param   MSG          A printf-style error message. As a matter of
                        user-friendliness the message should mention any
                        value that caused the @em CONDITION to fail.
  @see skip_if()
  @note unlike assert() this check cannot be disabled
 */
/*----------------------------------------------------------------------------*/
#define error_if(CONDITION, ERROR, ...)                                 \
    cpl_error_ensure(cpl_error_get_code() == CPL_ERROR_NONE &&          \
                     !(CONDITION), ERROR, goto cleanup,  __VA_ARGS__)

/*----------------------------------------------------------------------------*/
/*
  @brief   Propagate a preexisting error, if any
  @param   MSG          A printf-style error message.
  @see skip_if()
 */
/*----------------------------------------------------------------------------*/
#define any_if(...)                                                     \
    cpl_error_ensure(!cpl_error_get_code(), cpl_error_get_code(),       \
                     goto cleanup,  __VA_ARGS__)

/*----------------------------------------------------------------------------*/
/*
  @brief   Define the single point of resource deallocation and return
  @see skip_if()
  @note end_skip should be used exactly once in functions that use skip_if() etc
*/
/*----------------------------------------------------------------------------*/
#define end_skip                                                        \
    do {                                                                \
    cleanup:                                                            \
        if (cpl_error_get_code())                                       \
            cpl_msg_debug(cpl_func, "Cleanup in " __FILE__ " line "     \
                          CPL_STRINGIFY(__LINE__) " with error '%s' at %s", \
                          cpl_error_get_message(), cpl_error_get_where()); \
        else                                                            \
            cpl_msg_debug(cpl_func, "Cleanup in " __FILE__ " line "     \
                          CPL_STRINGIFY(__LINE__));                     \
    } while (0)


/*----------------------------------------------------------------------------*/
/**
   @brief   Conditional skip to the (unqiue) return point of the function
   @param   CONDITION           An error is set if this test condition
                                evaluates to false
   @param   ec                  The CPL error code
   @param   msg                 A printf-style error
                                message. As a matter of user-friendliness
                                the message should mention any value
                                that caused the @em CONDITION to fail.
   @see skip_if()
*/
/*----------------------------------------------------------------------------*/
#define irplib_ensure(CONDITION, ec, ...)                                      \
    cpl_error_ensure(CONDITION, ec, goto cleanup,  __VA_ARGS__)

/*----------------------------------------------------------------------------*/
/**
   @brief   Catch an error
   @param   COMMAND         Command to execute and check. This command is
                            expected to set the cpl_error_code in case of
                            failure
   @param   msg             A printf-style error message.
   @see skip_if()

   This macro is used to catch an error from a function that sets the
   @c cpl_error_code in case of error.

   Example:
   @code
   irplib_check( cpl_object_do_something(object), ("Could not do something"));
   @endcode

   If the @c cpl_error_code is set before or after calling the function,
   execution jumps to the @em cleanup label.

   The macro can also be used to check a sequence of commands:
   @code
   irplib_check(
       x = cpl_table_get_int(table, "x", 0, NULL);
       y = cpl_table_get_int(table, "y", 0, NULL);
       z = cpl_table_get_int(table, "z", 0, NULL),    / *  Comma here!  * /
       ("Error reading wavelength catalogue"));
   @endcode

*/
/*----------------------------------------------------------------------------*/

#define irplib_check(COMMAND, ...)                                      \
    do {                                                                \
        cpl_errorstate irplib_check_prestate = cpl_errorstate_get();    \
        skip_if(0);                                                     \
        COMMAND;                                                        \
        irplib_trace();                                                 \
        irplib_ensure(cpl_errorstate_is_equal(irplib_check_prestate),   \
                      cpl_error_get_code(), __VA_ARGS__);               \
        irplib_trace();                                                 \
    } while (0)

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code irplib_dfs_save_image(cpl_frameset            *,
                                     const cpl_parameterlist *,
                                     const cpl_frameset      *,
                                     const cpl_image         *,
                                     cpl_type_bpp             ,
                                     const char              *,
                                     const char              *,
                                     const cpl_propertylist  *,
                                     const char              *,
                                     const char              *,
                                     const char              *);


cpl_error_code irplib_dfs_save_propertylist(cpl_frameset            *,
                                            const cpl_parameterlist *,
                                            const cpl_frameset      *,
                                            const char              *,
                                            const char              *,
                                            const cpl_propertylist  *,
                                            const char              *,
                                            const char              *,
                                            const char              *);

cpl_error_code irplib_dfs_save_imagelist(cpl_frameset            *,
                                         const cpl_parameterlist *,
                                         const cpl_frameset      *,
                                         const cpl_imagelist     *,
                                         cpl_type_bpp             ,
                                         const char              *,
                                         const char              *,
                                         const cpl_propertylist  *,
                                         const char              *,
                                         const char              *,
                                         const char              *);

cpl_error_code irplib_dfs_save_table(cpl_frameset            *,
                                     const cpl_parameterlist *,
                                     const cpl_frameset      *,
                                     const cpl_table         *,
                                     const cpl_propertylist  *,
                                     const char              *,
                                     const char              *,
                                     const cpl_propertylist  *,
                                     const char              *,
                                     const char              *,
                                     const char              *);

cpl_error_code irplib_dfs_save_image_(cpl_frameset            *,
                                      cpl_propertylist        *,
                                      const cpl_parameterlist *,
                                      const cpl_frameset      *,
                                      const cpl_frame         *,
                                      const cpl_image         *,
                                      cpl_type                 ,
                                      const char              *,
                                      const cpl_propertylist  *,
                                      const char              *,
                                      const char              *,
                                      const char              *);

void irplib_reset(void);
int irplib_compare_tags(cpl_frame *, cpl_frame *);
const char * irplib_frameset_find_file(const cpl_frameset *, const char *);
const cpl_frame * irplib_frameset_get_first_from_group(const cpl_frameset *,
                                                       cpl_frame_group);

cpl_error_code irplib_apertures_find_max_flux(const cpl_apertures *, int *,
                                              int);

#if defined HAVE_ISNAN && HAVE_ISNAN != 0
#if !defined isnan && defined HAVE_DECL_ISNAN && HAVE_DECL_ISNAN == 0
/* HP-UX and Solaris may have isnan() available at link-time
   without the prototype */
int isnan(double);
#endif
#endif

cpl_error_code
irplib_dfs_table_convert(cpl_table *, cpl_frameset *, const cpl_frameset *,
                         int, char, const char *, const char *,
                         const cpl_parameterlist *, const char *,
                         const cpl_propertylist *, const cpl_propertylist *,
                         const char *, const char *, const char *,
                         cpl_boolean (*)(cpl_table *, const char *, int,
                                            const cpl_frame *,
                                            const cpl_parameterlist *),
                         cpl_error_code (*)(cpl_table *,
                                            const cpl_frameset *,
                                            const cpl_parameterlist *));

cpl_error_code irplib_table_read_from_frameset(cpl_table *,
                                               const cpl_frameset *,
                                               int,
                                               char,
                                               const cpl_parameterlist *,
                                               cpl_boolean (*)
                                               (cpl_table *, const char *,
                                                int, const cpl_frame *,
                                                const cpl_parameterlist *));

cpl_error_code irplib_image_split(const cpl_image *,
                                  cpl_image *, cpl_image *, cpl_image *,
                                  double, cpl_boolean,
                                  double, cpl_boolean,
                                  double, double,
                                  cpl_boolean, cpl_boolean, cpl_boolean);

void irplib_errorstate_dump_warning(unsigned, unsigned, unsigned);
void irplib_errorstate_dump_info(unsigned, unsigned, unsigned);
void irplib_errorstate_dump_debug(unsigned, unsigned, unsigned);
/* wrapper for replace deprecated function cpl_polynomial_fit_1d_create*/
cpl_polynomial * irplib_polynomial_fit_1d_create(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   mse
        );
cpl_polynomial * irplib_polynomial_fit_1d_create_chiq(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   rechiq
        );
/*----------------------------------------------------------------------------*/
/**
   @brief   Sort a frameset based on the exposure time
   @param   self         Frameset to sort (input)
   @param   iindex       Index array with sort results (output), each element is
					     a frame number
   @param   exptime      Array with exposure time for each frame (output)
*/
cpl_error_code irplib_frameset_sort(
        const cpl_frameset *  self,
        int* iindex,
        double* exptime);


/* FIXME: add alloc_size(2) */
void * irplib_aligned_malloc(size_t alignment, size_t size) CPL_ATTR_ALLOC;
void * irplib_aligned_calloc(size_t alignment,
                             size_t nelem, size_t nbytes) CPL_ATTR_ALLOC;
void irplib_aligned_free (void * aligned_ptr);

const cpl_frame *
irplib_frameset_get_first_const(cpl_frameset_iterator **iterator,
                                const cpl_frameset *frameset);

const cpl_frame *
irplib_frameset_get_next_const(cpl_frameset_iterator *iterator);


void irplib_vector_get_kth(cpl_vector *, cpl_size);

/*-----------------------------------------------------------------------------
                                   Function inlines
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
   @brief   portable isinf
 */
/*----------------------------------------------------------------------------*/
static __inline__ int irplib_isinf(double value)
{
/* documented only on 4.4, but available in at least 4.2 */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
    return __builtin_isinf(value);
#elif defined HAVE_ISINF && HAVE_ISINF
    return isinf(value);
#else
    return value != 0 && value == 2 * value;
#endif
}


/*----------------------------------------------------------------------------*/
/**
   @brief   portable isnan
 */
/*----------------------------------------------------------------------------*/
static __inline__ int irplib_isnan(double value)
{
/* documented only on 4.4, but available in at least 4.2 */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
    return __builtin_isnan(value);
#elif defined HAVE_ISNAN && HAVE_ISNAN
    return isnan(value);
#else
    return value != value;
#endif
}

#endif
