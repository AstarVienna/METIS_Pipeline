/*
 * hdrl_download.c
 *
 *  Created on: Jan 28, 2022
 *      Author: agabasch
 */

/*
 * This file is part of HDRL
 * Copyright (C) 2022 European Southern Observatory
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

/*-----------------------------------------------------------------------------
                                  Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <cpl.h>
#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_download   Download from the Internet.

  @brief This module contains functions to download information from
  the internet. It is based on libcurl (the multiprotocol file
  transfer library), a free and easy-to-use client-side URL transfer
  library. For detailed informations see https://curl.se/libcurl

 \verbatim

 ***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2021, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************
\endverbatim 

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2021, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/




/* <DESC>
 * Shows how the write callback function can be used to download data into a
 * chunk of memory instead of storing it in a file.
 * </DESC>
 */
struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
      /* out of memory! */
      cpl_error_set_message(cpl_func, CPL_ERROR_UNSPECIFIED,
			    "Not enough memory (realloc returned NULL)");
      /*printf("not enough memory (realloc returned NULL)\n");*/
      return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}




/*----------------------------------------------------------------------------*/
/**
  @brief    Downloads a url into a c data buffer
  @param    url          The url to download from
  @param    data_length  The length of the downloaded data buffer

  @return   The downloaded data buffer or NULL in case of error.

  For the supported protocols please see the https://curl.se/libcurl

  \warning This function is not threadsafe, to the extent that it may only be
  called from the main thread, with no other threads running. So as long as
  esorex or alike is not using threads it still may be called from within a
  recipe before the the recipe itself, or hdrl launches any additional threads.

 */
/*----------------------------------------------------------------------------*/
char * hdrl_download_url_to_buffer (const char * url, size_t * data_length)
{

  /* Check and dump the input */
  cpl_ensure (url,    CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure (data_length, CPL_ERROR_NULL_INPUT, NULL);

  cpl_msg_debug (cpl_func, "Using URL %s", url);


  /* Writing into a data buffer */

  CURL *curl_handle;
  CURLcode res;
  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /*
   * A long parameter set to 1 tells the library to fail the request if the HTTP
   *  code returned is equal to or larger than 400. The default action would be
   *  to return the page normally, ignoring that code.
   *  This method is not fail-safe and there are occasions where non-successful
   *  response codes will slip through, especially when authentication is
   *  involved(response codes 401 and 407).
   */
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);

  if(cpl_msg_get_level() == CPL_MSG_DEBUG){
      /* Switch on full protocol/debug output while testing */
      curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
      /* disable progress meter, set to 0L to enable it */
      curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  }

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers do not like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
      /*
      fprintf(stderr, "libcurl failed to retrieve: %s\n", curl_easy_strerror(res));
       */
      cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
			    "Could not retrieve data: %s",curl_easy_strerror(res));
      curl_easy_cleanup(curl_handle);
      curl_global_cleanup();
      free(chunk.memory);
      return NULL;
  }
  else {
      /*
       * Now, our chunk.memory points to a memory block that is chunk.size
       * bytes big and contains the remote file.
       *
       * Do something nice with it!
       */

      *data_length =  chunk.size;
      curl_easy_cleanup(curl_handle);
      curl_global_cleanup();
      return chunk.memory;
   }

 }

/* <DESC>
 * Download a given URL into a local file.
 * </DESC>
 */

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Downloads a url into a file on disc
  @param    url       The url to download from
  @param    filename  The name of the file where the url is downloaded

  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise

  For the supported protocols please see the https://curl.se/libcurl

  \warning This function is not threadsafe, to the extent that it may only be
  called from the main thread, with no other threads running. So as long as
  esorex or alike is not using threads it still may be called from within a
  recipe before the the recipe itself, or hdrl launches any additional threads.


 */
/*----------------------------------------------------------------------------*/

cpl_error_code
hdrl_download_url_to_file (const char * url, const char * filename)
{
  /* see also https://curl.se/libcurl/c/url2file.html */

  /* Check and dump the input */
  cpl_ensure_code(url,          CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(filename, CPL_ERROR_NULL_INPUT);

  cpl_msg_debug (cpl_func, "Using URL %s", url);
  cpl_msg_debug (cpl_func, "Using File %s", filename);

  CURL *curl_handle;
  CURLcode res;
  FILE *pagefile;

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /*
   * A long parameter set to 1 tells the library to fail the request if the HTTP
   *  code returned is equal to or larger than 400. The default action would be
   *  to return the page normally, ignoring that code.
   *  This method is not fail-safe and there are occasions where non-successful
   *  response codes will slip through, especially when authentication is
   *  involved(response codes 401 and 407).
   */
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);

  if(cpl_msg_get_level() == CPL_MSG_DEBUG){
      /* Switch on full protocol/debug output while testing */
      curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
      /* disable progress meter, set to 0L to enable it */
      curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  }

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

  /* open the file */
  pagefile = fopen(filename, "wb");

  if(pagefile) {

      /* write the page body to this file handle */
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

      /* get it! */
      res = curl_easy_perform(curl_handle);

      if(res != CURLE_OK) {
	  curl_easy_cleanup(curl_handle);
	  curl_global_cleanup();
	  /* close the header file */
	  fclose(pagefile);
	  return  cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
					"Could not retrieve data: %s",
					curl_easy_strerror(res));
      }

      /* close the header file */
      fclose(pagefile);
  } else {
      /* cleanup curl stuff */
      curl_easy_cleanup(curl_handle);
      curl_global_cleanup();
      return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_NOT_CREATED,
				   "The file %s could not be created",
				   filename);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();

  return cpl_error_get_code();
}


/**@} */
