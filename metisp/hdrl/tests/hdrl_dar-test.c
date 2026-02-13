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

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/
#include "hdrl_dar.h"

#include <cpl.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_dar_test    Testing of the HDRL DAR
 */
/*----------------------------------------------------------------------------*/

/* maximum number of wavelength planes = pixels in table to expect */
#define NMAX 300

/* standard star 1: BD+75d325 at airmass ~ 1.27922 (run86_00131b.fits) */
const float kStd1[][5] = { /* every 10th wavelength plane of the centroid */
  { 3737.84, -0.27, 0.007, 0.774, 0.008 },
  { 3770.84, -0.265, 0.007, 0.732, 0.007 },
  { 3803.84, -0.268, 0.007, 0.702, 0.007 },
  { 3836.84, -0.255, 0.006, 0.665, 0.007 },
  { 3869.84, -0.25, 0.006, 0.644, 0.006 },
  { 3902.84, -0.248, 0.006, 0.613, 0.006 },
  { 3935.84, -0.236, 0.006, 0.589, 0.006 },
  { 3968.84, -0.244, 0.006, 0.569, 0.006 },
  { 4001.84, -0.23, 0.005, 0.543, 0.005 },
  { 4034.84, -0.19, 0.005, 0.517, 0.005 },
  { 4067.84, -0.2, 0.005, 0.507, 0.005 },
  { 4100.84, -0.207, 0.005, 0.493, 0.005 },
  /* { 4133.84, -0.141, 0.005, 0.485, 0.005 }, high x values */
  /* { 4166.84, -0.131, 0.005, 0.479, 0.005 }, high x values */
  { 4199.84, -0.186, 0.005, 0.444, 0.005 },
  { 4232.84, -0.162, 0.005, 0.417, 0.005 },
  { 4265.84, -0.166, 0.005, 0.405, 0.005 },
  { 4298.84, -0.17, 0.005, 0.39, 0.005 },
  { 4331.84, -0.163, 0.005, 0.368, 0.005 },
  { 4364.84, -0.112, 0.005, 0.346, 0.005 },
  { 4397.84, -0.148, 0.005, 0.34, 0.005 },
  { 4430.84, -0.145, 0.005, 0.325, 0.005 },
  { 4463.84, -0.131, 0.005, 0.308, 0.005 },
  { 4496.84, -0.104, 0.005, 0.285, 0.005 },
  { 4529.84, -0.118, 0.005, 0.276, 0.005 },
  { 4562.84, -0.093, 0.005, 0.253, 0.005 },
  /* { 4595.84, -0.069, 0.005, 0.306, 0.005 }, high y value */
  { 4628.84, -0.085, 0.005, 0.236, 0.005 },
  { 4661.84, -0.072, 0.005, 0.219, 0.005 },
  { 4694.84, -0.1, 0.005, 0.213, 0.005 },
  { 4727.84, -0.072, 0.005, 0.191, 0.005 },
  { 4760.84, -0.082, 0.005, 0.183, 0.005 },
  { 4793.84, -0.074, 0.005, 0.171, 0.005 },
  { 4826.84, -0.061, 0.005, 0.16, 0.005 },
  { 4859.84, -0.057, 0.005, 0.145, 0.005 },
  /* { 4892.84, 0, 0.005, 0.119, 0.005 }, high x value */
  { 4925.84, -0.033, 0.005, 0.117, 0.005 },
  { 4958.84, -0.029, 0.005, 0.099, 0.005 },
  { 4991.84, -0.028, 0.005, 0.091, 0.005 },
  { 5024.84, -0.031, 0.005, 0.08, 0.005 },
  { 5057.84, -0.029, 0.005, 0.07, 0.005 },
  { 5090.84, -0.021, 0.005, 0.055, 0.005 },
  { 5123.84, -0.021, 0.005, 0.054, 0.005 },
  { 5156.84, -0.017, 0.005, 0.036, 0.005 },
  { 5189.84, -0.012, 0.005, 0.028, 0.005 },
  { 5222.84, -0.009, 0.005, 0.018, 0.005 },
  { 5255.84, -0.004, 0.005, 0.008, 0.005 },
  { 5288.84, 0.002, 0.005, -0.008, 0.005 },
  { 5321.84, 0.003, 0.005, -0.009, 0.005 },
  { 5354.84, 0.011, 0.005, -0.025, 0.005 },
  { 5387.84, 0.011, 0.005, -0.033, 0.005 },
  /* { 5420.84, -0.01, 0.005, -0.036, 0.005 }, low x value */
  { 5453.84, 0.031, 0.005, -0.057, 0.005 },
  { 5486.84, 0.021, 0.005, -0.068, 0.005 },
  { 5519.84, 0.028, 0.005, -0.077, 0.005 },
  { 5552.84, 0.035, 0.005, -0.088, 0.005 },
  { 5585.84, 0.035, 0.005, -0.092, 0.005 },
  { 5618.84, 0.048, 0.005, -0.106, 0.005 },
  { 5651.84, 0.054, 0.005, -0.113, 0.005 },
  { 5684.84, 0.06, 0.005, -0.118, 0.005 },
  { 5717.84, 0.066, 0.005, -0.13, 0.005 },
  { 5750.84, 0.063, 0.005, -0.131, 0.005 },
  { 5783.84, 0.067, 0.005, -0.136, 0.005 },
  { 5816.84, 0.063, 0.005, -0.148, 0.005 },
  { 5849.84, 0.064, 0.005, -0.153, 0.005 },
  { 5882.84, 0.062, 0.005, -0.162, 0.005 },
  { 5915.84, 0.081, 0.005, -0.174, 0.005 },
  /* { 5948.84, 0.066, 0.005, -0.194, 0.006 }, low y value */
  { 5981.84, 0.091, 0.006, -0.187, 0.006 },
  { 6014.84, 0.095, 0.006, -0.193, 0.006 },
  { 6047.84, 0.094, 0.006, -0.193, 0.006 },
  { 6080.84, 0.087, 0.006, -0.201, 0.006 },
  { 6113.84, 0.089, 0.006, -0.21, 0.006 },
  { 6146.84, 0.103, 0.006, -0.222, 0.006 },
  { 6179.84, 0.114, 0.006, -0.225, 0.006 },
  { 6212.84, 0.114, 0.006, -0.23, 0.006 },
  { 6245.84, 0.101, 0.006, -0.235, 0.006 },
  { 6278.84, 0.093, 0.006, -0.239, 0.006 },
  { 6311.84, 0.11, 0.006, -0.247, 0.006 },
  { 6344.84, 0.119, 0.006, -0.253, 0.006 },
  { 6377.84, 0.119, 0.006, -0.253, 0.006 },
  { 6410.84, 0.107, 0.006, -0.257, 0.006 },
  { 6443.84, 0.113, 0.006, -0.267, 0.006 },
  { 6476.84, 0.124, 0.006, -0.276, 0.006 },
  { 6509.84, 0.125, 0.007, -0.277, 0.006 },
  { 6542.84, 0.111, 0.007, -0.28, 0.007 },
  /* { 6575.84, 0.147, 0.007, -0.297, 0.007 }, high x value */
  { 6608.84, 0.132, 0.007, -0.294, 0.007 },
  { 6641.84, 0.13, 0.007, -0.295, 0.007 },
  { 6674.84, 0.118, 0.007, -0.299, 0.007 },
  { 6707.84, 0.129, 0.007, -0.311, 0.007 },
  { 6740.84, 0.138, 0.007, -0.316, 0.007 },
  { 6773.84, 0.134, 0.007, -0.316, 0.007 },
  { 6806.84, 0.126, 0.007, -0.325, 0.007 },
  { 6839.84, 0.138, 0.007, -0.325, 0.007 },
  /* { 6872.84, 0.199, 0.008, -0.334, 0.008 }, high x value */
  { 6905.84, 0.15, 0.008, -0.333, 0.008 },
  { 6938.84, 0.142, 0.007, -0.332, 0.008 },
  { 0.0,     0.0,   0.0,   0.0,    0.0   }
};

