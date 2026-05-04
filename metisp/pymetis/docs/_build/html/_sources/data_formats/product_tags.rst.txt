Product Tags (PRO.CATG)
=======================

Every pipeline product has a unique string identifier called its **tag** (stored
in the FITS keyword ``ESO PRO CATG``).  Tags are used by the pipeline to:

* Match output products to the inputs of downstream recipes.
* Identify the data format (FITS structure) of a file.
* Provide the link between the Python class hierarchy and the DRLD data-model.

How tags are derived
---------------------

Tags are computed from a ``DataItem`` class's ``_name_template`` attribute by
filling in all ``{placeholder}`` slots with their tag-parameter values:

.. code-block:: python

    class MasterDark(ImageDataItem, abstract=True):
        _name_template = r'MASTER_DARK_{detector}'

    class MasterDark2rg(Detector2rgMixin, MasterDark):
        pass

    MasterDark2rg.name()   # → 'MASTER_DARK_2RG'
    MasterDarkGeo.name()   # → 'MASTER_DARK_GEO'
    MasterDarkIfu.name()   # → 'MASTER_DARK_IFU'

Tag naming conventions
----------------------

METIS product tags follow the pattern::

    <PRODUCT_KIND>[_<DETECTOR>][_<BAND>][_<TARGET>][_<QUALIFIER>]

Common segments:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Segment
     - Meaning
   * - ``2RG``
     - 2RG Hawaii-2RG detector (LM and N band imaging)
   * - ``GEO``
     - GeoSnap detector (N band imaging)
   * - ``IFU``
     - IFU detector array
   * - ``LM``
     - L/M band (3–5 µm)
   * - ``N``
     - N band (8–13 µm)
   * - ``RAW``
     - Raw, uncalibrated frame (input to calibration recipes)
   * - ``SCI`` / ``STD``
     - Science target / standard star

Examples::

    DARK_2RG_RAW          ← raw dark frames for the 2RG detector
    MASTER_DARK_2RG       ← combined master dark for the 2RG detector
    BADPIX_MAP_GEO        ← bad-pixel mask for the GEO detector
    MASTER_FLAT_2RG_LM    ← master flat for 2RG in LM band
    LM_BASIC_REDUCED_SCI  ← basic-reduced LM-band science frame

SOF file usage
--------------

Tags appear as the second column of a Set-Of-Frames (SOF) file::

    /data/raw/dark_001.fits    DARK_2RG_RAW
    /data/raw/dark_002.fits    DARK_2RG_RAW
    /data/cal/badpix.fits      BADPIX_MAP_2RG

Each tag must exactly match the ``_name_template`` of the ``PipelineInput.Item``
class declared in the recipe's ``InputSet``.

For the full list of registered tags and their FITS structures, see
:doc:`data_item_catalog`.
