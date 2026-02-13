/* $Id: hdrl.h,v 1.4 2013-10-23 09:42:14 jtaylor Exp $
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

/*----------------------------------------------------------------------------*/
/* Documentation here is used in the reference manual Main Page */

/** 

\mainpage HDRL Introduction

The European Southern Observatory (ESO) provides pipelines to reduce data
for almost all Very Large Telescopes (VLT) instruments. In order to reduce
the cost of development, verification, and maintenance of ESO pipelines, and
at the same time to improve the scientific quality of pipelines data
products, ESO develops a limited set of versatile and instrument-independent
high-level scientific functions to be used in the pipelines. These routines
are provided by the High-level Data Reduction Library (HDRL).
This Doxygen based documentation provides in the following sections
information on the HDRL Releases, on its Usage, what are the external
packages the library depends on, and a few
other useful links.

\section releases Releases

There is no fixed release cycle for the HDRL library (as e.g. for
CPL), but new releases are feature-driven, i.e. if there are new
functionality/algorithms available and carefully tested a new
release will be announced and the pipeline developer can change the
svn::external to this new release. This has the advantage that the
pipeline developer has more freedom to decide when to update the
pipeline. On the other hand it allows the developer to incorporate the
new HDRL release to his pipeline on a very short timescale and
prepare a new Paranal/public release. This is only possible as the
library is not installed in the Data Flow System environment in
Paranal (like CPL) but delivered inside each pipeline.


\subsection version150 Release version 1.5.0
The HDRL release version 1.5.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.5.0

In this release we updated and/or added the following algorithms:

- The computation of the barycenric correction, i.e. the wavelength
  shift to apply to a spectrum to compensate for the motion of the
  observer with respect to the barycenter of the solar system.  The
  implemented algorithm derives the barycentric correction of an
  observation by using the <a
  href="https://github.com/liberfa/erfa">ERFA</a> (Essential Routines
  for Fundamental Astronomy) library. ERFA is a C library containing
  key algorithms for astronomy, and is based on the <a
  href="http://www.iausofa.org">SOFA library</a> published by the
  International Astronomical Union (IAU). See the manual for detailed
  information.

- The build system has been modified to include GSL as direct
   dependency to the hdrl unit-test as direct GSL calls are performed
   when unit-testing the limiting magnitude module.

- For the barycentric correction algorithm two additional
   dependencies were added to HDRL.
    - A dependency to the <a
      href="https://github.com/liberfa/erfa">ERFA</a> (Essential
      Routines for Fundamental Astronomy) library.
    - A dependency to the <a href="https://curl.se/libcurl">libcurl</a>
      (The multiprotocol file transfer library) library

\subsection version140  Release version 1.4.0

The HDRL release version 1.4.0 can be included from
http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.4.0

In this release we updated and/or added the following algorithms:

- The computation of the limiting magnitude of an image as defined in the <a
  href="https://www.eso.org/sci/observing/phase3.html">ESO Phase 3
  Standard</a>. The limiting magnitude characterizes the depth of an observation
  and is defined as the magnitude of a unresolved source whose flux is 5 times
  the noise background, i.e. the magnitude of a point like source detected with
  \f$ \frac{S}{N}=5 \f$. See the manual for detailed information.

- To the statistical estimators we added the \b mode of a distribution,
  i.e. the following algorithms are now supported for collapsing imagelists or
  deriving statistics on images:
    - Mean
    - Weighted mean
    - Min-max rejected mean
    - \f$ \mathbf{\kappa\sigma} \f$ clipped mean
    - Median
    - Mode

  Please note that all methods but the mode are doing \b error \b
  propagation. The mode method is special in this case as it <b>calculates the
  error from the data</b>. The error estimation can either be done analytically
  or based on a bootstrap Montecarlo simulation. In this case the input data are
  perturbed with the bootstrap technique and the mode is calculated N times
  (controlled with a parameter). From this N modes the standard deviation is
  calculated and returned as error. See the manual for detailed information on
  the mode algorithm.

- Due to the addition of the mode, the functions
  hdrl_overscan_parameter_create_parlist() and
  hdrl_collapse_parameter_create_parlist() have changed. The two functions now
  require an additional default mode hdrl parameter. See the doxygen information
  of the two functions for more details.

\subsection version130  Release version 1.3.0

The HDRL release version 1.3.0 can be included from
http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.3.0

In this release we updated and/or added the following algorithms:
- Resampling of 2-dimensional images and 3-dimensional cubes. A common
  problem in astronomy is the resampling of images (or cubes) onto a common
  grid. Ideally, this is done only once in the data reduction workflow as each
  sub pixel resampling redistributes the flux and leads to correlations. The
  algorithm provided by the HDRL is doing the 2D and 3D interpolation in
  2-dimensional and 3-dimensional spaces, respectively. Currently there are six
  different interpolation methods implemented:
    - \b Nearest: Nearest neighbour resampling
    - \b Linear: Weighted resampling using an inverse distance weighting
      function
    - \b Quadratic: Weighted resampling using a quadratic inverse distance
      weighting function
    - \b Renka: Weighted resampling using a Renka weighting function
    - \b Drizzle: Weighted resampling using a drizzle-like weighting scheme
    - \b Lanczos: Weighted resampling using a lanczos-like restricted sinc as
      weighting function

- The object catalogue generation code has been updated. In
  previous releases, pixels with a value of exactly 0 where
  automatically added to the confidence map as zero and excluded
  in all further computations. This has been removed.

\subsection  version120 Release version 1.2.0
The HDRL release version 1.2.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.2.0

In this release we updated and/or added the following algorithms:

- Detection of fixed pattern noise. A classical example is pick noise,
  i.e. low-amplitude, quasi-periodical patterns super-imposed on the
  normal read-noise. It is due to electronic interference and might
  show up or disappear on short timescales (days or hours). The
  algorithms tries to identify it by the usage of the power spectrum.
- An error in the documentation of the strehl ratio variable \verbatim
  m1_radius \endverbatim and \verbatim m2_radius \endverbatim was
  corrected. The code was correct.
- In the spectral efficiency computation a sign error in the
  atmospheric correction was corrected in the documentation as well as
  in the code.

\subsection version110 Release version 1.1.0
The HDRL release version 1.1.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.1.0

In this release we added five new algorithms:

- Computation of the Strehl ratio. The Strehl ratio is defined as the
  ratio of the peak image intensity from a point source compared to
  the maximum attainable intensity using an ideal optical system
  limited only by diffraction over the telescope aperture. The Strehl
  ratio is very frequently used to perform the quality control of the
  scientific data obtained with the AO assisted instrumentation.  
- Computation of the <em>spectral efficiency</em> as a function of
  wavelength: The efficiency is used to monitor the system performance
  and health. It is calculated from observing flux standard stars (in
  photometric conditions). Then, the observed 1D spectrum is compared
  with the reference spectrum, as it would be observed outside the
  Earth's atmosphere. The reference spectrum is provided by the user,
  usually via a catalog of standard stars.  
- Computation of the
  <em>spectral response</em> as a function of wavelength: The
  algorithm is divided in two parts: <em>Telluric correction</em> and
  <em>Response calculation</em>. In the provided implementation the
  <em>Telluric correction</em> is optional and can be disabled by the
  user.  
- Computation of the <em>Differential Atmospheric
  Refraction</em> as a function on wavelength. The differential
  atmospheric refraction is calculated according to the algorithm from
  Filippenko (1982, PASP, 94, 715) using the Owens formula which
  converts relative humidity in water vapor pressure.   
- Computation of the <em>effective air mass</em> of an observation.

\subsection version100 Release version 1.0.0
The HDRL release version 1.0.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-1.0.0

In order to provide astrometric and photometric calibration
information, the HDRL implements in this release a functionality to
generate a catalogue of detected objects (i.e. stars, galaxies).

A high-level summary of the implemented data reduction sequence is:

- estimate the local sky background over the image and track any
  variations at adequate resolution to eventually remove them,
- detect objects/blends of objects and keep a list of pixels belonging
  to each blend for further analysis
- parametrise the detected objects, i.e. perform astrometry,
  photometry and a shape analysis.

\subsection version030b1 Release version 0.3.0b1
The HDRL release version 0.3.0b1 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-0.3.0b1

In this release we added an algorithm to do fringe correction. In a
first step the algorithm creates a master-fringe image using a
Gaussian mixture model. A properly scaled version of the master-fringe
image is then used to remove the fringes from the single images.

\subsection version020 Release version 0.2.0
The HDRL release version 0.2.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-0.2.0

In this release we added two algorithms to derive a master flatfield
and one algorithm to compute the Strehl ratio


\subsection version015 Release version 0.1.5
The HDRL release version 0.1.5 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-0.1.5

The sigma clipping algorithm has been changed. It now uses a scaled
Median Absolute Deviation (MAD) to derive a robust RMS for the
clipping and not anymore the interquartile range (IQR). The MAD gives
better results for the case for low number statistics and a high
fraction of pixels affected by e.g. cosmic ray hits.  Furthermore, the
library integration in the pipeline slightly changed.

\subsection version010 Release version 0.1.0

The HDRL release version 0.1.0 can be included from

http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl/hdrl-0.1.0

Various methods for bad pixel detection are added in this release.

\section dependencies Dependencies

Relationship with CPL and other libraries:

The latest hdrl library depends on

- The Common Pipeline Library (CPL) version 7.0 or higher.<em> Please
  note that CPL must be compiled with wcs functionality available
  </em>
- The GSL - GNU Scientific Library version 1.16 or higher.

- The <a href="https://github.com/liberfa/erfa">ERFA</a> (Essential
  Routines for Fundamental Astronomy) library.

- The <a href="https://curl.se/libcurl">libcurl</a> (The multiprotocol
  file transfer library) library

\section usage SVN usage and library integration

Th HDRL library should be included into the pipeline library during
the svn checkout by using the svn::external concept. The external
should point to a given library release in the tags and not to the
trunk, i.e. to a subdirectory of \verbatim
http://svnhq2.hq.eso.org/p2/tags/Pipelines/common/hdrl \endverbatim. In
order to build the library one has to add the <em>hdrl.m4</em> from
the common <em>m4macros</em> repository (<em>Pipelines/common/m4macros
in SVN</em>) to the pipeline sources.

Then one has to modify the <em>configure.ac</em> and the top-level
<em>Makefile.am</em> as follows:

- <em>configure.ac</em>: 
\verbatim 
# mark hdrl folder as containing a configurable package
AC_CONFIG_SUBDIRS([hdrl])

# check and define all required variables to build and
# link hdrl external located in the hdrl folder
HDRL_CHECK([hdrl])
\endverbatim 

- <em>Makefile.am</em>:
\verbatim 
SUBDIRS = hdrl ...
\endverbatim 

Moreover, the variable <em>$(HDRL_LIBS)</em> must also be added to the
link variable, <em>$(HDRL_LDFLAGS)</em> to the linker flag variable,
and <em>$(HDRL_INCLUDES)</em> to the <em>AM_CPPFLAGS</em> variable in
the <em>Makefile.am</em> of any folder making use of HDRL. As HDRL is
currently a static library it also needs has to be added as a
dependency of objects using it so these are relinked when HDRL
changes. For example:

\verbatim 
hdrldemo_bias_la_LDFLAGS = $(HDRL_LDFLAGS) ...
hdrldemo_bias_la_LIBADD = $(HDRL_LIBS) ...
hdrldemo_bias_la_DEPENDENCY = $(LIBHDRL) ...
\endverbatim 

In the source files, the only include needed is:

\verbatim 
#include <hdrl.h>
\endverbatim 

\section hdrldemop The hdrldemo pipeline

In order to test the various algorithms the HDRL team has also written
the hdrldemo pipeline. The pipeline follows the version numbers of the
HDRL release and can be downloaded from a sub-directory of
\verbatim
http://svnhq2.hq.eso.org/p2/tags/Pipelines/hdrldemo/ 
\endverbatim
For example version 1.2.0 can be downloaded from
\verbatim
http://svnhq2.hq.eso.org/p2/tags/Pipelines/hdrldemo/hdrldemo-1.2.0/hdrldemop
\endverbatim

Please note that the pipeline is only meant to test the HDRL library
and the recipes are in most of the cases not ready for operations,
i.e. the HDRL team spends very little resources on the hdrldemo
pipeline.

 */