/* standard star 2: bd332642 at airmass ~ 1.02331 (run86_00175b.fits) */
const float kStd2[][5] = { /* every 10th wavelength plane of the centroid */
  /* { 3736.2, 0.156, 0.010, -0.049, 0.012 }, weird values */
  /* { 3769.2, 0.337, 0.009, -0.124, 0.011 }, noise?! */
  /* { 3802.2, 0.263, 0.009, -0.108, 0.010 }, noise?! */
  /* { 3835.2, 0.329, 0.009, -0.115, 0.010 }, noise?! */
  { 3868.2, 0.184, 0.008, -0.093, 0.009 },
  { 3901.2, 0.193, 0.008, -0.089, 0.009 },
  { 3934.2, 0.182, 0.008, -0.088, 0.009 },
  /* { 3967.2, 0.046, 0.008, -0.058, 0.009 }, low x value */
  { 4000.2, 0.154, 0.007, -0.075, 0.008 },
  /* { 4033.2, 0.188, 0.007, -0.087, 0.008 }, high x value */
  { 4066.2, 0.138, 0.007, -0.061, 0.008 },
  /* { 4099.2, 0.074, 0.007, -0.051, 0.008 }, low x value */
  /* { 4132.2, 0.17, 0.007, -0.065, 0.008 }, weird values */
  /* { 4165.2, 0.16, 0.006, -0.056, 0.008 }, weird values */
  { 4198.2, 0.131, 0.007, -0.059, 0.007 },
  { 4231.2, 0.112, 0.007, -0.055, 0.008 },
  { 4264.2, 0.102, 0.007, -0.049, 0.007 },
  { 4297.2, 0.101, 0.007, -0.054, 0.007 },
  { 4330.2, 0.097, 0.007, -0.052, 0.007 },
  /* { 4363.2, 0.149, 0.007, -0.063, 0.008 }, weird values */
  /* { 4396.2, 0.059, 0.007, -0.043, 0.007 }, weird values */
  { 4429.2, 0.089, 0.007, -0.046, 0.007 },
  { 4462.2, 0.088, 0.006, -0.045, 0.007 },
  { 4495.2, 0.098, 0.006, -0.042, 0.007 },
  { 4528.2, 0.07, 0.006, -0.037, 0.007 },
  { 4561.2, 0.062, 0.006, -0.034, 0.007 },
  { 4594.2, 0.073, 0.006, -0.044, 0.007 },
  { 4627.2, 0.067, 0.006, -0.04, 0.007 },
  { 4660.2, 0.055, 0.006, -0.037, 0.007 },
  { 4693.2, 0.054, 0.006, -0.027, 0.007 },
  { 4726.2, 0.036, 0.006, -0.024, 0.007 },
  { 4759.2, 0.044, 0.006, -0.026, 0.007 },
  { 4792.2, 0.045, 0.006, -0.024, 0.007 },
  { 4825.2, 0.038, 0.006, -0.027, 0.007 },
  { 4858.2, 0.04, 0.006, -0.023, 0.007 },
  /* { 4891.2, 0.094, 0.006, -0.034, 0.007 }, weird values */
  { 4924.2, 0.031, 0.006, -0.013, 0.007 },
  { 4957.2, 0.029, 0.006, -0.013, 0.007 },
  { 4990.2, 0.021, 0.006, -0.012, 0.007 },
  { 5023.2, 0.01, 0.006, -0.002, 0.007 },
  { 5056.2, 0.017, 0.006, -0.003, 0.007 },
  { 5089.2, 0.019, 0.006, -0.003, 0.007 },
  { 5122.2, 0.015, 0.006, -0.006, 0.007 },
  { 5155.2, 0.016, 0.006, -0.004, 0.007 },
  { 5188.2, 0.007, 0.006, -0.004, 0.007 },
  { 5221.2, 0.008, 0.006, -0.002, 0.007 },
  { 5254.2, 0.005, 0.006, 0, 0.007 },
  { 5287.2, 0.001, 0.006, 0.001, 0.007 },
  { 5320.2, -0.002, 0.006, 0.006, 0.007 },
  { 5353.2, -0.002, 0.006, 0.005, 0.007 },
  { 5386.2, -0.008, 0.006, 0.007, 0.007 },
  { 5419.2, -0.013, 0.006, 0.007, 0.007 },
  { 5452.2, -0.014, 0.006, 0.006, 0.007 },
  { 5485.2, -0.019, 0.006, 0.011, 0.007 },
  { 5518.2, -0.017, 0.006, 0.007, 0.007 },
  { 5551.2, -0.02, 0.007, 0.013, 0.007 },
  { 5584.2, -0.03, 0.007, 0.013, 0.007 },
  { 5617.2, -0.028, 0.007, 0.012, 0.007 },
  { 5650.2, -0.034, 0.007, 0.01, 0.007 },
  { 5683.2, -0.034, 0.007, 0.009, 0.007 },
  { 5716.2, -0.031, 0.007, 0.011, 0.007 },
  /* { 5749.2, 0.059, 0.008, 0.04, 0.007 }, high values */
  { 5782.2, -0.031, 0.007, 0.016, 0.007 },
  { 5815.2, -0.03, 0.007, 0.02, 0.007 },
  { 5848.2, -0.03, 0.007, 0.005, 0.007 },
  { 5881.2, -0.044, 0.007, 0.022, 0.007 },
  { 5914.2, -0.029, 0.007, 0.023, 0.007 },
  { 5947.2, -0.04, 0.007, 0.01, 0.007 },
  { 5980.2, -0.037, 0.007, 0.019, 0.007 },
  { 6013.2, -0.035, 0.007, 0.015, 0.008 },
  { 6046.2, -0.034, 0.007, 0.019, 0.008 },
  { 6079.2, -0.037, 0.007, 0.014, 0.008 },
  { 6112.2, -0.041, 0.007, 0.024, 0.008 },
  { 6145.2, -0.043, 0.007, 0.02, 0.008 },
  { 6178.2, -0.044, 0.007, 0.02, 0.008 },
  { 6211.2, -0.043, 0.007, 0.021, 0.008 },
  { 6244.2, -0.046, 0.007, 0.025, 0.008 },
  { 6277.2, -0.055, 0.007, 0.025, 0.008 },
  { 6310.2, -0.049, 0.008, 0.027, 0.008 },
  { 6343.2, -0.052, 0.008, 0.02, 0.008 },
  { 6376.2, -0.048, 0.008, 0.022, 0.008 },
  { 6409.2, -0.054, 0.008, 0.025, 0.008 },
  { 6442.2, -0.064, 0.008, 0.028, 0.008 },
  { 6475.2, -0.068, 0.008, 0.027, 0.009 },
  { 6508.2, -0.058, 0.008, 0.022, 0.009 },
  { 6541.2, -0.057, 0.008, 0.029, 0.009 },
  /* { 6574.2, -0.089, 0.009, 0.031, 0.009 }, low x value */
  { 6607.2, -0.068, 0.008, 0.034, 0.009 },
  { 6640.2, -0.066, 0.008, 0.025, 0.009 },
  /* { 6673.2, -0.091, 0.009, 0.028, 0.009 }, low x value */
  { 6706.2, -0.069, 0.009, 0.028, 0.009 },
  { 6739.2, -0.07, 0.009, 0.03, 0.009 },
  { 6772.2, -0.065, 0.009, 0.034, 0.009 },
  { 6805.2, -0.075, 0.009, 0.034, 0.009 },
  { 6838.2, -0.079, 0.009, 0.026, 0.009 },
  /* { 6871.2, -0.002, 0.010, 0.016, 0.011 }, high x value */
  { 6904.2, -0.057, 0.009, 0.032, 0.010 },
  { 6937.2, -0.075, 0.009, 0.034, 0.010 },
  { 0.0,     0.0,   0.0,   0.0,    0.0  }
};

