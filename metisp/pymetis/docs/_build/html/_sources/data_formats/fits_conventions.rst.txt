FITS Conventions
================

All ``pymetis`` data products are multi-extension FITS (MEF) files following
the ESO pipeline conventions.  This page describes the naming conventions,
mandatory keywords, and extension structures used throughout the METIS pipeline.

Extension naming
----------------

Extension names (``EXTNAME`` keyword) follow the pattern::

    DET<n>.<role>

where:

* ``<n>`` is the detector index (``1`` for 2RG/GEO; ``1``–``4`` for IFU)
* ``<role>`` is one of:

  .. list-table::
     :header-rows: 1
     :widths: 15 85

     * - Role
       - Content
     * - ``SCI``
       - Science data (detector counts / flux / wavelength-calibrated signal)
     * - ``ERR``
       - Uncertainty / noise map (same units as ``SCI``)
     * - ``DQ``
       - Data quality / bad-pixel mask.
         ``0`` = good pixel; non-zero = flagged (see below for bit definitions)
     * - ``DATA``
       - Raw uncalibrated data (used for raw frames)

Single-detector products (2RG, GEO) have one set of extensions::

    PRIMARY   (header only)
    DET1.SCI
    DET1.ERR
    DET1.DQ

IFU products have one set per quadrant (four detectors)::

    PRIMARY
    DET1.SCI   DET1.ERR   DET1.DQ
    DET2.SCI   DET2.ERR   DET2.DQ
    DET3.SCI   DET3.ERR   DET3.DQ
    DET4.SCI   DET4.ERR   DET4.DQ

DQ bit definitions
~~~~~~~~~~~~~~~~~~

The ``DQ`` plane uses bitmask flags.  Bit 0 (value 1) is the general bad-pixel
flag set by the master dark recipe.  Additional bits are defined per recipe as
the pipeline matures.

Mandatory primary header keywords
-----------------------------------

These keywords are written to every product primary header by the pipeline:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Keyword
     - Content
   * - ``ESO PRO CATG``
     - Product category tag — the fully-resolved ``DataItem.name()`` string,
       e.g. ``MASTER_DARK_2RG``
   * - ``ESO PRO REC1 ID``
     - Name of the recipe that produced this file
   * - ``ESO PRO REC1 DRS ID``
     - Version of ``pymetis`` used
   * - Provenance keywords
     - ``ESO PRO REC1 RAW<n> NAME`` / ``TAG`` for each input frame used

OCA / DPR keywords (raw frames only)
--------------------------------------

Raw frames are characterised by the OB-level DPR keywords set by the
instrument control software:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Keyword
     - Meaning
   * - ``ESO DPR CATG``
     - ``CALIB`` or ``SCIENCE``
   * - ``ESO DPR TECH``
     - Observing technique: ``IMAGE``, ``LSS``, ``IFU``, etc.
   * - ``ESO DPR TYPE``
     - Exposure type: ``DARK``, ``FLAT``, ``STD``, ``OBJECT``, etc.

These three keywords together form the OCA classification triplet used by EDPS
to route raw frames to the correct recipe.

Data types
----------

All science / calibration images are stored as 32-bit floats (``BITPIX = -32``).
Bad-pixel masks are stored as 32-bit integers (``BITPIX = 32``).
Spectral tables use binary FITS tables (``XTENSION = BINTABLE``).
