Using the Pipeline for AIT Data Reduction
==========================================

This guide is written for AIT (Assembly, Integration, and Test) engineers who
want to run specific METIS pipeline recipes to reduce data acquired during
instrument testing.

You do not need to understand the internal Python framework to follow this guide.
You only need to know which recipe to run, what input frames it requires, and
what output products it produces.

For a full list of available recipes and their data requirements, see the
:doc:`../reference/index`.

Prerequisites
-------------

* Pipeline installed and environment configured; see :doc:`../getting_started/installation`.
* ``pyesorex --recipes`` lists the METIS recipes without error.
* Your raw data files are available locally.

Workflow overview
-----------------

.. code-block:: text

    Raw FITS files
         │
         ▼
    SOF file (frame tag assignments)
         │
         ▼
    pyesorex <recipe_name> sof_file.sof
         │
         ▼
    Output FITS products (one per DataItem in the recipe's ProductSet)
         │
         ▼
    QC keywords written to primary headers

Step 1: Identify the right recipe
-----------------------------------

Look up the recipe in the :doc:`../reference/index`.  Each recipe page lists:

* The input frame types it expects (with their PRO.CATG / DPR tags)
* The output products it creates
* The QC parameters it computes

For AIT purposes the most commonly used recipes are:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Recipe
     - Purpose
   * - ``metis_det_dark``
     - Create master dark from raw dark frames
   * - ``metis_det_lingain``
     - Measure detector linearity and gain from lamp exposures
   * - ``metis_lm_img_flat``
     - Create master flat-field for LM-band imaging
   * - ``metis_n_img_flat``
     - Create master flat-field for N-band imaging
   * - ``metis_lm_basic_reduction``
     - Basic science reduction: dark subtract, flat field, bad-pixel mask

Step 2: Prepare the input FITS files
--------------------------------------

Verify that your frames carry the correct DPR header keywords.  For example,
dark frames must have::

    ESO DPR TYPE  = 'DARK'
    ESO DPR CATG  = 'CALIB'

Check the recipe reference page for the exact keywords matched.

Step 3: Create the SOF file
----------------------------

A SOF (Set-Of-Frames) file is a plain text file where each line assigns a frame
to a role::

    /path/to/dark_001.fits    DARK_2RG_RAW
    /path/to/dark_002.fits    DARK_2RG_RAW
    /path/to/dark_003.fits    DARK_2RG_RAW

The second column is the frame **tag**.  The allowed tags for each input are
listed on the recipe reference page (and in the ``pyesorex --man-page`` output).

If you have both detector types in your data, create separate SOF files — one
per detector variant.

Step 4: Run the recipe
-----------------------

::

    pyesorex metis_det_dark my_darks.sof

Output products are written to the current directory with names like::

    MASTER_DARK_2RG_2024-06-01T12-00-00-000000.fits

To change the output directory::

    pyesorex --output-dir /results/ metis_det_dark my_darks.sof

To override a recipe parameter::

    pyesorex --recipe-config="metis_det_dark.stacking.method=median" \
        metis_det_dark my_darks.sof

To see all available parameters::

    pyesorex --man-page metis_det_dark

Step 5: Chain recipes
----------------------

Products from one recipe feed into the next.  Create new SOF files including
the products you just created plus any additional raw frames:

.. code-block:: text

    # flat_sof.sof
    /data/flat_001.fits         FLAT_LM_2RG_RAW
    /data/flat_002.fits         FLAT_LM_2RG_RAW
    /results/MASTER_DARK_2RG_<timestamp>.fits    MASTER_DARK_2RG

Then run::

    pyesorex metis_lm_img_flat flat_sof.sof

Step 6: Inspect the output
---------------------------

Use any FITS viewer (e.g. ``ds9``, ``QFitsView``) or ``astropy``::

    from astropy.io import fits
    hdul = fits.open('MASTER_DARK_2RG_<timestamp>.fits')
    hdul.info()
    # HDU 0: PRIMARY (header only)
    # HDU 1: DET1.SCI (Image 2048×2048)
    # HDU 2: DET1.ERR (Image 2048×2048)
    # HDU 3: DET1.DQ  (Image 2048×2048)

QC values are in the primary header::

    from astropy.io import fits
    hdr = fits.getheader('MASTER_DARK_2RG_<timestamp>.fits', 0)
    print(hdr['ESO QC DARK MEAN'])

Running with EDPS (automated cascade)
--------------------------------------

For a complete calibration cascade, EDPS is more convenient than calling each
recipe manually:

.. code-block:: bash

    # List available tasks for your data
    edps -w metis.metis_lm_img_wkf -i /data/sof_dir -lt

    # Run a specific recipe
    edps -w metis.metis_lm_img_wkf -i /data/sof_dir -t metis_det_dark

    # Run the full science chain
    edps -w metis.metis_wkf -i /data/sof_dir -m science

EDPS automatically resolves the dependencies between recipes and runs them in
the correct order.  See the `EDPS documentation <https://www.eso.org/sci/software/edps.html>`_
for full details.
