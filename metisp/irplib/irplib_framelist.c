/* 
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/*-----------------------------------------------------------------------------
                                 Includes
 -----------------------------------------------------------------------------*/

#include "irplib_framelist.h"
#include "irplib_utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <math.h>
#include <assert.h>


/*-----------------------------------------------------------------------------
                                 New types
 -----------------------------------------------------------------------------*/

/* @cond */
struct _irplib_framelist_ {
    int size;
    cpl_frame        ** frame;
    cpl_propertylist ** propertylist;

};
/* @endcond */

/*-----------------------------------------------------------------------------
                                 Private funcions
 -----------------------------------------------------------------------------*/

static void irplib_framelist_set_size(irplib_framelist *)
#if defined __GNUC__ &&  __GNUC__ >= 4
    __attribute__((nonnull))
#endif
;

static cpl_boolean irplib_property_equal(const cpl_propertylist *,
                                         const cpl_propertylist *,
                                         const char *, cpl_type, double,
                                         char **, char **)
#if defined __GNUC__ &&  __GNUC__ >= 4
    __attribute__((nonnull))
#endif
;

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_framelist  Lists of frames with properties.

   This module implements a container type for frames and their propertylists.
   It differs from the cpl_frameset in these ways:
   1) A propertylist can be associated to each frame
   2) Access by index is a O(1)-operation
   3) It can not be corrupted due to caching bugs (e.g. DFS02731).

   @par Synopsis:
   @code
     #include <irplib_framelist.h>
   @endcode

   @par Example:
   @code

   static int rrecipe(cpl_frameset * frameset)
   {
      // Error handling omitted for brevity

      irplib_framelist * allframes = irplib_framelist_cast(frameset);

      // Get raw frames of either type
      irplib_framelist * rawframes = irplib_framelist_extract_regexp(allframes,
                                                                   "^("
                                                                   RAW_TYPE1 "|"
                                                                   RAW_TYPE2 ")$",
                                                                   CPL_FALSE);

      // Load the list of images
      cpl_imagelist * ilist = irplib_imagelist_load_framelist(rawframes,
                                                              CPL_TYPE_FLOAT,
                                                              0, 0);

      const cpl_propertylist * plist;

      // A regular expression of the FITS cards needed by this recipe
      const char cards[] = "^(RA|DEC|EXPTIME)$";
      double ra, dec;


      // Load the specified FITS cards for all raw frames
      irplib_framelist_load_propertylist_all(rawframes, 0, cards, CPL_FALSE));


      // Verify the presence and uniformity of the FITS cards
      if (irplib_framelist_contains(rawframes, "RA",
                                     CPL_TYPE_DOUBLE, CPL_TRUE, 1e-5)) {
         // RA is missing in one or more headers
         //  - or it varies by more than 1e-5
      }

       if (irplib_framelist_contains(rawframes, "DEC",
                                     CPL_TYPE_DOUBLE, CPL_TRUE, 1e-5)) {
         // DEC is missing in one or more headers
         //  - or it varies by more than 1e-5
      }

      // Process the FITS cards 
      plist = irplib_framelist_get_propertylist_const(rawframes, 0);

      ra  = cpl_propertylist_get_double(plist, "RA");
      dec = cpl_propertylist_get_double(plist, "DEC");

      // Object deallocation
      irplib_framelist_delete(allframes);
      irplib_framelist_delete(rawframes);
      cpl_imagelist_delete(ilist);

      return 0;

   }

   @endcode


 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Create an empty framelist
  @return   1 newly allocated irplib_framelist
  @note The returned irplib_framelist must be deallocated using
        irplib_framelist_delete()

 */
