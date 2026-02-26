         /*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include "hdrl_fpn.h"

#include "hdrl_random.h"
#include "hdrl_bpm_utils.h"

#include <cpl.h>
#include <complex.h>


/*----------------------------------------------------------------------------*/
/**
 *                              Defines and global variables
 */
/*----------------------------------------------------------------------------*/

#define HDRL_DELTA_COMPARE_QC               HDRL_EPS_DATA * 2.0e1
#define HDRL_DELTA_COMPARE_IMAGE            HDRL_EPS_DATA * 2.5e1


#define HDRL_FPN_TEST_EVEN_IMG_OUT          "even_img_out.fits"
#define HDRL_FPN_TEST_EVEN_MASK_OUT         "even_mask_out.fits"

#define HDRL_FPN_TEST_ODD_IMG_OUT           "odd_img_out.fits"
#define HDRL_FPN_TEST_ODD_MASK_OUT          "odd_mask_out.fits"

#define HDRL_FPN_TEST_FILTER_IMG_OUT        "filter_img_out.fits"
#define HDRL_FPN_TEST_FILTER_MASK_OUT       "filter_mask_out.fits"

#define HDRL_FPN_TEST_FILTER_MASK_IMG_OUT   "filter_with_mask_img_out.fits"
#define HDRL_FPN_TEST_FILTER_MASK_MASK_OUT  "filter_with_mask_mask_out.fits"


const double const_even_img[][4] = {
  {  0.84, -0.27,   0.07,  0.74 },
  {  0.57, -0.265, -0.07,  0.32 },
  {  0.25, -0.268,  0.07,  0.72 },
  { -0.9,  -0.2,   -0.05,  0.57 }
};

const double const_odd_img[][5] = {
  {  0.84, -0.27,   0.07,  0.74,  0.28 },
  { -1.2,  -0.255, -0.06,  0.65,  0.74 },
  { -1.5,  -0.25,   0.06,  0.64,  0.63 },
  { -0.84, -0.248, -0.06, -0.63,  0.56 },
  { -0.9,  -0.2,   -0.05,  0.57, -1.05 }
};

const double const_filter_img[][5] = {
  {  0.84, -0.27,   0.07,  0.74,  0.28 },
  {  0.57, -0.265, -0.07,  0.32,  0.37 },
  {  0.25, -0.268,  0.07,  0.72,  0.47 },
  { -1.2,  -0.255, -0.06,  0.65,  0.74 },
  { -1.5,  -0.25,   0.06,  0.64,  0.63 },
  { -0.84, -0.248, -0.06, -0.63,  0.56 },
  {  0.84, -0.236,  0.06,  0.59,  0.26 },
  {  0.94, -0.244, -0.06,  0.69, -0.16 },
  { -0.84, -0.23,   0.05,  0.43, -0.50 },
  { -0.9,  -0.2,   -0.05,  0.57, -1.05 }
};

const double const_even_img_out_python[][4] = {
  { 0.2827580625,      0.1036180625,       0.2962080625,       0.1036180625 },
  { 0.7368880625,      0.1449405625,       0.1099405625,       0.1804230625 },
  { 0.0200930625,      0.2151505625,       0.0874680625,       0.2151505625 },
  { 0.7368880625,      0.1804230625,       0.1099405625,       0.1449405625 }
};

const double const_odd_img_out_python[][5] = {
  { 0.12013156,        0.383159364837234,  0.265579755162766,  0.265579755162766,  0.383159364837234  },
  { 1.54915433534651,  0.400709353348878,  0.29934484816896,   0.369019554281073,  0.0991814702143874 },
  { 0.359162784653486, 0.467499565718927,  0.400883766651123,  0.0112476497856126, 0.72090627183104   },
  { 0.359162784653486, 0.72090627183104,   0.0112476497856126, 0.400883766651123,  0.467499565718927  },
  { 1.54915433534651,  0.0991814702143874, 0.369019554281073,  0.29934484816896,   0.400709353348878  }
};

const double const_filter_img_out_python[][10] = {
  { 0.08193152,        0.063075752708677,  0.822150095300018,  0.344980107291323,  0.0179478446999824, 0.28697888,         0.0179478446999824, 0.344980107291323,  0.822150095300018, 0.0630757527086773 },
  { 1.58489635987986,  0.220252697693417,  0.896622688396041,  0.0654115585573622, 0.0653615133387257, 0.0349653602187383, 0.0286088875424546, 0.0741165605524927, 0.633926559509007, 0.179705468727281  },
  { 0.089400680120138, 0.595806301442638,  0.211423052457545,  0.125587391272719,  0.156505251603959,  0.0308983997812617, 0.0250233804909936, 0.0969931623065833, 0.786391426661275, 0.722799299447507  },
  { 0.089400680120138, 0.722799299447507,  0.786391426661275,  0.0969931623065836, 0.0250233804909936, 0.0308983997812617, 0.156505251603959,  0.125587391272719,  0.211423052457545, 0.595806301442638  },
  { 1.58489635987986,  0.179705468727281,  0.633926559509007,  0.0741165605524925, 0.0286088875424546, 0.0349653602187383, 0.0653615133387257, 0.0654115585573621, 0.896622688396041, 0.220252697693417  }
};



