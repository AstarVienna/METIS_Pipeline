Data Item Catalog
=================

This page lists all ``DataItem`` subclasses registered in the METIS pipeline.
Each entry corresponds to one type of FITS file (either an input frame type or
an output product) and mirrors the data-structure entries in the DRLD.

.. note::

   This catalog is a living document.  As the pipeline matures, new data items
   will be added here.  The ``autorecipe`` Sphinx extension can generate a
   machine-readable JSON version of this catalog — see
   ``_build/drld_manifest/dataitems/`` after a documentation build.

.. tip::

   Use ``DataItem.find('<TAG>')`` at the Python prompt to look up the class
   corresponding to any PRO.CATG tag.

Detector calibration products
-------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 10 10 50

   * - PRO.CATG tag
     - Detector
     - Level
     - FITS extensions (name → type)
   * - ``MASTER_DARK_2RG``
     - 2RG
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)
   * - ``MASTER_DARK_GEO``
     - GEO
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)
   * - ``MASTER_DARK_IFU``
     - IFU
     - FINAL
     - PRIMARY, DET1–4.SCI/ERR/DQ (12 extensions total)
   * - ``BADPIX_MAP_2RG``
     - 2RG
     - FINAL
     - PRIMARY, DET1.SCI (Image)
   * - ``BADPIX_MAP_GEO``
     - GEO
     - FINAL
     - PRIMARY, DET1.SCI (Image)
   * - ``BADPIX_MAP_IFU``
     - IFU
     - FINAL
     - PRIMARY, DET1.SCI (Image)
   * - ``LINEARITY_2RG``
     - 2RG
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image)
   * - ``GAIN_MAP_2RG``
     - 2RG
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image)

LM-band imaging products
--------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 10 10 50

   * - PRO.CATG tag
     - Detector
     - Level
     - FITS extensions
   * - ``MASTER_FLAT_2RG_LM``
     - 2RG
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)
   * - ``LM_BASIC_REDUCED_SCI``
     - 2RG
     - INTERMEDIATE
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)
   * - ``LM_BASIC_REDUCED_STD``
     - 2RG
     - INTERMEDIATE
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)

N-band imaging products
------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 10 10 50

   * - PRO.CATG tag
     - Detector
     - Level
     - FITS extensions
   * - ``MASTER_FLAT_GEO_N``
     - GEO
     - FINAL
     - PRIMARY, DET1.SCI (Image), DET1.ERR (Image), DET1.DQ (Image)

IFU products
------------

.. list-table::
   :header-rows: 1
   :widths: 30 10 10 50

   * - PRO.CATG tag
     - Detector
     - Level
     - FITS extensions
   * - ``IFU_WAVECAL``
     - IFU
     - INTERMEDIATE
     - PRIMARY, DET1–4.SCI/ERR/DQ
   * - ``IFU_REDUCED_SCI``
     - IFU
     - INTERMEDIATE
     - PRIMARY, DET1–4.SCI/ERR/DQ

Raw frame types
---------------

These are the input frame tags expected by calibration and science recipes.
They are declared as the ``Item`` class of a ``PipelineInput``.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Tag
     - Description
   * - ``DARK_2RG_RAW``
     - Raw dark exposure from the 2RG detector
   * - ``DARK_GEO_RAW``
     - Raw dark exposure from the GEO detector
   * - ``DARK_IFU_RAW``
     - Raw dark exposure from the IFU detector array
   * - ``DETLIN_2RG_RAW``
     - Raw linearity/gain calibration exposure (2RG)
   * - ``FLAT_LM_2RG_RAW``
     - Raw flat-field exposure in LM band (2RG detector)
   * - ``FLAT_N_GEO_RAW``
     - Raw flat-field exposure in N band (GEO detector)
   * - ``LM_SCI_RAW``
     - Raw LM-band science exposure
   * - ``IFU_SCI_RAW``
     - Raw IFU science exposure
   * - ``LSS_SCI_RAW``
     - Raw LSS science exposure

.. todo::

   Auto-generate this table from the ``DataItem._registry`` at build time using
   the ``autorecipe`` Sphinx extension (``autodataitem`` directive).