/*----------------------------------------------------------------------------*/
irplib_framelist * irplib_framelist_new(void)
{

    return (irplib_framelist *) cpl_calloc(1, sizeof(irplib_framelist));

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Deallocate an irplib_framelist with its frames and properties
  @param    self  the framelist
 */
/*----------------------------------------------------------------------------*/
void irplib_framelist_delete(irplib_framelist * self)
{

    irplib_framelist_empty(self);
    cpl_free(self);
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Create an irplib_framelist from a cpl_framelist
  @param    frameset The cpl_frameset
  @return   1 newly allocated irplib_framelist or NULL on error
  @note The returned irplib_framelist must be deallocated using
        irplib_framelist_delete()

 */
/*----------------------------------------------------------------------------*/
irplib_framelist * irplib_framelist_cast(const cpl_frameset * frameset)
{
    irplib_framelist * self;
    int i ;

    cpl_ensure(frameset != NULL, CPL_ERROR_NULL_INPUT, NULL);

    /* The function cannot fail now */
    self = irplib_framelist_new();

    for (i = 0; i < cpl_frameset_get_size(frameset); i++) 
    {
        const cpl_frame * frame = cpl_frameset_get_position_const(frameset, i);

        cpl_frame * copy = cpl_frame_duplicate(frame);

#ifndef NDEBUG
        const cpl_error_code error =
#endif
            irplib_framelist_set(self, copy, i);

        assert(error == CPL_ERROR_NONE);

    }

    assert(self->size == cpl_frameset_get_size(frameset));

    return self;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Create a CPL frameset from an irplib_framelist
  @param    self    The framelist
  @return   1 newly allocated cpl_frameset or NULL on error
  @note The returned cpl_frameset must be deallocated using
        cpl_frameset_delete()

 */
/*----------------------------------------------------------------------------*/
cpl_frameset * irplib_frameset_cast(const irplib_framelist * self)
{
    cpl_frameset * new;
    int i ;

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);

    /* The function cannot fail now */
    new = cpl_frameset_new();

    for (i = 0; i < self->size; i++) {
        cpl_frame * frame = cpl_frame_duplicate(self->frame[i]);
#ifndef NDEBUG
        const cpl_error_code error =
#endif
            cpl_frameset_insert(new, frame);

        assert(error == CPL_ERROR_NONE);

    }

    assert(self->size == cpl_frameset_get_size(new));

    return new;

}


/*----------------------------------------------------------------------------*/
/**
   @brief Extract the frames with the given tag from a framelist
   @param self  A non-empty framelist
   @param tag   The frame tag to search for.
   @return      The newly created framelist or NULL on error
   @see         cpl_frameset_find
   @note Any propertylists of the extracted frames are also extracted.
         It is an error if no matching frames are found, in which case
         an error is set.

 */
/*----------------------------------------------------------------------------*/
irplib_framelist * irplib_framelist_extract(const irplib_framelist * self,
                                            const char * tag)
{

    irplib_framelist * new;
    int i, newsize;


    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(tag  != NULL, CPL_ERROR_NULL_INPUT, NULL);

    new = irplib_framelist_new();
    newsize = 0;

    for (i = 0; i < self->size; i++) {
        const cpl_frame * frame = self->frame[i];
        const char * ftag = cpl_frame_get_tag(frame);
        cpl_frame * copy;
        cpl_error_code code;

        if (ftag == NULL) {
            /* The frame is ill-formed */
            irplib_framelist_delete(new);
            (void)cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
            return NULL;
        }

        if (strcmp(tag, ftag)) continue;

        copy = cpl_frame_duplicate(frame);

        code = irplib_framelist_set(new, copy, newsize);
        if (code != CPL_ERROR_NONE) break; /* Should not be possible */

        if (self->propertylist[i] != NULL) new->propertylist[newsize]
            = cpl_propertylist_duplicate(self->propertylist[i]);

        newsize++;
    }

    assert( newsize == new->size );

    if (newsize == 0) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "The list of %d frame(s) has no frames "
                              "with tag: %s", self->size, tag);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "The list of frame(s) has no frames "
                              "with the given tag");
#endif
        irplib_framelist_delete(new);
        new = NULL;
    }

    return new;

}

/*----------------------------------------------------------------------------*/
/**
   @brief Extract the frames with the given tag from a framelist
   @param self   A non-empty framelist
   @param regexp The regular expression of frame tag(s) to search for.
   @param invert Boolean to invert the sense of the pattern matching.
   @return       The newly created framelist or NULL on error
   @see irplib_framelist_extract

 */