/*----------------------------------------------------------------------------*/
/**
 * @defgroup cpl_fft_test  Testing cpl_fft function for pattern noise with iraf->powerspec() output
 *
 **/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @brief   Call to cpl and hdrl functions with a generated noise image
 *
 **/
/*----------------------------------------------------------------------------*/
static void hdrl_fpn_tests(void)
{
  /*** Create DUMMY images ***/

  /* Even image */
  cpl_size   even_img_nx = 4;
  cpl_size   even_img_ny = 4;
  cpl_image  *even_img   = cpl_image_new(even_img_nx, even_img_ny, CPL_TYPE_DOUBLE);
  for(cpl_size x = 0; x < even_img_nx; x++){
      for(cpl_size y = 0; y < even_img_ny; y++){
          cpl_image_set(even_img, x + 1, y + 1, const_even_img[x][y]);
      }
  }
  /* cpl_image_save(even_img, "even_img_in.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_CREATE); */

  /* Odd image */
  cpl_size   odd_img_nx = 5;
  cpl_size   odd_img_ny = 5;
  cpl_image  *odd_img   = cpl_image_new(odd_img_nx, odd_img_ny, CPL_TYPE_DOUBLE);
  for(cpl_size x = 0; x < odd_img_nx; x++){
      for(cpl_size y = 0; y < odd_img_ny; y++){
          cpl_image_set(odd_img, x + 1, y + 1, const_odd_img[x][y]);
      }
  }
  /* cpl_image_save(odd_img, "odd_img_in.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_CREATE); */

  /* Filter image */
  cpl_size  filter_img_nx   = 10;
  cpl_size  filter_img_ny   = 5;
  cpl_image *filter_img     = cpl_image_new(filter_img_nx,     filter_img_ny, CPL_TYPE_DOUBLE);
  for(cpl_size x = 0; x < filter_img_nx; x++){
      for(cpl_size y = 0; y < filter_img_ny; y++){
          cpl_image_set(filter_img, x + 1, y + 1, const_filter_img[x][y]);
      }
  }
  /* cpl_image_save(filter_img, "filter_img_in.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_CREATE); */


  /*** Output variables ***/
  cpl_image        *out_img = NULL;
  cpl_image        *out_mask;
  cpl_propertylist *qclist;
  double           rms;
  double           mad;
  int              null;


  /*** TESTS: Non-valid inputs ***/

  /* Null img_in */
  hdrl_fpn_compute(NULL, NULL, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* Bad dc_mask_x */
  hdrl_fpn_compute(filter_img, NULL, 0, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  /* Bad dc_mask_y */
  hdrl_fpn_compute(filter_img, NULL, 1, 0, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  /* Bad power_spectrum in the input */
  cpl_image *in_img_dummy  = cpl_image_new(2, 2, CPL_TYPE_DOUBLE);
  hdrl_fpn_compute(filter_img, NULL, 1, 1, &in_img_dummy, &rms, &mad);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_image_delete(in_img_dummy);

  /* Bad img_in because contain reject pixels */
  cpl_image *filter_img_reject = cpl_image_duplicate(filter_img);
  cpl_image_reject(filter_img_reject, 1, 1);
  hdrl_fpn_compute(filter_img_reject, NULL, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_image_delete(filter_img_reject);

  /* Bad size in the input mask */
  cpl_mask *mask_wrong_img = cpl_mask_new(filter_img_nx - 1, filter_img_ny - 1);
  hdrl_fpn_compute(filter_img, mask_wrong_img, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_mask_delete(mask_wrong_img);


  /*** TEST: Even image ***/
  out_img = NULL;
  hdrl_fpn_compute(even_img, NULL, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(out_img);

  /* Test files with python output (See python fpn code at the end of this function) */
  out_mask = cpl_image_new_from_mask(cpl_image_get_bpm(out_img));
  qclist   = cpl_propertylist_new();
  cpl_propertylist_update_double(qclist, "ESO QC FPN RMS", rms);
  cpl_propertylist_update_double(qclist, "ESO QC FPN MAD", mad);
  cpl_image_save(out_img,  HDRL_FPN_TEST_EVEN_IMG_OUT,  CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);
  cpl_image_save(out_mask, HDRL_FPN_TEST_EVEN_MASK_OUT, CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);

  /* cpl_msg_info(cpl_func, "EVEN image -> rms=%g, mad=%g", rms, mad); */
  cpl_test_rel(rms, 0.217609641787739, HDRL_DELTA_COMPARE_QC);
  cpl_test_rel(mad, 0.0612647385,      HDRL_DELTA_COMPARE_QC);

  /* Test output values with python */
  for(cpl_size x = 0; x < even_img_nx; x++){
      for(cpl_size y = 0; y < even_img_ny; y++){
          if (!cpl_image_get(out_mask, x + 1, y + 1, &null)) {
              cpl_test_rel(cpl_image_get(out_img, x + 1, y + 1, &null), const_even_img_out_python[y][x], HDRL_DELTA_COMPARE_IMAGE);
          }
      }
  }

  cpl_image_delete(        out_img   );
  cpl_image_delete(        out_mask  );
  cpl_propertylist_delete( qclist    );
  cpl_image_delete(        even_img  );


  /*** TEST: Odd image ***/
  out_img = NULL;
  hdrl_fpn_compute(odd_img, NULL, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(out_img);

  /* Test files with python output (See python fpn code at the end of this function) */
  out_mask = cpl_image_new_from_mask(cpl_image_get_bpm(out_img));
  qclist   = cpl_propertylist_new();
  cpl_propertylist_update_double(qclist, "ESO QC FPN RMS", rms);
  cpl_propertylist_update_double(qclist, "ESO QC FPN MAD", mad);
  cpl_image_save(out_img,  HDRL_FPN_TEST_ODD_IMG_OUT,  CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);
  cpl_image_save(out_mask, HDRL_FPN_TEST_ODD_MASK_OUT, CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);

  /* cpl_msg_info(cpl_func, "ODD image -> rms=%g, mad=%g", rms, mad); */
  cpl_test_rel(rms, 0.381960894533284, HDRL_DELTA_COMPARE_QC);
  cpl_test_rel(mad, 0.124653092119791, HDRL_DELTA_COMPARE_QC);

  /* Test output values with python */
  for(cpl_size x = 0; x < odd_img_nx; x++){
      for(cpl_size y = 0; y < odd_img_ny; y++){
          if (!cpl_image_get(out_mask, x + 1, y + 1, &null)) {
              cpl_test_rel(cpl_image_get(out_img, x + 1, y + 1, &null), const_odd_img_out_python[y][x], HDRL_DELTA_COMPARE_IMAGE);
          }
      }
  }

  cpl_image_delete(        out_img  );
  cpl_image_delete(        out_mask );
  cpl_propertylist_delete( qclist   );
  cpl_image_delete(        odd_img  );



  /*** TEST: filter image ***/
  out_img = NULL;
  hdrl_fpn_compute(filter_img, NULL, 3, 3, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(out_img);

  /* Duplicate mask for the TEST: filter image with mask */
  cpl_mask *filter_mask = cpl_mask_duplicate(cpl_image_get_bpm(out_img));

  /* Test files with python output (See python fpn code at the end of this function) */
  out_mask = cpl_image_new_from_mask(cpl_image_get_bpm(out_img));
  qclist = cpl_propertylist_new();
  cpl_propertylist_update_double(qclist, "ESO QC FPN RMS", rms);
  cpl_propertylist_update_double(qclist, "ESO QC FPN MAD", mad);
  cpl_image_save(out_img,  HDRL_FPN_TEST_FILTER_IMG_OUT,  CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);
  cpl_image_save(out_mask, HDRL_FPN_TEST_FILTER_MASK_OUT, CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);

  /* cpl_msg_info(cpl_func, "FILTER image -> rms=%g, mad=%g", rms, mad); */
  cpl_test_rel(rms, 0.346571043362885, HDRL_DELTA_COMPARE_QC);
  cpl_test_rel(mad, 0.140385898785234, HDRL_DELTA_COMPARE_QC);

  /* Test output values with python */
  for(cpl_size x = 0; x < filter_img_nx; x++){
      for(cpl_size y = 0; y < filter_img_ny; y++){
          if (!cpl_image_get(out_mask, x + 1, y + 1, &null)) {
              cpl_test_rel(cpl_image_get(out_img, x + 1, y + 1, &null), const_filter_img_out_python[y][x], HDRL_DELTA_COMPARE_IMAGE);
          }
      }
  }

  cpl_image_delete(        out_img        );
  cpl_image_delete(        out_mask       );
  cpl_propertylist_delete( qclist         );


  /*** TEST: filter image with mask ***/
  out_img = NULL;
  cpl_mask_set(filter_mask, 1, 1, CPL_BINARY_0);  /* Unset the peak */
  hdrl_fpn_compute(filter_img, filter_mask, 1, 1, &out_img, &rms, &mad);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(out_img);

  /* Test files with python output (See python fpn code at the end of this function) */
  const cpl_mask *out_filter_mask = cpl_image_get_bpm(out_img);
  out_mask = cpl_image_new_from_mask(out_filter_mask);
  qclist = cpl_propertylist_new();
  cpl_propertylist_update_double(qclist, "ESO QC FPN RMS", rms);
  cpl_propertylist_update_double(qclist, "ESO QC FPN MAD", mad);
  cpl_image_save(out_img,  HDRL_FPN_TEST_FILTER_MASK_IMG_OUT,  CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);
  cpl_image_save(out_mask, HDRL_FPN_TEST_FILTER_MASK_MASK_OUT, CPL_TYPE_DOUBLE, qclist, CPL_IO_CREATE);

  /* cpl_msg_info(cpl_func, "FILTER image -> rms=%g, mad=%g", rms, mad); */
  cpl_test_rel(rms, 0.346571043362885, HDRL_DELTA_COMPARE_QC);
  cpl_test_rel(mad, 0.140385898785234, HDRL_DELTA_COMPARE_QC);

  /* Test output values with python */
  for(cpl_size x = 0; x < filter_img_nx; x++){
      for(cpl_size y = 0; y < filter_img_ny; y++){
          if (!cpl_image_get(out_mask, x + 1, y + 1, &null)) {
              cpl_test_rel(cpl_image_get(out_img, x + 1, y + 1, &null), const_filter_img_out_python[y][x], HDRL_DELTA_COMPARE_IMAGE);
          }
      }
  }

  /* Test output mask */
  for(cpl_size x = 1; x <= filter_img_nx; x++){
      for(cpl_size y = 1; y <= filter_img_ny; y++){
          cpl_binary bpm_pixel = cpl_mask_get(out_filter_mask, x, y);
          if (x <= 3 && y <= 3) {
              cpl_test_eq(bpm_pixel, CPL_BINARY_1);
          } else {
              cpl_test_eq(bpm_pixel, CPL_BINARY_0);
          }
      }
  }

  /* Cleanup */
  cpl_image_delete(        out_img        );
  cpl_image_delete(        out_mask       );
  cpl_propertylist_delete( qclist         );
  cpl_mask_delete(         filter_mask    );
  cpl_image_delete(        filter_img     );

  /* Remove files */
  cpl_test_zero(remove(HDRL_FPN_TEST_EVEN_IMG_OUT        ));
  cpl_test_zero(remove(HDRL_FPN_TEST_EVEN_MASK_OUT       ));
  cpl_test_zero(remove(HDRL_FPN_TEST_ODD_IMG_OUT         ));
  cpl_test_zero(remove(HDRL_FPN_TEST_ODD_MASK_OUT        ));
  cpl_test_zero(remove(HDRL_FPN_TEST_FILTER_IMG_OUT      ));
  cpl_test_zero(remove(HDRL_FPN_TEST_FILTER_MASK_OUT     ));
  cpl_test_zero(remove(HDRL_FPN_TEST_FILTER_MASK_IMG_OUT ));
  cpl_test_zero(remove(HDRL_FPN_TEST_FILTER_MASK_MASK_OUT));


/*
 * Python code to crosscheck the output power spectrum files
 * =========================================================

import numpy as np
import math
import pyfits
import os
import glob

feven=pyfits.open("even_img_in.fits")
fodd=pyfits.open("odd_img_in.fits")
ffilter=pyfits.open("filter_img_in.fits")

pseven = np.abs(np.fft.fft2(feven[0].data))**2
psodd = np.abs(np.fft.fft2(fodd[0].data))**2
psfilter = np.abs(np.fft.fft2(ffilter[0].data))**2

pseven /= pseven.size
psodd /= psodd.size
psfilter /= psfilter.size

feven[0].data=pseven
fodd[0].data=psodd
ffilter[0].data=psfilter

feven[0].writeto("even_img_out_python.fits", clobber=True)
fodd[0].writeto("odd_img_out_python.fits", clobber=True)
ffilter[0].writeto("filter_img_out_python.fits", clobber=True)

*/

}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Unit tests of Fixed pattern noise module
 *
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
  cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);


  /* Tests with noise files */
  hdrl_fpn_tests();


  cpl_test_error(CPL_ERROR_NONE);

  return cpl_test_end(0);
}
