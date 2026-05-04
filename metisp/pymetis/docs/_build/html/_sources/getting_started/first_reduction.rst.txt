First Data Reduction (AIT Quick-Start)
=======================================

This guide walks an AIT engineer through running a single pipeline recipe on
real detector data.  We use ``metis_det_dark`` — the master dark recipe — as the
example, since it requires only raw dark frames and no prior calibration products.

.. note::

   Make sure you have completed the :doc:`installation` steps first.

Overview
--------

The pipeline is driven by a **Set-Of-Frames (SOF) file** — a plain-text file that
lists the input FITS files and their roles.  You pass this to ``pyesorex``, which
runs the recipe and writes output products to the current directory.

For automated multi-recipe workflows (e.g. running the full calibration cascade)
use EDPS; see :ref:`running-with-edps` below.

Step 1: Prepare your input data
--------------------------------

Collect your raw dark frames in a directory, e.g. ``/data/darks/``.  Each file
must have the correct FITS header keyword ``ESO DPR TYPE = DARK``.

Step 2: Create a SOF file
--------------------------

A SOF file has one line per frame::

    /data/darks/dark_001.fits   DARK_2RG_RAW
    /data/darks/dark_002.fits   DARK_2RG_RAW
    /data/darks/dark_003.fits   DARK_2RG_RAW

The second column is the frame **tag** (``PRO.CATG``).  Valid tags for
``metis_det_dark`` are:

* ``DARK_2RG_RAW`` — 2RG detector (LM/N band imaging)
* ``DARK_GEO_RAW`` — GEO detector (N band imaging)
* ``DARK_IFU_RAW`` — IFU detector

Step 3: Run the recipe
-----------------------

::

    pyesorex metis_det_dark my_darks.sof

By default the recipe stacks frames using the average.  To use median stacking::

    pyesorex --recipe-config=metis_det_dark.stacking.method=median \
        metis_det_dark my_darks.sof

To list all available parameters::

    pyesorex --man-page metis_det_dark

Step 4: Examine the output
---------------------------

``pyesorex`` writes the master dark to a FITS file in the current directory, e.g.::

    MASTER_DARK_2RG_2024-06-01T12-00-00-000000.fits

The file has the following FITS extension structure (see
:doc:`../reference/detector/metis_det_dark` for the full data-format specification):

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Extension
     - Content
   * - ``PRIMARY``
     - Empty data; header carries provenance keywords (``ESO PRO CATG``, etc.)
   * - ``DET1.SCI``
     - Combined dark image (counts, float32)
   * - ``DET1.ERR``
     - Propagated noise estimate
   * - ``DET1.DQ``
     - Bad-pixel mask (0 = good, non-zero = flagged)

The ``ESO PRO CATG`` keyword in the primary header will be set to
``MASTER_DARK_2RG``, which is the tag the pipeline uses when it looks for this
product as an input to subsequent recipes.

QC parameter values are written to the primary header under ``ESO QC`` keywords,
e.g. ``ESO QC DARK MEAN``.

.. _running-with-edps:

Running with EDPS
-----------------

For full-cascade reduction (dark → flat → science) use EDPS::

    edps -w metis.metis_lm_img_wkf -i /data/my_sof_dir -t metis_det_dark

Common EDPS commands::

    # List available recipes in the workflow
    edps -w metis.metis_wkf -i /data/my_sof_dir -lt

    # Run the full science chain
    edps -w metis.metis_wkf -i /data/my_sof_dir -m science

    # Clear all cached state (useful during debugging)
    edps -shutdown && rm -rf edps.log pyesorex.log $HOME/EDPS_data/*

Troubleshooting
---------------

**Recipe not found**
    Check that ``PYESOREX_PLUGIN_DIR`` and ``PYCPL_RECIPE_DIR`` are set and point
    to ``metisp/pyrecipes/``.

**No frames match**
    Verify the frame tags in your SOF file match what the recipe expects.  Run
    ``pyesorex --man-page metis_det_dark`` to see the accepted tags for each input.

**Empty output**
    Check that ``ESO PRO CATG`` (for calibration data) or ``ESO DPR TYPE`` (for raw
    data) is set correctly in your FITS headers.
