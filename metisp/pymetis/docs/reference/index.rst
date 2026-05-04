Recipe Reference
================

This section provides a structured reference for every METIS pipeline recipe,
mirroring the layout of the DRLD (Data Reduction Library Design document).

Each recipe page is generated directly from the Python source code via the
``autorecipe`` Sphinx extension, so the information here stays in sync with
the implementation automatically.

.. tip::

   A machine-readable JSON version of each recipe's data (inputs, outputs,
   algorithm, QC parameters) is written to ``_build/drld_manifest/recipes/``
   during the documentation build.  This manifest can be used to scaffold
   or update the DRLD LaTeX source.

.. toctree::
   :maxdepth: 1
   :caption: Detector Calibration

   detector/index

.. toctree::
   :maxdepth: 1
   :caption: LM-Band Imaging

   imaging/index

.. toctree::
   :maxdepth: 1
   :caption: Spectroscopy (LSS and IFU)

   spectroscopy/index

.. toctree::
   :maxdepth: 1
   :caption: High-Contrast Imaging

   hci/index

.. toctree::
   :maxdepth: 1

   qc_parameters
