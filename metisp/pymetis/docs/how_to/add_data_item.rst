How to Add a New DataItem
=========================

This guide walks through defining a new ``DataItem`` subclass — the Python
class that describes a FITS file type (input or output) in the pipeline.

You need a new ``DataItem`` whenever you are introducing a new kind of FITS file:
a new calibration product, a new intermediate product, or a new raw frame type.

Prerequisites
-------------

* You understand :doc:`../concepts/data_items` and
  :doc:`../concepts/parametrization`.
* You know the FITS extension structure of the new product (from the DRLD or your
  own design).

Step 1: Choose the base class
------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Base class
     - Use when
   * - ``ImageDataItem``
     - All or most extensions contain image data (``cpl.core.Image``)
   * - ``TableDataItem``
     - The primary data extension is a binary table (``cpl.core.Table``)
   * - ``DataItem``
     - Mixed content, or if you need full control

All three are in ``pymetis.engine.dataitems``.

Step 2: Create the file
------------------------

Put the new class in a sensible location under::

    src/pymetis/instruments/metis/dataitems/<category>/<product_name>.py

For example, a new background map product::

    src/pymetis/instruments/metis/dataitems/background/background.py

Step 3: Write the abstract base class
--------------------------------------

Start with an abstract base that uses template placeholders for the parts that
vary by detector, band, or target:

.. code-block:: python

    import cpl
    from cpl.core import Image

    from pymetis.engine.dataitems.image import ImageDataItem


    class BackgroundMap(ImageDataItem, abstract=True):
        _name_template        = r'LM_BACKGROUND_MAP_{detector}'
        _title_template       = r'{detector} LM-band background map'
        _description_template = 'Background map derived from LM-band science frames ({detector})'

        _frame_group = cpl.ui.Frame.FrameGroup.CALIB
        _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        _schema = {
            'PRIMARY':  None,
            'DET1.SCI': Image,
            'DET1.ERR': Image,
            'DET1.DQ':  Image,
        }

Rules for ``_name_template``:

* Use ``{detector}``, ``{band}``, ``{target}``, ``{source}`` as placeholders.
* Only UPPER-CASE letters, digits, and underscores are allowed in the final
  resolved name (the regex ``^[A-Z]+[A-Z0-9_]+[A-Z0-9]+$`` is checked at
  instantiation time).
* Keep names consistent with the DRLD data-item table.

Rules for ``_schema``:

* Every extension your recipe produces **must** appear in ``_schema``.
* Extensions not listed will cause a ``ValueError`` at instantiation.
* The data type (``Image``, ``Table``, or ``None``) must match what you pass
  to the constructor.

Step 4: Add DRLD-traceability attributes (optional but recommended)
--------------------------------------------------------------------

.. code-block:: python

        _dpr_catg = 'CALIB'
        _dpr_tech = 'IMAGE'
        _dpr_type = 'BACKGROUND'

These mirror the FITS DPR keyword triplet from the DRLD and are used by the
``autorecipe`` Sphinx extension to generate the DRLD JSON manifest.

Step 5: Create concrete specializations
-----------------------------------------

For each detector (or band / target) variant, create a concrete subclass by
mixing in the appropriate mixin:

.. code-block:: python

    from pymetis.instruments.metis.mixins.detector import Detector2rgMixin, DetectorGeoMixin

    class BackgroundMap2rg(Detector2rgMixin, BackgroundMap):
        pass   # name() → 'LM_BACKGROUND_MAP_2RG'

    class BackgroundMapGeo(DetectorGeoMixin, BackgroundMap):
        pass   # name() → 'LM_BACKGROUND_MAP_GEO'

If the schema differs between variants (e.g. the IFU has four detector
quadrants), override ``_schema`` in the concrete subclass:

.. code-block:: python

    from pymetis.instruments.metis.mixins.detector import DetectorIfuMixin

    class BackgroundMapIfu(DetectorIfuMixin, BackgroundMap):
        _schema = {
            'PRIMARY': None,
            **{f'DET{d:1d}.{e}': Image
               for d in [1, 2, 3, 4]
               for e in ['SCI', 'ERR', 'DQ']},
        }

Step 6: Export the new classes
--------------------------------

Add an import to the ``__init__.py`` of your category directory so the classes
are reachable and auto-register themselves:

.. code-block:: python

    # src/pymetis/instruments/metis/dataitems/background/__init__.py
    from .background import BackgroundMap, BackgroundMap2rg, BackgroundMapGeo, BackgroundMapIfu

Also ensure the parent ``dataitems/__init__.py`` imports the sub-package.

Step 7: Use the new DataItem in a recipe
-----------------------------------------

Reference the new class in your ``RecipeImpl.ProductSet``:

.. code-block:: python

    class ProductSet(PipelineProductSet):
        Background = BackgroundMap   # will be promoted to BackgroundMap2rg etc. at runtime

Step 8: Document the new DataItem
----------------------------------

Add an entry to :doc:`../data_formats/data_item_catalog`.

To auto-generate a reference page using the ``autorecipe`` extension, add::

    .. autodataitem:: pymetis.instruments.metis.dataitems.background.background.BackgroundMap2rg

to any RST page in ``docs/reference/`` or ``docs/data_formats/``.

Summary checklist
------------------

.. list-table::
   :header-rows: 0

   * - ☐ Abstract base class with ``_name_template``, ``_title_template``,
       ``_description_template``, ``_frame_group``, ``_frame_level``, ``_schema``
   * - ☐ Concrete subclasses for each detector/band/target variant (via mixins)
   * - ☐ DRLD attributes: ``_dpr_catg``, ``_dpr_tech``, ``_dpr_type``
   * - ☐ Imported in the ``dataitems`` sub-package ``__init__.py``
   * - ☐ Referenced in ``RecipeImpl.ProductSet``
   * - ☐ Entry added to :doc:`../data_formats/data_item_catalog`
   * - ☐ Tests verify the schema contract (correct extension names and types)
