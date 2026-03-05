/*
 * hdrl_download-test.c
 *
 *  Created on: Jan 28, 2022
 *      Author: agabasch
 */

/*
 * This file is part of the HDRL Toolkit.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE - 0) < 500
#define _XOPEN_SOURCE 500 /* posix 2001, srandom */
#endif

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include <stdio.h> // for fseek
#include <string.h>
#include <stdlib.h>
#include "hdrl_utils.h"
#include "hdrl_download.h"

#define INVALID_HOSTNAME "notthere.invalid"

#define ESO_FTP_IP_ADDR "https://ftp.eso.org"


/*----------------------------------------------------------------------------*/
/**
 *                              Defines
 */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/**
 *                              Functions prototypes
 */
/*----------------------------------------------------------------------------*/



static cpl_error_code  hdrl_download_test(void)
{

  //Length of the raw retrieved data
  size_t data_length;
  char * raw_text = NULL;

  //Get how many tests have failed so far
  int test_failed_before = cpl_test_get_failed();

  raw_text = hdrl_download_url_to_buffer(
      ESO_FTP_IP_ADDR"/pub/dfs/pipelines/gravity/finals2000A.data", &data_length);

  //Check no cpl error is set
  cpl_test_error(CPL_ERROR_NONE);

  //Check there if the buffer is not empty
  cpl_test(data_length > 0);

  //Check pointer is not null
  cpl_test_nonnull(raw_text);

  /* Download file directly on disc and compare results */

  hdrl_download_url_to_file(ESO_FTP_IP_ADDR"/pub/dfs/pipelines/gravity/finals2000A.data",
			    "finals2000A.data");

  //Check no cpl error is set
  cpl_test_error(CPL_ERROR_NONE);

  // Open the file in read mode.
  FILE *file = fopen("finals2000A.data", "r");
  // Check if there was an error.

  cpl_test_nonnull(file);

  //If those tests passed (no more failed tests than before),
  //then continue with further tests on the retrieved data
  if(cpl_test_get_failed() == test_failed_before)
    {


      // Get the file length
      fseek(file, 0, SEEK_END);
      long length = ftell(file);
      fseek(file, 0, SEEK_SET);

      /*Compare the lenght of the buffer/file*/
      cpl_test_eq(length, data_length);

      // Create the string for the file contents.
      char *buffer = malloc(sizeof(char) * (length + 1));
      buffer[length] = '\0';
      // Set the contents of the string.
      size_t read = fread(buffer, sizeof(char), length, file);

      /* Compare the length of the two buffer to make sure that there was no
       * error when reading*/
      cpl_test_eq(read, data_length);

      int comp = memcmp(buffer, raw_text, data_length);

      cpl_test_eq(comp, 0);

      free(buffer);

      //the first characters of raw_text is a number
      char * endptr;
      int num = strtol(raw_text, &endptr, 10);
      cpl_test(num > 0);

      //Test conversion to a table
      cpl_table * eop_table =
	  hdrl_eop_data_totable (raw_text, data_length);

      //Test that the table has more than one row
      cpl_test(cpl_table_get_nrow(eop_table) > 0);

      //Test that PMX, PMY, DUT don't have nonsense values (corrections are small)
      cpl_test(cpl_table_get_column_max(eop_table, "PMX") < 10);
      cpl_test(cpl_table_get_column_min(eop_table, "PMX") > -10);
      cpl_test(cpl_table_get_column_max(eop_table, "PMY") < 10);
      cpl_test(cpl_table_get_column_min(eop_table, "PMY") > -10);
      cpl_test(cpl_table_get_column_max(eop_table, "DUT") < 10);
      cpl_test(cpl_table_get_column_min(eop_table, "DUT") > -10);

      //Test that MJD increases monotonically
      for(cpl_size i_row = 1; i_row < cpl_table_get_nrow(eop_table); i_row++)
	{
	  int null;
	  cpl_test(cpl_table_get_double(eop_table, "MJD", i_row, &null) >
	  cpl_table_get_double(eop_table, "MJD", i_row - 1, &null));
	}
      cpl_table_delete(eop_table);

      //Test conversion with NULL pointer
      eop_table = hdrl_eop_data_totable (NULL, data_length);
      cpl_test_error(CPL_ERROR_NULL_INPUT);
      cpl_test_null(eop_table);

      //Test with wrong data length
      eop_table = hdrl_eop_data_totable (raw_text, data_length - 1);
      cpl_test_error(CPL_ERROR_NULL_INPUT);
      cpl_test_null(eop_table);

      //Test with wrong data length and NULL pointer
      eop_table = hdrl_eop_data_totable (NULL, data_length - 1);
      cpl_test_error(CPL_ERROR_NULL_INPUT);
      cpl_test_null(eop_table);

      free(raw_text);

    }
  // Close the file.
  fclose(file);

  //Test with wrong HOST name
  hdrl_download_url_to_buffer(
      INVALID_HOSTNAME"/products/eop/rapid/standard/finals2000A.data", &data_length);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  //Test with wrong HOST name and PATH
  hdrl_download_url_to_buffer(INVALID_HOSTNAME"/invalid/path",
			      &data_length);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  //Test with wrong URL
  hdrl_download_url_to_buffer(ESO_FTP_IP_ADDR"/invalid/path", &data_length);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  //Test with NULL pointers
  hdrl_download_url_to_buffer(
      ESO_FTP_IP_ADDR"/products/eop/rapid/standard/finals2000A.data", NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  hdrl_download_url_to_buffer(NULL, &data_length);
  cpl_test_error(CPL_ERROR_NULL_INPUT);


  //Test with wrong URL
  hdrl_download_url_to_file(ESO_FTP_IP_ADDR"/invalid/path", "file.fits");
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  //Test with NULL pointers
  hdrl_download_url_to_file(
      ESO_FTP_IP_ADDR"/products/eop/rapid/standard/finals2000A.data", NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  hdrl_download_url_to_file(NULL, "file.fits");
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  //Test with wrong file
  hdrl_download_url_to_file(ESO_FTP_IP_ADDR"/pub/dfs/pipelines/gravity/finals2000A.data",
			    "/dev/null/finals2000A.data");
  cpl_test_error(CPL_ERROR_FILE_NOT_CREATED);


  /* Checking the code inside the CPL_MSG_DEBUG if - statement */
  cpl_msg_set_level(CPL_MSG_DEBUG);
  raw_text = hdrl_download_url_to_buffer(
      ESO_FTP_IP_ADDR"/pub/dfs/pipelines/gravity/finals2000A.data",
      &data_length);

  hdrl_download_url_to_file(ESO_FTP_IP_ADDR"/pub/dfs/pipelines/gravity/finals2000A.data",
			    "finals2000A.data");

  //Check no cpl error is set
  cpl_test_error(CPL_ERROR_NONE);
  free(raw_text);


  return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
 * @brief   Main function
 *
 * @return (int)cpl_error_code if error, CPL_ERROR_NONE (== 0) in other case
 *
 */
/*----------------------------------------------------------------------------*/

int main(void)
{
  cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

  hdrl_download_test();

  return cpl_test_end(0);
}