/* imcentroid of star in Pre-Dry-Run_001_001_002_float.fits */
const float kMuse1[][5] = { /* peak centroid shifts of three objects */
  { 4651.657, -6.732, 0.499, -1.614, 0.500 },
  { 4701.657, -6.337, 0.477, -1.624, 0.463 },
  { 4714.157, -6.311, 0.470, -1.569, 0.460 },
  { 4726.657, -6.309, 0.465, -1.566, 0.465 },
  { 4739.157, -6.124, 0.475, -1.560, 0.469 },
  { 4751.657, -6.108, 0.471, -1.503, 0.466 },
  { 4764.157, -6.118, 0.474, -1.500, 0.472 },
  { 4776.657, -5.966, 0.468, -1.499, 0.467 },
  { 4789.157, -5.958, 0.492, -1.481, 0.486 },
  { 4801.657, -5.961, 0.465, -1.490, 0.463 },
  { 4814.157, -5.854, 0.460, -1.566, 0.448 },
  { 4826.657, -5.801, 0.461, -1.491, 0.462 },
  { 4839.157, -5.807, 0.460, -1.435, 0.462 },
  { 4851.657, -5.788, 0.450, -1.527, 0.461 },
  { 4864.157, -5.619, 0.464, -1.503, 0.457 },
  { 4876.657, -5.591, 0.467, -1.379, 0.470 },
  { 4889.157, -5.624, 0.475, -1.426, 0.475 },
  { 4901.657, -5.432, 0.450, -1.418, 0.452 },
  { 4914.157, -5.390, 0.452, -1.420, 0.459 },
  { 4926.657, -5.361, 0.462, -1.262, 0.463 },
  { 4939.157, -5.375, 0.466, -1.313, 0.439 },
  { 4951.657, -5.170, 0.443, -1.384, 0.448 },
  { 4964.157, -5.165, 0.451, -1.295, 0.452 },
  { 4976.657, -5.172, 0.456, -1.295, 0.463 },
  { 4989.157, -5.153, 0.469, -1.317, 0.469 },
  { 5001.657, -4.976, 0.464, -1.261, 0.460 },
  { 5014.157, -4.978, 0.486, -1.335, 0.487 },
  { 5026.657, -4.967, 0.476, -1.231, 0.476 },
  { 5039.157, -4.959, 0.487, -1.268, 0.497 },
  { 5051.657, -4.818, 0.475, -1.274, 0.496 },
  { 5064.157, -4.807, 0.485, -1.209, 0.483 },
  { 5076.657, -4.815, 0.487, -1.274, 0.488 },
  { 5089.157, -4.743, 0.497, -1.251, 0.487 },
  { 5101.657, -4.609, 0.501, -1.177, 0.505 },
  { 5114.157, -4.612, 0.512, -1.184, 0.510 },
  { 5126.657, -4.612, 0.521, -1.105, 0.530 },
  { 5139.157, -4.569, 0.500, -1.070, 0.505 },
  { 5151.657, -4.415, 0.539, -1.065, 0.546 },
  { 5164.157, -4.438, 0.494, -1.103, 0.517 },
  { 5176.657, -4.376, 0.597, -1.017, 0.605 },
  { 5189.157, -4.401, 0.536, -1.085, 0.543 },
  { 5201.657, -4.233, 0.505, -1.093, 0.503 },
  { 5214.157, -4.222, 0.492, -1.137, 0.481 },
  { 5226.657, -4.233, 0.470, -1.220, 0.497 },
  { 5251.657, -4.020, 0.456, -1.030, 0.440 },
  { 5264.157, -4.027, 0.477, -1.009, 0.447 },
  { 5289.157, -4.002, 0.439, -0.993, 0.440 },
  { 5301.657, -4.005, 0.455, -1.096, 0.462 },
  { 5314.157, -3.823, 0.423, -1.055, 0.460 },
  { 5326.657, -3.859, 0.472, -1.126, 0.441 },
  { 5339.157, -3.803, 0.439, -0.994, 0.446 },
  { 5351.657, -3.786, 0.441, -0.922, 0.455 },
  { 5364.157, -3.585, 0.432, -0.843, 0.440 },
  { 5376.657, -3.569, 0.450, -0.809, 0.449 },
  { 5389.157, -3.584, 0.435, -0.862, 0.444 },
  { 5401.657, -3.567, 0.456, -0.829, 0.452 },
  { 5414.157, -3.573, 0.451, -0.847, 0.464 },
  { 5426.657, -3.427, 0.437, -0.884, 0.450 },
  { 5439.157, -3.412, 0.444, -0.896, 0.453 },
  { 5451.657, -3.383, 0.444, -0.833, 0.445 },
  { 5464.157, -3.381, 0.438, -0.829, 0.448 },
  { 5476.657, -3.393, 0.441, -0.862, 0.450 },
  { 5489.157, -3.205, 0.441, -0.778, 0.443 },
  { 5501.657, -3.196, 0.444, -0.766, 0.454 },
  { 5514.157, -3.175, 0.450, -0.753, 0.454 },
  { 5526.657, -3.184, 0.439, -0.755, 0.443 },
  { 5539.157, -3.206, 0.436, -0.837, 0.433 },
  { 5551.657, -3.009, 0.432, -0.815, 0.438 },
  { 5564.157, -3.027, 0.436, -0.771, 0.494 },
  { 5589.157, -3.008, 0.456, -0.748, 0.457 },
  { 5601.657, -3.003, 0.443, -0.855, 0.453 },
  { 5614.157, -2.964, 0.427, -0.743, 0.435 },
  { 5626.657, -2.805, 0.433, -0.815, 0.455 },
  { 5639.157, -2.767, 0.429, -0.525, 0.444 },
  { 5651.657, -2.765, 0.429, -0.644, 0.436 },
  { 5664.157, -2.775, 0.436, -0.585, 0.466 },
  { 5676.657, -2.757, 0.429, -0.639, 0.438 },
  { 5689.157, -2.561, 0.435, -0.667, 0.440 },
  { 5701.657, -2.561, 0.434, -0.625, 0.443 },
  { 5714.157, -2.563, 0.440, -0.687, 0.446 },
  { 5726.657, -2.570, 0.430, -0.683, 0.435 },
  { 5739.157, -2.567, 0.429, -0.671, 0.435 },
  { 5751.657, -2.572, 0.425, -0.672, 0.431 },
  { 5764.157, -2.395, 0.424, -0.587, 0.435 },
  { 5776.657, -2.386, 0.420, -0.560, 0.435 },
  { 5789.157, -2.412, 0.428, -0.573, 0.440 },
  { 5801.657, -2.419, 0.421, -0.577, 0.435 },
  { 5814.157, -2.431, 0.424, -0.556, 0.438 },
  { 5826.657, -2.412, 0.420, -0.582, 0.430 },
  { 5839.157, -2.207, 0.422, -0.567, 0.432 },
  { 5851.657, -2.201, 0.423, -0.583, 0.430 },
  { 5876.657, -2.221, 0.424, -0.601, 0.432 },
  { 5889.157, -2.586, 0.527, -0.440, 0.536 },
  { 5926.657, -1.979, 0.429, -0.421, 0.425 },
  { 5939.157, -1.985, 0.421, -0.436, 0.438 },
  { 5951.657, -2.039, 0.410, -0.523, 0.426 },
  { 5964.157, -2.010, 0.422, -0.459, 0.436 },
  { 5989.157, -2.006, 0.422, -0.445, 0.431 },
  { 6001.657, -1.809, 0.428, -0.420, 0.425 },
  { 6014.157, -1.850, 0.425, -0.506, 0.435 },
  { 6026.657, -1.828, 0.430, -0.498, 0.433 },
  { 6039.157, -1.823, 0.418, -0.457, 0.432 },
  { 6051.657, -1.852, 0.418, -0.483, 0.430 },
  { 6064.157, -1.846, 0.419, -0.466, 0.430 },
  { 6076.657, -1.791, 0.416, -0.488, 0.430 },
  { 6089.157, -1.657, 0.421, -0.480, 0.437 },
  { 6101.657, -1.676, 0.421, -0.451, 0.447 },
  { 6114.157, -1.667, 0.420, -0.370, 0.431 },
  { 6126.657, -1.677, 0.431, -0.399, 0.444 },
  { 6139.157, -1.685, 0.431, -0.329, 0.453 },
  { 6151.657, -1.647, 0.413, -0.345, 0.444 },
  { 6164.157, -1.685, 0.452, -0.413, 0.464 },
  { 6176.657, -1.455, 0.423, -0.353, 0.455 },
  { 6189.157, -1.447, 0.412, -0.396, 0.436 },
  { 6201.657, -1.527, 0.411, -0.336, 0.489 },
  { 6214.157, -1.424, 0.425, -0.327, 0.408 },
  { 6226.657, -1.486, 0.416, -0.397, 0.438 },
  { 6239.157, -1.464, 0.414, -0.487, 0.460 },
  { 6251.657, -1.485, 0.418, -0.398, 0.441 },
  { 6264.157, -1.210, 0.427, -0.405, 0.401 },
  { 6276.657, -1.233, 0.425, -0.428, 0.413 },
  { 6314.157, -1.271, 0.414, -0.253, 0.434 },
  { 6326.657, -1.217, 0.419, -0.284, 0.404 },
  { 6339.157, -1.262, 0.419, -0.244, 0.437 },
  { 6351.657, -1.240, 0.410, -0.348, 0.422 },
  { 6376.657, -1.082, 0.412, -0.238, 0.427 },
  { 6389.157, -1.060, 0.415, -0.274, 0.423 },
  { 6401.657, -1.076, 0.420, -0.238, 0.427 },
  { 6414.157, -1.049, 0.414, -0.276, 0.406 },
  { 6426.657, -1.060, 0.415, -0.241, 0.431 },
  { 6439.157, -1.086, 0.415, -0.302, 0.425 },
  { 6451.657, -1.054, 0.412, -0.266, 0.426 },
  { 6464.157, -0.887, 0.427, -0.457, 0.388 },
  { 6476.657, -0.868, 0.414, -0.334, 0.399 },
  { 6489.157, -0.915, 0.414, -0.307, 0.425 },
  { 6514.157, -0.901, 0.412, -0.020, 0.443 },
  { 6526.657, -0.910, 0.408, -0.226, 0.420 },
  { 6539.157, -0.910, 0.412, -0.088, 0.421 },
  { 6589.157, -0.713, 0.406, -0.224, 0.425 },
  { 6614.157, -0.729, 0.411, -0.213, 0.425 },
  { 6626.657, -0.748, 0.410, -0.268, 0.425 },
  { 6639.157, -0.711, 0.405, -0.239, 0.423 },
  { 6651.657, -0.715, 0.408, -0.204, 0.418 },
  { 6664.157, -0.740, 0.409, -0.204, 0.416 },
  { 6676.657, -0.708, 0.406, -0.185, 0.419 },
  { 6689.157, -0.531, 0.406, -0.276, 0.421 },
  { 6701.657, -0.528, 0.407, -0.163, 0.422 },
  { 6714.157, -0.544, 0.405, -0.261, 0.425 },
  { 6726.657, -0.532, 0.408, -0.208, 0.418 },
  { 6739.157, -0.561, 0.400, -0.265, 0.419 },
  { 6751.657, -0.496, 0.403, -0.108, 0.420 },
  { 6764.157, -0.512, 0.403, -0.159, 0.420 },
  { 6776.657, -0.522, 0.404, -0.127, 0.421 },
  { 6789.157, -0.521, 0.406, -0.047, 0.412 },
  { 6801.657, -0.417, 0.401, -0.103, 0.417 },
  { 6814.157, -0.316, 0.396, -0.105, 0.414 },
  { 6826.657, -0.346, 0.400, -0.218, 0.446 },
  { 6839.157, -0.329, 0.396, -0.110, 0.422 },
  { 6851.657, -0.318, 0.404, -0.106, 0.406 },
  { 6876.657, -0.339, 0.436, -0.111, 0.467 },
  { 6889.157, -0.287, 0.407,  0.134, 0.403 },
  { 6901.657, -0.296, 0.422,  0.090, 0.383 },
  { 6914.157, -0.428, 0.408,  0.104, 0.441 },
  { 6926.657, -0.344, 0.396, -0.160, 0.426 },
  { 6939.157, -0.129, 0.408,  0.002, 0.384 },
  { 6951.657, -0.144, 0.401, -0.225, 0.445 },
  { 6964.157, -0.136, 0.396, -0.167, 0.414 },
  { 6989.157, -0.171, 0.397, -0.116, 0.418 },
  { 7001.657, -0.192, 0.395, -0.285, 0.431 },
  { 7014.157, -0.158, 0.396, -0.017, 0.418 },
  { 7026.657, -0.147, 0.400, -0.007, 0.412 },
  { 7039.157, -0.110, 0.397, -0.222, 0.420 },
  { 7051.657, -0.161, 0.397,  0.015, 0.416 },
  { 7064.157,  0.017, 0.403, -0.030, 0.426 },
  { 7076.657,  0.020, 0.402, -0.184, 0.409 },
  { 7089.157,  0.000, 0.407,  0.197, 0.409 },
  { 7101.657, -0.000, 0.400, -0.000, 0.418 },
  { 7114.157,  0.009, 0.398, -0.043, 0.420 },
  { 7126.657,  0.001, 0.400, -0.128, 0.391 },
  { 7139.157,  0.016, 0.402, -0.158, 0.409 },
  { 7151.657,  0.037, 0.405,  0.096, 0.419 },
  { 7164.157,  0.007, 0.403, -0.173, 0.411 },
  { 7176.657,  0.031, 0.413,  0.118, 0.419 },
  { 7189.157,  0.048, 0.418,  0.088, 0.425 },
  { 7201.657,  0.044, 0.406,  0.134, 0.415 },
  { 7214.157,  0.230, 0.401, -0.061, 0.413 },
  { 7226.657,  0.233, 0.403, -0.050, 0.428 },
  { 7264.157,  0.236, 0.400,  0.022, 0.413 },
  { 7351.657,  0.180, 0.398,  0.112, 0.406 },
  { 7364.157,  0.351, 0.397, -0.049, 0.411 },
  { 7376.657,  0.378, 0.394,  0.044, 0.407 },
  { 7389.157,  0.438, 0.398,  0.112, 0.403 },
  { 7414.157,  0.437, 0.392,  0.068, 0.416 },
  { 7426.657,  0.431, 0.398,  0.101, 0.390 },
  { 7451.657,  0.382, 0.392,  0.048, 0.411 },
  { 7464.157,  0.467, 0.390,  0.189, 0.420 },
  { 7476.657,  0.435, 0.391,  0.058, 0.405 },
  { 7489.157,  0.390, 0.391,  0.067, 0.416 },
  { 7501.657,  0.399, 0.392,  0.033, 0.408 },
  { 7514.157,  0.398, 0.391,  0.048, 0.414 },
  { 7526.657,  0.541, 0.387, -0.125, 0.423 },
  { 7539.157,  0.604, 0.394,  0.064, 0.400 },
  { 7551.657,  0.576, 0.389,  0.042, 0.407 },
  { 7564.157,  0.595, 0.390,  0.095, 0.405 },
  { 7576.657,  0.600, 0.392,  0.037, 0.410 },
  { 7589.157,  0.580, 0.394,  0.087, 0.413 },
  { 7601.657,  0.578, 0.613,  0.136, 0.693 },
  { 7626.657,  0.518, 0.515,  0.096, 0.794 },
  { 7639.157,  0.581, 0.427,  0.104, 0.405 },
  { 7664.157,  0.564, 0.404,  0.054, 0.358 },
  { 7676.657,  0.586, 0.406,  0.148, 0.416 },
  { 7689.157,  0.578, 0.405,  0.115, 0.376 },
  { 7701.657,  0.787, 0.404, -0.034, 0.395 },
  { 7764.157,  0.743, 0.393,  0.126, 0.406 },
  { 7776.657,  0.747, 0.395, -0.004, 0.408 },
  { 7801.657,  0.714, 0.396, -0.083, 0.399 },
  { 7814.157,  0.722, 0.401, -0.079, 0.396 },
  { 7826.657,  0.764, 0.396, -0.054, 0.420 },
  { 7864.157,  0.770, 0.393,  0.222, 0.406 },
  { 7889.157,  0.926, 0.399, -0.081, 0.457 },
  { 7939.157,  0.953, 0.401,  0.137, 0.408 },
  { 7951.657,  0.910, 0.401, -0.054, 0.399 },
  { 7989.157,  0.921, 0.404,  0.021, 0.394 },
  { 8001.657,  0.978, 0.400,  0.180, 0.410 },
  { 8039.157,  0.932, 0.401,  0.067, 0.399 },
  { 8051.657,  0.912, 0.407,  0.223, 0.451 },
  { 8076.657,  0.981, 0.400,  0.305, 0.402 },
  { 8089.157,  1.162, 0.403,  0.280, 0.413 },
  { 8101.657,  1.236, 0.408,  0.405, 0.375 },
  { 8114.157,  1.177, 0.407,  0.304, 0.411 },
  { 8126.657,  1.150, 0.407,  0.217, 0.406 },
  { 8139.157,  1.179, 0.398,  0.336, 0.410 },
  { 8151.657,  1.178, 0.411,  0.281, 0.426 },
  { 8164.157,  1.138, 0.429,  0.261, 0.427 },
  { 8176.657,  1.188, 0.415,  0.255, 0.423 },
  { 8189.157,  1.176, 0.410,  0.028, 0.451 },
  { 8201.657,  1.173, 0.426,  0.267, 0.424 },
  { 8214.157,  1.153, 0.415,  0.329, 0.419 },
  { 8226.657,  1.172, 0.410,  0.223, 0.442 },
  { 8239.157,  1.205, 0.404,  0.125, 0.420 },
  { 8251.657,  1.181, 0.406,  0.243, 0.414 },
  { 8264.157,  1.167, 0.406,  0.276, 0.412 },
  { 8301.657,  1.332, 0.402,  0.311, 0.409 },
  { 8314.157,  1.367, 0.404,  0.247, 0.414 },
  { 8326.657,  1.366, 0.404,  0.238, 0.417 },
  { 8339.157,  1.356, 0.405,  0.033, 0.429 },
  { 8376.657,  1.348, 0.401,  0.386, 0.421 },
  { 8389.157,  1.347, 0.410,  0.055, 0.427 },
  { 8439.157,  1.367, 0.412,  0.248, 0.423 },
  { 8476.657,  1.389, 0.401,  0.152, 0.417 },
  { 8489.157,  1.375, 0.402,  0.216, 0.409 },
  { 8514.157,  1.380, 0.401,  0.250, 0.413 },
  { 8526.657,  1.363, 0.403,  0.343, 0.415 },
  { 8551.657,  1.576, 0.412,  0.306, 0.425 },
  { 8564.157,  1.571, 0.398,  0.346, 0.412 },
  { 8576.657,  1.550, 0.401,  0.322, 0.411 },
  { 8589.157,  1.605, 0.408,  0.393, 0.402 },
  { 8601.657,  1.579, 0.400,  0.307, 0.409 },
  { 8614.157,  1.605, 0.402,  0.323, 0.406 },
  { 8639.157,  1.577, 0.409,  0.369, 0.397 },
  { 8689.157,  1.603, 0.411,  0.226, 0.392 },
  { 8701.657,  1.618, 0.412,  0.125, 0.394 },
  { 8714.157,  1.601, 0.401,  0.315, 0.409 },
  { 8726.657,  1.590, 0.396,  0.467, 0.426 },
  { 8739.157,  1.608, 0.407,  0.358, 0.407 },
  { 8751.657,  1.596, 0.394,  0.370, 0.426 },
  { 8776.657,  1.320, 0.351,  0.323, 0.388 },
  { 8801.657,  1.820, 0.402,  0.389, 0.424 },
  { 8814.157,  1.832, 0.404,  0.373, 0.418 },
  { 8876.657,  1.861, 0.404,  0.378, 0.417 },
  { 8889.157,  1.855, 0.403,  0.247, 0.418 },
  { 8914.157,  1.873, 0.403,  0.334, 0.409 },
  { 8926.657,  1.855, 0.412,  0.313, 0.383 },
  { 8939.157,  1.894, 0.404,  0.218, 0.423 },
  { 8951.657,  1.868, 0.412,  0.404, 0.425 },
  { 8964.157,  1.876, 0.431,  0.237, 0.432 },
  { 8976.657,  1.888, 0.407,  0.547, 0.421 },
  { 9014.157,  1.889, 0.422,  0.361, 0.429 },
  { 9026.657,  1.845, 0.425,  0.307, 0.437 },
  { 9076.657,  2.072, 0.417,  0.343, 0.427 },
  { 9101.657,  1.878, 0.459,  0.380, 0.345 },
  { 9114.157,  2.075, 0.412,  0.462, 0.424 },
  { 9126.657,  2.076, 0.412,  0.462, 0.425 },
  { 9139.157,  2.064, 0.418,  0.530, 0.431 },
  { 9164.157,  2.058, 0.414,  0.460, 0.421 },
  { 9176.657,  2.057, 0.421,  0.489, 0.440 },
  { 9189.157,  2.048, 0.412,  0.464, 0.426 },
  { 9201.657,  2.038, 0.408,  0.469, 0.422 },
  { 9214.157,  2.038, 0.418,  0.482, 0.433 },
  { 9226.657,  2.022, 0.410,  0.460, 0.423 },
  { 9239.157,  2.007, 0.414,  0.445, 0.426 },
  { 9251.657,  2.011, 0.413,  0.467, 0.426 },
  { 9264.157,  2.007, 0.419,  0.464, 0.433 },
  { 9276.657,  2.004, 0.415,  0.487, 0.428 },
  { 9289.157,  1.975, 0.412,  0.464, 0.424 },
  { 9301.657,  1.950, 0.435,  0.311, 0.455 },
  { 0.0,     0.0,   0.0,   0.0,    0.0  }
};

