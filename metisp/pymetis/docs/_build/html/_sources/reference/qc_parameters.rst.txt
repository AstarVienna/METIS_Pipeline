QC Parameters Reference
========================

This page lists all QC (Quality Control) parameters produced by the METIS
pipeline recipes, mirroring the QC parameter tables in the DRLD.

QC values are written to the primary FITS header of each output product under
``ESO QC`` keywords.

Detector calibration QC parameters
-------------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 10 15 45

   * - Name
     - Type
     - Unit
     - Description
   * - ``QC DARK MEAN``
     - float
     - Counts
     - Mean pixel value of the combined master dark
   * - ``QC DARK MEDIAN``
     - float
     - Counts
     - Median pixel value of the combined master dark
   * - ``QC DARK RMS``
     - float
     - Counts
     - RMS noise of the combined master dark
   * - ``QC DARK NBADPIX``
     - int
     - Pixels
     - Total number of flagged bad pixels
   * - ``QC DARK NCOLDPIX``
     - int
     - Pixels
     - Number of pixels flagged as cold (below threshold)
   * - ``QC DARK NHOTPIX``
     - int
     - Pixels
     - Number of pixels flagged as hot (above threshold)
   * - ``QC LIN GAIN MEAN``
     - float
     - e-/ADU
     - Mean gain value across the detector
   * - ``QC LIN NUM BADPIX``
     - int
     - Pixels
     - Number of pixels with non-linear response flagged by the gain recipe
   * - ``QC PERSIST COUNT``
     - int
     - Pixels
     - Number of pixels affected by persistence

.. todo::

   Auto-generate this table from ``QcParameter._registry`` using the
   ``autorecipe`` Sphinx extension.