/*----------------------------------------------------------------------------*/
irplib_framelist * irplib_framelist_extract_regexp(const irplib_framelist* self,
                                                   const char * regexp,
                                                   cpl_boolean invert)
{

    irplib_framelist * new;
    int error;
    int i, newsize;
    const int xor_val = (invert == CPL_FALSE ? 0 : 1);
    regex_t re;


    cpl_ensure(self   != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(regexp != NULL, CPL_ERROR_NULL_INPUT, NULL);

    error = regcomp(&re, regexp, REG_EXTENDED | REG_NOSUB);
    cpl_ensure(!error, CPL_ERROR_ILLEGAL_INPUT, NULL);

    new = irplib_framelist_new();
    newsize = 0;

    for (i = 0; i < self->size; i++) {
        const cpl_frame * frame = self->frame[i];
        const char * tag = cpl_frame_get_tag(frame);
        cpl_frame * copy;

        if (tag == NULL) {
            /* The frame is ill-formed */
            irplib_framelist_delete(new);
            regfree(&re);
            cpl_ensure(0, CPL_ERROR_ILLEGAL_INPUT, NULL);
        }

        if ((regexec(&re, tag, (size_t)0, NULL, 0) == REG_NOMATCH ? 1 : 0)
            ^ xor_val) continue;

        copy = cpl_frame_duplicate(frame);

        error = (int)irplib_framelist_set(new, copy, newsize);
        assert(error == CPL_ERROR_NONE);

        if (self->propertylist[i] != NULL) new->propertylist[newsize]
            = cpl_propertylist_duplicate(self->propertylist[i]);

        newsize++;

    }

    regfree(&re);

    assert( newsize == new->size );

    if (newsize == 0) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "The list of %d frame(s) has no frames "
                              "that match: %s", self->size, regexp);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "The list of frames has no frames "
                              "that match the regular expression");
#endif
        irplib_framelist_delete(new);
        new = NULL;
    }

    return new;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Get the size of a framelist
  @param    self  The framelist
  @return   The size or a negative number on error

 */