/* Reference wavelenghts */
const float kStd1Ref  = 5275.644,
            kStd2Ref  = 5273.996,
            kMuse1Ref = 7100.;

/* Aproximation method */
hdrl_airmass_approx typeAirmassAprox = HDRL_AIRMASS_APPROX_HARDIE;


/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_test_dar_correct() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_test_dar_correct(void)
{
	/* create minimal headers for testing; first, add                *
	 * generic stuff, same for both standard star exposures          *
	 * PMAS has a position angle of about -90 deg, but the position  *
	 * table to create the data orients it to a pos. angle of 0 deg. *
	 * add some fake pixel table limits                              */

	cpl_propertylist *header1 = cpl_propertylist_new();
	cpl_propertylist_append_double(header1, "RA", 					122.994945);
	cpl_propertylist_append_double(header1, "DEC", 					74.95304);
	cpl_propertylist_append_double(header1, "LST", 					25407.072748);
	cpl_propertylist_append_double(header1, "EXPTIME", 				120.);
	cpl_propertylist_append_double(header1, "ESO TEL GEOLAT",		37.2236);
	cpl_propertylist_append_double(header1, "ESO TEL AIRM START",	1.2784545043);
	cpl_propertylist_append_double(header1, "ESO TEL AIRM END",		1.2776009017);

	cpl_propertylist *header2 = cpl_propertylist_new();
	cpl_propertylist_append_double(header2, "RA", 					238.071555);
	cpl_propertylist_append_double(header2, "DEC", 					32.92533);
	cpl_propertylist_append_double(header2, "LST", 					60515.584209);
	cpl_propertylist_append_double(header2, "EXPTIME", 				300.);
	cpl_propertylist_append_double(header2, "ESO TEL GEOLAT", 		37.2236);
	cpl_propertylist_append_double(header2, "ESO TEL AIRM START",	1.0233752603);
	cpl_propertylist_append_double(header2, "ESO TEL AIRM END",		1.02725831881);

	cpl_propertylist *header3 = cpl_propertylist_new();
	cpl_propertylist_append_double(header3, "RA", 					0.125);
	cpl_propertylist_append_double(header3, "DEC", 					-30.);
	cpl_propertylist_append_double(header3, "LST", 					69446.2765265328);
	cpl_propertylist_append_double(header3, "EXPTIME", 				3600.);
	cpl_propertylist_append_double(header3, "ESO TEL GEOLAT", 		-24.625278);
	cpl_propertylist_append_double(header3, "ESO TEL AIRM START",	2.14734965358319);
	cpl_propertylist_append_double(header3, "ESO TEL AIRM END",		1.5383756343353);

	/* Try again with a table that was astrometrically calibrated *
	 * this is PMAS data, so cannot use hdrl_wcs_create_default() */

 	cpl_propertylist *plWCS = cpl_propertylist_new();
	cpl_propertylist_append_string( plWCS, "CTYPE1", 	"RA---TAN"		);
	cpl_propertylist_append_string( plWCS, "CTYPE2", 	"DEC--TAN"		);
    cpl_propertylist_append_double( plWCS, "CRVAL1", 	 30.0			);
    cpl_propertylist_append_double( plWCS, "CRVAL2", 	 12.0			);
	cpl_propertylist_append_double( plWCS, "CRPIX1", 	 8.				);
	cpl_propertylist_append_double( plWCS, "CRPIX2", 	 8.				);
	cpl_propertylist_append_double( plWCS, "CD1_1", 	-0.2 /3600.		);
	cpl_propertylist_append_double( plWCS, "CD1_2", 	 0.				);
	cpl_propertylist_append_double( plWCS, "CD2_2", 	 0.2 /3600.		);
	cpl_propertylist_append_double( plWCS, "CD2_1", 	 0.				);
	cpl_propertylist_append_string( plWCS, "CUNIT1", 	"deg"			);
	cpl_propertylist_append_string( plWCS, "CUNIT2", 	"deg"			);
	cpl_propertylist_append_int(    plWCS, "WCSAXES",	 2				);
	cpl_wcs *wcs = cpl_wcs_new_from_propertylist(plWCS);



	/******* Charge data *************/

	/* MUSE DataSheet 1 */
	cpl_size ind = 0;
	while (kStd1[ind][0] > 1000. && ind - 1 < NMAX) {
		ind++;
	}
	cpl_vector *lambdaIn1    = cpl_vector_new(ind);
	cpl_vector *xShift1      = cpl_vector_new(ind);
	cpl_vector *yShift1      = cpl_vector_new(ind);
	cpl_vector *xShift1Err   = cpl_vector_new(ind);
	cpl_vector *yShift1Err   = cpl_vector_new(ind);
	cpl_vector *xCorrect1    = cpl_vector_new(ind);
	cpl_vector *yCorrect1    = cpl_vector_new(ind);
	ind = 0;
	while (kStd1[ind][0] > 1000. && ind - 1 < NMAX) {
		cpl_vector_set(lambdaIn1, ind, kStd1[ind][0]);
		ind++;
	}


	/* MUSE DataSheet 2 */
	ind = 0;
	while (kStd2[ind][0] > 1000. && ind - 1 < NMAX) {
		ind++;
	}
	cpl_vector *lambdaIn2    = cpl_vector_new(ind);
	cpl_vector *xShift2      = cpl_vector_new(ind);
	cpl_vector *yShift2      = cpl_vector_new(ind);
	cpl_vector *xShift2Err   = cpl_vector_new(ind);
	cpl_vector *yShift2Err   = cpl_vector_new(ind);
	cpl_vector *xCorrect2    = cpl_vector_new(ind);
	cpl_vector *yCorrect2    = cpl_vector_new(ind);

	cpl_vector *xShift4      = cpl_vector_new(ind);
	cpl_vector *xShift5      = cpl_vector_new(ind);
	cpl_vector *xShift6      = cpl_vector_new(ind);
	cpl_vector *xShift7      = cpl_vector_new(ind);
	cpl_vector *xShift8      = cpl_vector_new(ind);

	cpl_vector *yShift4      = cpl_vector_new(ind);
	cpl_vector *yShift5      = cpl_vector_new(ind);
	cpl_vector *yShift6      = cpl_vector_new(ind);
	cpl_vector *yShift7      = cpl_vector_new(ind);
	cpl_vector *yShift8      = cpl_vector_new(ind);

	cpl_vector *xShift4Err   = cpl_vector_new(ind);
	cpl_vector *xShift5Err   = cpl_vector_new(ind);
	cpl_vector *xShift6Err   = cpl_vector_new(ind);
	cpl_vector *xShift7Err   = cpl_vector_new(ind);
	cpl_vector *xShift8Err   = cpl_vector_new(ind);

	cpl_vector *yShift4Err   = cpl_vector_new(ind);
	cpl_vector *yShift5Err   = cpl_vector_new(ind);
	cpl_vector *yShift6Err   = cpl_vector_new(ind);
	cpl_vector *yShift7Err   = cpl_vector_new(ind);
	cpl_vector *yShift8Err   = cpl_vector_new(ind);

	ind = 0;
	while (kStd2[ind][0] > 1000. && ind - 1 < NMAX) {
		cpl_vector_set(lambdaIn2, ind, kStd2[ind][0]);
		ind++;
	}


	/* MUSE DataSheet 3 */
	ind = 0;
	while (kMuse1[ind][0] > 1000. && ind - 1 < NMAX) {
		ind++;
	}
	cpl_vector *lambdaIn3    = cpl_vector_new(ind);
	cpl_vector *xShift3      = cpl_vector_new(ind);
	cpl_vector *yShift3      = cpl_vector_new(ind);
	cpl_vector *xShift3Err   = cpl_vector_new(ind);
	cpl_vector *yShift3Err   = cpl_vector_new(ind);
	cpl_vector *xCorrect3    = cpl_vector_new(ind);
	cpl_vector *yCorrect3    = cpl_vector_new(ind);
	ind = 0;
	while (kMuse1[ind][0] > 1000. && ind - 1 < NMAX) {
		cpl_vector_set(lambdaIn3, ind, kMuse1[ind][0]);
		ind++;
	}


	/* Random value for errors */

	double delta        = 1e-2;
	double deltaAirmass = 1e-4;

	double lambdaRefErr = delta;
	double parangErr    = delta;
	double posangErr    = delta;
	double tempErr      = delta;
	double rhumErr      = delta;
	double presErr      = delta;

	/********************* Do the test *************************/
	cpl_errorstate prestate;

	/* Input parameters and airmass (and error propagation) with MUSE datasheet 1 */
	hdrl_value ra1           = {cpl_propertylist_get_double(header1, "RA"					), 0.};
	hdrl_value dec1          = {cpl_propertylist_get_double(header1, "DEC"					), 0.};
	hdrl_value lst1          = {cpl_propertylist_get_double(header1, "LST"					), 0.};
	hdrl_value exptime1      = {cpl_propertylist_get_double(header1, "EXPTIME"				), 0.};
	hdrl_value geolat1       = {cpl_propertylist_get_double(header1, "ESO TEL GEOLAT"		), 0.};

	ra1.error           	 = deltaAirmass * fabs(ra1.data);
	dec1.error          	 = deltaAirmass * fabs(dec1.data);
	lst1.error          	 = deltaAirmass * fabs(lst1.data);
	exptime1.error      	 = deltaAirmass * fabs(exptime1.data);
	geolat1.error      	     = deltaAirmass * fabs(geolat1.data);

	hdrl_value airmass1 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox);

	hdrl_parameter *h_par1 = hdrl_dar_parameter_create(
		(hdrl_value){ airmass1.data,    airmass1.error},
		(hdrl_value){ -157.90507882793, parangErr * fabs(-157.90507882793)},
		(hdrl_value){ 0.,               posangErr * fabs(0.) +posangErr},
		(hdrl_value){ -6.2,             tempErr   * fabs(-6.2)},
		(hdrl_value){ 65.,              rhumErr   * fabs(65.)},
		(hdrl_value){ 768.4,            presErr   * fabs(768.4)},
		wcs);

	/* Input parameters and airmass (and error propagation) with MUSE datasheet 2 */
	hdrl_value ra2           = {cpl_propertylist_get_double(header2, "RA"					), 0.};
	hdrl_value dec2          = {cpl_propertylist_get_double(header2, "DEC"					), 0.};
	hdrl_value lst2          = {cpl_propertylist_get_double(header2, "LST"					), 0.};
	hdrl_value exptime2      = {cpl_propertylist_get_double(header2, "EXPTIME"				), 0.};
	hdrl_value geolat2       = {cpl_propertylist_get_double(header2, "ESO TEL GEOLAT"		), 0.};

	ra2.error           	 = deltaAirmass * fabs(ra2.data);
	dec2.error          	 = deltaAirmass * fabs(dec2.data);
	lst2.error          	 = deltaAirmass * fabs(lst2.data);
	exptime2.error      	 = deltaAirmass * fabs(exptime2.data);
	geolat2.error      	     = deltaAirmass * fabs(geolat2.data);

	hdrl_value airmass2 = hdrl_utils_airmass(ra2, dec2, lst2, exptime2, geolat2, typeAirmassAprox);

	hdrl_parameter *h_par2 = hdrl_dar_parameter_create(
		(hdrl_value){ airmass2.data,	airmass2.error},
		(hdrl_value){ 65.577407,		parangErr * fabs(65.577407)},
		(hdrl_value){ 0.,				posangErr * fabs(0.) + posangErr},
		(hdrl_value){ -5.7,				tempErr   * fabs(-5.7)},
		(hdrl_value){ 81.,				rhumErr   * fabs(81.)},
		(hdrl_value){ 768.4,			presErr   * fabs(768.4)},
		wcs);


	/* Input parameters and airmass (and error propagation) with MUSE datasheet 1 */
	hdrl_value ra3           = {cpl_propertylist_get_double(header3, "RA"					), 0.};
	hdrl_value dec3          = {cpl_propertylist_get_double(header3, "DEC"					), 0.};
	hdrl_value lst3          = {cpl_propertylist_get_double(header3, "LST"					), 0.};
	hdrl_value exptime3      = {cpl_propertylist_get_double(header3, "EXPTIME"				), 0.};
	hdrl_value geolat3       = {cpl_propertylist_get_double(header3, "ESO TEL GEOLAT"		), 0.};

	ra3.error           	 = deltaAirmass * fabs(ra3.data);
	dec3.error          	 = deltaAirmass * fabs(dec3.data);
	lst3.error          	 = deltaAirmass * fabs(lst3.data);
	exptime3.error      	 = deltaAirmass * fabs(exptime3.data);
	geolat3.error      	     = deltaAirmass * fabs(geolat3.data);

	hdrl_value airmass3 = hdrl_utils_airmass(ra3, dec3, lst3, exptime3, geolat3, typeAirmassAprox);

	hdrl_parameter *h_par3   = hdrl_dar_parameter_create(
		(hdrl_value){ airmass3.data,	airmass3.error},
		(hdrl_value){ -100.978115969,	parangErr * fabs(-100.978115969)},
		(hdrl_value){ 0.,				posangErr * fabs(0.) + posangErr},
		(hdrl_value){ 10.,				tempErr   * fabs(10.)},
		(hdrl_value){ 10.,				rhumErr   * fabs(10.)},
		(hdrl_value){ 775.,				presErr   * fabs(775.)},
		wcs);

	/********** Test hdrl_dar_wcs_get_scales with deformed scales  **********/

	/* Create propertylist for build a non-equal scale wcs */
    cpl_propertylist *header = cpl_propertylist_new();

    const char *skeys[2] = {"CTYPE1", "CTYPE2"};
    const char *svals[2] = {"RA---ZPN", "DEC--ZPN"};
    for (int i = 0; i < 2; i++) {
        cpl_propertylist_append_string(header,skeys[i],svals[i]);
    }

    const char *dkeys[13] = {"CRVAL1", "CRVAL2", "CRPIX1", "CRPIX2",
                              "CD1_1", "CD1_2", "CD2_1", "CD2_2", "PV2_1",
                              "PV2_2", "PV2_3", "PV2_4", "PV2_5"};
    const double dvals[13] = {5.57368333333, -72.0576388889, 5401.6, 6860.8,
                               5.81347849634012E-21, 9.49444444444444E-05,
                               -9.49444444444444E-05, -5.81347849634012E-21,
                               1.0, 0.0, 42.0, 0.0, 0.0};
    for (int i = 0; i < 13; i++) {
        cpl_propertylist_append_double(header,dkeys[i],dvals[i]);
    }

    const char *ikeys[3] = {"NAXIS","NAXIS1","NAXIS2"};
    const int ivals[3] = {2, 2048, 2048};
    for (int i = 0; i < 3; i++) {
        cpl_propertylist_append_int(header,ikeys[i],ivals[i]);
    }

    /* Create the WCS header */
    cpl_wcs *wcsFake = cpl_wcs_new_from_propertylist(header);

    /* Test scales from WCS */
    double xscale, yscale;
	cpl_test(hdrl_dar_wcs_get_scales(wcsFake, &xscale, &yscale) == CPL_ERROR_NONE);

	cpl_propertylist_delete(header);
	cpl_wcs_delete(wcsFake);


	/*************** Test of failure cases ************************/

	/* Test failure case: Create hdrl_parameter with non valid data */
	cpl_test( hdrl_dar_parameter_create( (hdrl_value){-1.,0.},
									 	 (hdrl_value){-1.,0.},
										 (hdrl_value){-1.,0.},
										 (hdrl_value){-1.,0.},
										 (hdrl_value){-1.,0.},
										 (hdrl_value){-1.,0.},
										 NULL) == NULL);

	/* Test failure case: Null h_par input */
	hdrl_dar_compute(  NULL,
					   (hdrl_value){kStd1Ref, lambdaRefErr * kStd1Ref},
					   lambdaIn1,
					   xShift1,    yShift1,
					   xShift1Err, yShift1Err);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	/* Test failure case: Null lambda input */
	hdrl_dar_compute(	h_par1,
						(hdrl_value){kStd1Ref, lambdaRefErr * kStd1Ref},
						NULL,
						xShift1,    yShift1,
						xShift1Err, yShift1Err);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	/* Test failure case: Null xshift/yshift output */
	hdrl_dar_compute(	h_par1,
						(hdrl_value){kStd1Ref, lambdaRefErr * kStd1Ref},
						lambdaIn1,
						NULL, NULL,
						NULL, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);


	/*************** Test with Lambda fake (with NANs) *********************/

	/* Test failure case: Lambda with NaNs */
	cpl_vector *lambdaIn = cpl_vector_new(1);
	cpl_vector_set(lambdaIn, 0, NAN);
	hdrl_dar_compute(	h_par1,
						(hdrl_value){kStd1Ref, lambdaRefErr * kStd1Ref},
						lambdaIn,
						xShift1,    yShift1,
						xShift1Err, yShift1Err);
	//cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_vector_delete(lambdaIn);


	/********************* Test with MUSE real data *************************/

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par1,
							   (hdrl_value){kStd1Ref, lambdaRefErr * kStd1Ref},
							   lambdaIn1,
							   xShift1,    yShift1,
							   xShift1Err, yShift1Err)
							   == CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){kStd2Ref, lambdaRefErr * kStd2Ref},
							   lambdaIn2,
							   xShift2,    yShift2,
							   xShift2Err, yShift2Err)
							   == CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par3,
							   (hdrl_value){kMuse1Ref, lambdaRefErr * kMuse1Ref},
							   lambdaIn3,
							   xShift3,    yShift3,
							   xShift3Err, yShift3Err)
							   == CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);



	/* check the result */

	double size1 = cpl_vector_get_size(lambdaIn1);
	cpl_vector_set_size(xCorrect1, size1);
	cpl_vector_set_size(yCorrect1, size1);
	for (cpl_size i = 0; i < cpl_vector_get_size(lambdaIn1); i++) {
		cpl_vector_set(xCorrect1, i, -kStd1[i][1] / 0.2 + cpl_vector_get(xShift1, i));
		cpl_vector_set(yCorrect1, i, -kStd1[i][3] / 0.2 + cpl_vector_get(yShift1, i));
	}

	double size2 = cpl_vector_get_size(lambdaIn2);
	cpl_vector_set_size(xCorrect2, size2);
	cpl_vector_set_size(yCorrect2, size2);
	for (cpl_size i = 0; i < cpl_vector_get_size(lambdaIn2); i++) {
		cpl_vector_set(xCorrect2, i, -kStd2[i][1] / 0.2 + cpl_vector_get(xShift2, i));
		cpl_vector_set(yCorrect2, i, -kStd2[i][3] / 0.2 + cpl_vector_get(yShift2, i));
	}

	double size3 = cpl_vector_get_size(lambdaIn3);
	cpl_vector_set_size(xCorrect3, size3);
	cpl_vector_set_size(yCorrect3, size3);
	for (cpl_size i = 0; i < cpl_vector_get_size(lambdaIn3); i++) {
		cpl_vector_set(xCorrect3, i, -kMuse1[i][1] + cpl_vector_get(xShift3, i));
		cpl_vector_set(yCorrect3, i,  kMuse1[i][3] + cpl_vector_get(yShift3, i));
	}


	double x1mean = cpl_vector_get_mean(   xCorrect1),
		   x1med  = cpl_vector_get_median( xCorrect1),
		   x1std  = cpl_vector_get_stdev(  xCorrect1),
		   y1mean = cpl_vector_get_mean(   yCorrect1),
		   y1med  = cpl_vector_get_median( yCorrect1),
		   y1std  = cpl_vector_get_stdev(  yCorrect1);

	double x2mean = cpl_vector_get_mean(   xCorrect2),
		   x2med  = cpl_vector_get_median( xCorrect2),
		   x2std  = cpl_vector_get_stdev(  xCorrect2),
		   y2mean = cpl_vector_get_mean(   yCorrect2),
		   y2med  = cpl_vector_get_median( yCorrect2),
		   y2std  = cpl_vector_get_stdev(  yCorrect2);

	double x3mean = cpl_vector_get_mean(   xCorrect3),
		   x3med  = cpl_vector_get_median( xCorrect3),
		   x3std  = cpl_vector_get_stdev(  xCorrect3),
		   y3mean = cpl_vector_get_mean(   yCorrect3),
		   y3med  = cpl_vector_get_median( yCorrect3),
		   y3std  = cpl_vector_get_stdev(  yCorrect3);


	/* Residual mean, median, and stdev should be below 1/10 pix; write tests *
	 * using account actual values if they provide more stringent constraints.*/

	cpl_msg_info(cpl_func, "Std1: x %f(%f)+/-%f  y %f(%f)+/-%f",
							x1mean, x1med, x1std, y1mean, y1med, y1std);
	cpl_test(fabs(x1mean) < 0.045 );
	cpl_test(fabs(x1med ) < 0.045 );
	cpl_test(fabs(x1std ) < 0.068 );

	cpl_test(fabs(y1mean) < 0.0472);
	cpl_test(fabs(y1med ) < 0.0632);
	cpl_test(fabs(y1std ) < 0.0867);

	cpl_test(fabs(x1mean) < x1std );
	cpl_test(fabs(x1med ) < x1std );

	cpl_test(fabs(y1mean) < y1std );
	cpl_test(fabs(y1med ) < y1std );

	cpl_msg_info(cpl_func, "Std2: x %f(%f)+/-%f  y %f(%f)+/-%f",
							x2mean, x2med, x2std, y2mean, y2med, y2std);
	cpl_test(fabs(x2mean) < 0.020 );
	cpl_test(fabs(x2med ) < 0.0171);
	cpl_test(fabs(x2std ) < 0.039 );

	cpl_test(fabs(y2mean) < 0.015 );
	cpl_test(fabs(y2med ) < 0.014 );
	cpl_test(fabs(y2std ) < 0.024 );

	cpl_test(fabs(x2mean) < x2std );
	cpl_test(fabs(x2med ) < x2std );

	cpl_test(fabs(y2mean) < y2std );
	cpl_test(fabs(y2med ) < y2std );

	/* Larger residuals because of scale "problem" with INM data */
	cpl_msg_info(cpl_func, "Muse1 x %f(%f)+/-%f  y %f(%f)+/-%f",
							x3mean, x3med, x3std, y3mean, y3med, y3std);
	cpl_test(fabs(x3mean) < 0.27  );
	cpl_test(fabs(x3med ) < 0.13  );
	cpl_test(fabs(x3std ) < 0.58  );

	cpl_test(fabs(y3mean) < 0.15  );
	cpl_test(fabs(y3med ) < 0.12  );
	cpl_test(fabs(y3std ) < 0.24  );

	cpl_test(fabs(x3mean) < x3std );
	cpl_test(fabs(x3med ) < x3std );

	cpl_test(fabs(y3mean) < y3std );
	cpl_test(fabs(y3med ) < y3std );


	/******* More test with MUSE data: header2 with other lambdaRef *******/

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){2000., lambdaRefErr * 2000.},
							   lambdaIn2,
							   xShift4,    yShift4,
							   xShift4Err, yShift4Err)
												== CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate));

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){7000., lambdaRefErr * 7000.},
							   lambdaIn2,
							   xShift5,    yShift5,
							   xShift5Err, yShift5Err)
												== CPL_ERROR_NONE);
    cpl_test(cpl_errorstate_is_equal(prestate));

 	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){30000., lambdaRefErr * 30000.},
							   lambdaIn2,
							   xShift6,    yShift6,
							   xShift6Err, yShift6Err)
							   == CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate));


	/*** Check MUSE data: header2 with other lambdaRef with/without errors ***/

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){19500., lambdaRefErr * 19500.},
							   lambdaIn2,
							   xShift7,    yShift7,
							   xShift7Err, yShift7Err)
							   == CPL_ERROR_NONE);
	cpl_test(cpl_errorstate_is_equal(prestate));

	prestate = cpl_errorstate_get();
	cpl_test(hdrl_dar_compute( h_par2,
							   (hdrl_value){19500., 0.},
							   lambdaIn2,
							   xShift8,    yShift8,
							   xShift8Err, yShift8Err)
							   == CPL_ERROR_NONE);
    cpl_test(cpl_errorstate_is_equal(prestate));


	/******************** Delete Storage memory **********************/

	cpl_propertylist_delete(header1);	header1 = NULL;
	cpl_propertylist_delete(header2);	header2 = NULL;
	cpl_propertylist_delete(header3);	header3 = NULL;

	cpl_propertylist_delete(plWCS);	    plWCS   = NULL;

	cpl_wcs_delete(wcs);

	cpl_vector_delete(lambdaIn1);
	cpl_vector_delete(lambdaIn2);
	cpl_vector_delete(lambdaIn3);

	cpl_vector_delete(xShift1);			cpl_vector_delete(yShift1);
	cpl_vector_delete(xShift2);			cpl_vector_delete(yShift2);
	cpl_vector_delete(xShift3);			cpl_vector_delete(yShift3);
	cpl_vector_delete(xShift4);			cpl_vector_delete(yShift4);
	cpl_vector_delete(xShift5);			cpl_vector_delete(yShift5);
	cpl_vector_delete(xShift6);			cpl_vector_delete(yShift6);
	cpl_vector_delete(xShift7);			cpl_vector_delete(yShift7);
	cpl_vector_delete(xShift8);			cpl_vector_delete(yShift8);

	cpl_vector_delete(xShift1Err);		cpl_vector_delete(yShift1Err);
	cpl_vector_delete(xShift2Err);		cpl_vector_delete(yShift2Err);
	cpl_vector_delete(xShift3Err);		cpl_vector_delete(yShift3Err);
	cpl_vector_delete(xShift4Err);		cpl_vector_delete(yShift4Err);
	cpl_vector_delete(xShift5Err);		cpl_vector_delete(yShift5Err);
	cpl_vector_delete(xShift6Err);		cpl_vector_delete(yShift6Err);
	cpl_vector_delete(xShift7Err);		cpl_vector_delete(yShift7Err);
	cpl_vector_delete(xShift8Err);		cpl_vector_delete(yShift8Err);

	cpl_vector_delete(xCorrect1);		cpl_vector_delete(yCorrect1);
	cpl_vector_delete(xCorrect2);		cpl_vector_delete(yCorrect2);
	cpl_vector_delete(xCorrect3);		cpl_vector_delete(yCorrect3);

	hdrl_parameter_delete(h_par1);
	hdrl_parameter_delete(h_par2);
	hdrl_parameter_delete(h_par3);


    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of DAR
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_test_dar_correct();
    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