/*----------------------------------------------------------------------------*/

#ifndef HDRL_H
#define HDRL_H

  #include "hdrl_image.h"
  #include "hdrl_imagelist.h"
  #include "hdrl_parameter.h"
  #include "hdrl_imagelist_view.h"
  #include "hdrl_overscan.h"
  #include "hdrl_buffer.h"
  #include "hdrl_collapse.h"
  #include "hdrl_lacosmics.h"
  #include "hdrl_bpm_2d.h"
  #include "hdrl_bpm_3d.h"
  #include "hdrl_bpm_fit.h"
  #include "hdrl_bpm_utils.h"
  #include "hdrl_fit.h"
  #include "hdrl_strehl.h"
  #include "hdrl_flat.h"
  #include "hdrl_catalogue.h"
  #include "hdrl_random.h"
  #include "hdrl_iter.h"
  #include "hdrl_frameiter.h"
  #include "hdrl_multiiter.h"
  #include "hdrl_fringe.h"
  #include "hdrl_spectrum.h"
  #include "hdrl_spectrumlist.h"
  #include "hdrl_response.h"
  #include "hdrl_efficiency.h"
  #include "hdrl_spectrum_resample.h"
  #include "hdrl_dar.h"
  #include "hdrl_fpn.h"
  #include "hdrl_resample.h"
  #include "hdrl_maglim.h"
  #include "hdrl_barycorr.h"
  #include "hdrl_download.h"
#endif  /* HDRL_H */