/*----------------------------------------------------------------------------*/
int irplib_framelist_get_size(const irplib_framelist * self)
{

    cpl_ensure(self != NULL,  CPL_ERROR_NULL_INPUT, -1);

    return self->size;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the specified frame from the framelist
  @param    self  The framelist
  @param    pos   position (0 for first)
  @return   The frame or NULL on error

 */
/*----------------------------------------------------------------------------*/
cpl_frame * irplib_framelist_get(irplib_framelist * self, int pos)
{
    IRPLIB_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    return (cpl_frame *)irplib_framelist_get_const(self, pos);
    IRPLIB_DIAG_PRAGMA_POP;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Get the specified frame from the framelist
  @param    self  The framelist
  @param    pos   position (0 for first)
  @return   The frame or NULL on error

 */
/*----------------------------------------------------------------------------*/
const cpl_frame * irplib_framelist_get_const(const irplib_framelist * self,
                                             int pos)
{

    cpl_ensure(self != NULL,      CPL_ERROR_NULL_INPUT,          NULL);
    cpl_ensure(pos >= 0,          CPL_ERROR_ILLEGAL_INPUT,       NULL);
    cpl_ensure(pos  < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    return self->frame[pos];

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Duplicate a propertylist to the specified position in the framelist
  @param    self   The framelist to modify
  @param    pos    position (0 for first).
  @param    list   The propertylist to copy
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_set_propertylist(irplib_framelist * self,
                                                 int pos,
                                                 const cpl_propertylist * list)
{

    cpl_ensure_code(self != NULL,      CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(list != NULL,      CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pos  >= 0,         CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pos  < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE);

    cpl_propertylist_delete(self->propertylist[pos]);

    self->propertylist[pos] = cpl_propertylist_duplicate(list);

    cpl_ensure_code(self->propertylist[pos] != NULL, cpl_error_get_code());

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Get the propertylist of the specified frame in the framelist
  @param    self  The framelist
  @param    pos   position (0 for first)
  @return   The propertylist or NULL on error
  @note The propertylist must first be created, for example with
        irplib_framelist_load_propertylist(self, pos, ...), otherwise
        an error occurs.

 */
/*----------------------------------------------------------------------------*/
cpl_propertylist * irplib_framelist_get_propertylist(irplib_framelist * self,
                                                     int pos)
{

    IRPLIB_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    return (cpl_propertylist *)irplib_framelist_get_propertylist_const(self,
                                                                       pos);
    IRPLIB_DIAG_PRAGMA_POP;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Get the propertylist of the specified frame in the framelist
  @param    self  The framelist
  @param    pos   position (0 for first)
  @return   The propertylist or NULL on error
  @note The propertylist must first be created, for example with
        irplib_framelist_load_propertylist(self, pos, ...), otherwise
        an error occurs.

 */
/*----------------------------------------------------------------------------*/
const cpl_propertylist * irplib_framelist_get_propertylist_const(
                                                  const irplib_framelist * self,
                                                  int pos)
{
    cpl_ensure(self != NULL,      CPL_ERROR_NULL_INPUT,          NULL);
    cpl_ensure(pos  >= 0,         CPL_ERROR_ILLEGAL_INPUT,       NULL);
    cpl_ensure(pos  < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    cpl_ensure(self->propertylist[pos] != NULL,
                  CPL_ERROR_DATA_NOT_FOUND, NULL);

    return self->propertylist[pos];

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Load the propertylist of the specified frame in the framelist
  @param    self   The framelist to modify
  @param    pos    position (0 for first).
  @param    ind    The index of the date set to read
  @param    regexp The regular expression of properties to load
  @param    invert Boolean to invert the sense of the pattern matching.
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @see      cpl_propertylist_load_regexp()
  @note     Use a regexp of ".?" to load all properties. If a propertylist
            already exists it is deleted and replaced by the new one.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_load_propertylist(irplib_framelist * self,
                                                  int pos, int ind,
                                                  const char * regexp,
                                                  cpl_boolean invert)
{

    const char * filename;


    cpl_ensure_code(self   != NULL,    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(regexp != NULL,    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pos >= 0,          CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pos <  self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE);

    filename = cpl_frame_get_filename(self->frame[pos]);

    cpl_ensure_code(filename != NULL, cpl_error_get_code());

    cpl_propertylist_delete(self->propertylist[pos]);

    self->propertylist[pos] = cpl_propertylist_load_regexp(filename, ind,
                                                           regexp,
                                                           invert ? 1 : 0);

    if (self->propertylist[pos] == NULL) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        return cpl_error_set_message(cpl_func, cpl_error_get_code(), "Could "
                                     "not load FITS header from '%s' using "
                                     "regexp '%s'", filename, regexp);
#else
        return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                     "Could not load FITS header");
#endif
    }

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Load the propertylists of all frames in the framelist
  @param    self   The framelist to modify
  @param    ind    The index of the date set to read
  @param    regexp The regular expression of properties to load
  @param    invert Boolean to invert the sense of the pattern matching.
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @see      irplib_framelist_load_propertylist()
  @note     Use a regexp of ".?" to load all properties. If a frame already has
            a propertylist, it is not modified (and no propertylist is loaded
            for that frame).

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_load_propertylist_all(irplib_framelist * self,
                                                      int ind,
                                                      const char * regexp,
                                                      cpl_boolean invert)
{

    int nprops = 0;
    int nfiles = 0;
    int i;

    cpl_ensure_code(self   != NULL,    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(regexp != NULL,    CPL_ERROR_NULL_INPUT);

    for (i=0; i < self->size; i++) {
        if (self->propertylist[i] == NULL)
            cpl_ensure_code(!irplib_framelist_load_propertylist(self, i,
                                                                ind,
                                                                regexp,
                                                                invert),
                               cpl_error_get_code());
        /* Counting just for diagnostics - this actually causes
           the whole list to be reiterated :-( */
        nprops += cpl_propertylist_get_size(self->propertylist[i]);
        nfiles++;
    }

    cpl_msg_info(cpl_func, "List of %d frames has %d properties", nfiles,
                 nprops);

    return CPL_ERROR_NONE;

}



/*----------------------------------------------------------------------------*/
/**
  @brief    Set the tag of all frames in the list
  @param    self   The framelist to modify
  @param    tag    The new tag of the frames
  @return   CPL_ERROR_NONE or the relevant _cpl_error_code_

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_set_tag_all(irplib_framelist * self,
                                            const char * tag)
{

    int i;

    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(tag  != NULL, CPL_ERROR_NULL_INPUT);

    for (i=0; i < self->size; i++)
        cpl_ensure_code(!cpl_frame_set_tag(self->frame[i], tag),
                           cpl_error_get_code());

    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add a frame to a framelist
  @param    self  The framelist to modify
  @param    frame The frame to insert into the framelist
  @param    pos   position (0 for first).
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @note It is an error to call cpl_frame_delete() on a frame that is inserted
  in a framelist.

  It is allowed to specify the position equal to the size of the list.
  This will increment the size of the list.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_set(irplib_framelist * self, cpl_frame * frame,
                                    int pos)
{

    cpl_ensure_code(self  != NULL,     CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(frame != NULL,     CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pos >= 0,          CPL_ERROR_ILLEGAL_INPUT);

    if (pos == self->size) {

        self->size++;

        irplib_framelist_set_size(self);

    } else {

        cpl_ensure_code(pos < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE);

        cpl_frame_delete(self->frame[pos]);
        cpl_propertylist_delete(self->propertylist[pos]);
    }

    self->frame[pos] = frame;
    self->propertylist[pos] = NULL;

    return CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Erase a frame from a framelist and delete it and its propertylist
  @param    self  The non-empty framelist to modify
  @param    pos   position of frame to delete (0 for first).
  @return   CPL_ERROR_NONE or the relevant CPL error code


 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_erase(irplib_framelist * self, int pos)
{

    int i;

    cpl_ensure_code(self  != NULL,    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pos >= 0,         CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pos < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE);


    /* Delete the specified frame and its propertylist */
    cpl_frame_delete(self->frame[pos]);
    cpl_propertylist_delete(self->propertylist[pos]);

    /* Move following frames down one position */
    for (i = pos+1; i < self->size; i++) {

        self->frame[i-1] = self->frame[i];

        self->propertylist[i-1] = self->propertylist[i];

    }

    self->size--;

    irplib_framelist_set_size(self);

    return CPL_ERROR_NONE;

}



/*----------------------------------------------------------------------------*/
/**
  @brief    Erase a frame from a framelist and return it to the caller
  @param    self  The non-empty framelist to modify
  @param    pos   position of frame to delete (0 for first).
  @param    plist Pointer to a propertylist or NULL
  @return   CPL_ERROR_NONE or the relevant CPL error code

  The specified frame is removed from the framelist and its size is decreased
  by one. The frame is returned to the caller. The caller may also retrieve
  the propertylist of the frame by passing a non-NULL pointer. On success
  this may point to NULL, if a propertylist was not created for the
  frame. If the caller passes a NULL-pointer for the propertylist, the
  propertylist is deallocated.

 */
/*----------------------------------------------------------------------------*/
cpl_frame * irplib_framelist_unset(irplib_framelist * self, int pos,
                                   cpl_propertylist ** plist)

{
    cpl_frame * frame;
    int i;


    cpl_ensure(self  != NULL,    CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(pos >= 0,         CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(pos < self->size, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    /* Get the specified frame and its propertylist */
    frame = self->frame[pos];

    if (plist != NULL)
        *plist = self->propertylist[pos];
    else
        cpl_propertylist_delete(self->propertylist[pos]);


    /* Move following frames down one position */
    for (i = pos+1; i < self->size; i++) {

        self->frame[i-1] = self->frame[i];

        self->propertylist[i-1] = self->propertylist[i];

    }

    self->size--;

    irplib_framelist_set_size(self);

    return frame;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Erase all frames from a framelist
  @param    self  The framelist to modify, or NULL
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
void irplib_framelist_empty(irplib_framelist * self)
{

    if (self != NULL) {

        /* Deallocate all frames and their propertylists */
        while (self->size > 0) {
            self->size--;
            cpl_frame_delete(self->frame[self->size]);
            cpl_propertylist_delete(self->propertylist[self->size]);

        }
        
        /* Deallocate the arrays */
        irplib_framelist_set_size(self);

    }
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Verify that a property is present for all frames
  @param    self     The framelist to verify
  @param    key      Property that must be present for all the frames
  @param    type     The type the property must have, or CPL_TYPE_INVALID
  @param    is_equal If true, the value must be identical for all keys
  @param    fp_tol   The non-negative tolerance for floating point comparison
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @note It is allowed for a frame to have a NULL propertylist,
        in which case no check is performed.
        If type is CPL_TYPE_INVALID the check for a specific type is disabled.
        However, with is_equal true, all properties must nevertheless have the
        same type.
        fp_tol is used only when is_equal is true and the type is 
        (explicitly or implicitly) CPL_TYPE_FLOAT or CPL_TYPE_DOUBLE.

   To verify the presence of the MJD-OBS keyword:
   @code
       irplib_framelist_contains(myframes, "MJD-OBS", CPL_TYPE_INVALID,
                                 CPL_FALSE, 0.0);
   @endcode
  
   To verify that the EXPTIME is identical to within 0.1 millisecond:
   @code
       irplib_framelist_contains(myframes, "EXPTIME", CPL_TYPE_INVALID,
                                 CPL_TRUE, 0.0001);
   @endcode
  
   To verify that the keyword "ESO INS LAMP ST" is of type boolean and that
   it has the same value for all frames:
   @code
       irplib_framelist_contains(myframes, "ESO INS LAMP ST", CPL_TYPE_BOOL,
                                 CPL_TRUE, 0.0);
   @endcode


 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_framelist_contains(const irplib_framelist * self,
                                         const char * key, cpl_type type,
                                         cpl_boolean is_equal, double fp_tol)
{

    char * value_0 = NULL;
    char * value_i = NULL;
    cpl_type type_0 = CPL_TYPE_INVALID;
    int i, ifirst = -1;  /* First non-NULL propertylist */


    cpl_ensure_code(self  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(key   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(fp_tol >= 0.0, CPL_ERROR_ILLEGAL_INPUT);

    for (i=0; i < self->size; i++) {
        cpl_type type_i;


        if (self->propertylist[i] == NULL) continue;
        if (ifirst < 0) ifirst = i;

        type_i = cpl_propertylist_get_type(self->propertylist[i], key);

        if (type_i == CPL_TYPE_INVALID) {
            if (type == CPL_TYPE_INVALID)
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
                cpl_error_set_message(cpl_func, cpl_error_get_code(), "FITS "
                                      "key '%s' is missing from file %s", key,
                                      cpl_frame_get_filename(self->frame[i]));
            else
                cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                      "FITS key '%s' [%s] is missing from file "
                                      "%s", key, cpl_type_get_name(type),
                                      cpl_frame_get_filename(self->frame[i]));
#else
                cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                      "A FITS key is missing from a file");
            else
                cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                      "A FITS key is missing from a file");
#endif
            return cpl_error_get_code();
        }

        if (type != CPL_TYPE_INVALID && type_i != type) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            return cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                                         "FITS key '%s' has type %s instead of "
                                         "%s in file %s", key,
                                         cpl_type_get_name(type_i),
                                         cpl_type_get_name(type),
                                         cpl_frame_get_filename(self->frame[i]));
#else
            return cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                                         "A FITS key had an unexpected type");
#endif

        }

        if (!is_equal) continue;

        if (type_0 == CPL_TYPE_INVALID) {
            type_0 = type_i;
            continue;
        }

        if (type_i != type_0) {
            assert( type == CPL_TYPE_INVALID );
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            return cpl_error_set_message(cpl_func, CPL_ERROR_TYPE_MISMATCH,
                                         "FITS key '%s' has different types "
                                         "(%s <=> %s) in files %s and %s", key,
                                         cpl_type_get_name(type_0),
                                         cpl_type_get_name(type_i),
                                         cpl_frame_get_filename(self->frame[0]),
                                         cpl_frame_get_filename(self->frame[i]));
#else
            return cpl_error_set_message(cpl_func, CPL_ERROR_TYPE_MISMATCH,
                                         "A FITS key has different types in "
                                         "two files");
#endif
        }

        if (irplib_property_equal(self->propertylist[ifirst],
                                  self->propertylist[i],
                                  key, type_0, fp_tol, &value_0, &value_i))
            continue;

        if ((type_0 == CPL_TYPE_FLOAT || type_0 == CPL_TYPE_DOUBLE)
            && fp_tol > 0.0) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT, "FITS"
                                  " key '%s' [%s] has values that differ by "
                                  "more than %g (%s <=> %s) in files %s and %s",
                                  key, cpl_type_get_name(type_0), fp_tol,
                                  value_0, value_i,
                                  cpl_frame_get_filename(self->frame[0]),
                                  cpl_frame_get_filename(self->frame[i]));
#else
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT, "A "
                                  "FITS key has values that differ by more "
                                  "than the allowed tolerance in two file");
#endif
        } else {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                  "FITS key '%s' [%s] has different values "
                                  "(%s <=> %s) in files %s and %s", key,
                                  cpl_type_get_name(type_0),
                                  value_0, value_i,
                                  cpl_frame_get_filename(self->frame[0]),
                                  cpl_frame_get_filename(self->frame[i]));
#else
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT, "A "
                                  "FITS key has different values in two files");
#endif
        }
        cpl_free(value_0);
        cpl_free(value_i);

        return cpl_error_get_code();
    }        

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Load an imagelist from a framelist
  @param    self        The framelist
  @param    pixeltype   The required type of the pixels in the images
  @param    planenum    The (non-negative ) plane number 
  @param    extnum      The non-negative extension (0 for primary data unit)
  @return   The loaded list of images or NULL on error.
  @see      cpl_image_load()
  @note The returned cpl_imagelist must be deallocated using
        cpl_imagelist_delete()

 */
/*----------------------------------------------------------------------------*/
cpl_imagelist * irplib_imagelist_load_framelist(const irplib_framelist * self,
                                                cpl_type pixeltype,
                                                int planenum,
                                                int extnum)
{

    cpl_imagelist * list = NULL;
    cpl_image     * image = NULL;
    int i;


    cpl_ensure(self != NULL,  CPL_ERROR_NULL_INPUT,          NULL);
    cpl_ensure(extnum >= 0,   CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(planenum >= 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    list = cpl_imagelist_new();

    for (i=0; i < self->size; i++, image = NULL) {
        const char * filename = cpl_frame_get_filename(self->frame[i]);
        cpl_error_code code;

        if (filename == NULL) break;

        image = cpl_image_load(filename, pixeltype, planenum, extnum);
        if (image == NULL) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            (void)cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                        "Could not load FITS-image from plane "
                                        "%d in extension %d in file %s",
                                        planenum, extnum, filename);
#else
            (void)cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                        "Could not load FITS-image");
#endif
            break;
        }

        code = cpl_imagelist_set(list, image, i);
        if (code != CPL_ERROR_NONE) break; /* Should not be possible */
    }

    cpl_image_delete(image);
    
    if (cpl_imagelist_get_size(list) != self->size) {
        cpl_imagelist_delete(list);
        list = NULL;
        (void)cpl_error_set_where(cpl_func);
    }

    return list;

}


/**@}*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Set the size of the framelist - without handling any frames
  @param    self  The framelist to modify
  @return   void
  @note This private function should be used by all morphological functions;
        it will assert() on NULL input.

  This function updates the arrays used to hold the frames and propertylists
  to the size specified by the size member.

 */
/*----------------------------------------------------------------------------*/
static void irplib_framelist_set_size(irplib_framelist * self)
{


    assert( self != NULL);

    if (self->size == 0) {
        /* The list has been emptied */
        cpl_free(self->frame);
        cpl_free(self->propertylist);
        self->frame = NULL;
        self->propertylist = NULL;
    } else if (self->size > 0) {
        /* Update the size of the arrays */

        self->frame = cpl_realloc(self->frame, (size_t)self->size * sizeof(cpl_frame*));
        self->propertylist =
                        cpl_realloc(self->propertylist,
                                        (size_t)self->size * sizeof(cpl_propertylist*));
    } else {
        (void)cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
    }

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compare the value of two properties
  @param    self     The first property
  @param    other    The other property of the same type
  @param    fp_tol   The non-negative tolerance for floating point comparison
  @param    sstring  The string value of the first property (if they differ)
  @param    ostring  The string value of the other property (if they differ)
  @return   CPL_TRUE if equal, CPL_FALSE if not.
  @note This private function will assert() on illegal input.
        The names of the two properties are not compared.

  Iff the function returns CPL_FALSE *sstring and *ostring must be deallocated
  with cpl_free().

  The original content of *sstring and *ostring is not accessed.        

  FIXME: This function should really take two properties (and no key + type),
  but the cpl_propertylist module is flawed: It does not have
  an accessor by name... :-(
  The penalty for this is multiple propertylist searches by key
  (and loss of commutativity) :-(((

 */
/*----------------------------------------------------------------------------*/
static cpl_boolean irplib_property_equal(const cpl_propertylist * self,
                                         const cpl_propertylist * other,
                                         const char * key, cpl_type type,
                                         double fp_tol,
                                         char ** sstring, char ** ostring)
{

    cpl_boolean equal;


    assert(self    != NULL);
    assert(other   != NULL);
    assert(key     != NULL);
    assert(sstring != NULL);
    assert(ostring != NULL);

    /* FIXME: disable for better performance also with debugging */
    assert(cpl_propertylist_get_type(other, key) == type);
    assert(fp_tol >= 0.0);

    if (self == other) return CPL_TRUE;

    switch (type) {

    case CPL_TYPE_CHAR: {
        const char svalue = cpl_propertylist_get_char(self, key);
        const char ovalue = cpl_propertylist_get_char(other, key);

        equal = svalue == ovalue ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_sprintf("%c", svalue);
            *ostring = cpl_sprintf("%c", ovalue);
        }
        break;
    }

    case CPL_TYPE_BOOL: {
        const int svalue = cpl_propertylist_get_bool(self, key);
        const int ovalue = cpl_propertylist_get_bool(other, key);

        equal = svalue == ovalue ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_strdup(svalue == 0 ? "F" : "T");
            *ostring = cpl_strdup(ovalue == 0 ? "F" : "T");
        }
        break;
    }

    case CPL_TYPE_INT: {
        const int svalue = cpl_propertylist_get_int(self, key);
        const int ovalue = cpl_propertylist_get_int(other, key);

        equal = svalue == ovalue ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_sprintf("%d", svalue);
            *ostring = cpl_sprintf("%d", ovalue);
        }
        break;
    }

    case CPL_TYPE_LONG: {
        const long svalue = cpl_propertylist_get_long(self, key);
        const long ovalue = cpl_propertylist_get_long(other, key);

        equal = svalue == ovalue ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_sprintf("%ld", svalue);
            *ostring = cpl_sprintf("%ld", ovalue);
        }
        break;
    }

    case CPL_TYPE_FLOAT: {
        const double svalue = (double)cpl_propertylist_get_float(self, key);
        const double ovalue = (double)cpl_propertylist_get_float(other, key);

        equal = (fabs(svalue - ovalue) <= fp_tol) ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_sprintf("%f", svalue);
            *ostring = cpl_sprintf("%f", ovalue);
        }
        break;
    }

    case CPL_TYPE_DOUBLE: {
        const double svalue = cpl_propertylist_get_double(self, key);
        const double ovalue = cpl_propertylist_get_double(other, key);

        equal = (fabs(svalue - ovalue) <= fp_tol) ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_sprintf("%g", svalue);
            *ostring = cpl_sprintf("%g", ovalue);
        }
        break;
    }
    case CPL_TYPE_STRING: {
        const char * svalue = cpl_propertylist_get_string(self, key);
        const char * ovalue = cpl_propertylist_get_string(other, key);

        equal = strcmp(svalue, ovalue) == 0 ? CPL_TRUE : CPL_FALSE;
        if (!equal) {
            *sstring = cpl_strdup(svalue);
            *ostring = cpl_strdup(ovalue);
        }
        break;
    }
    default:
        /* Unknown property type */
#ifdef NDEBUG
        equal = CPL_FALSE;
#endif
        assert( 0 );

    }

    if (!equal) {
        assert( *sstring != NULL );
        assert( *ostring != NULL );
    }

    return equal;

}
