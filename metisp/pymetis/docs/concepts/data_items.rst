Data Items
==========

A :class:`~pymetis.engine.dataitems.DataItem` is the fundamental unit of
pipeline data in ``pymetis``.  Each ``DataItem`` subclass represents exactly one
type of FITS file — either an input frame or an output product.

The class hierarchy mirrors the DRLD data structure catalog:
every entry in the DRLD's data-item tables corresponds to (at most) one
fully-specialized ``DataItem`` subclass.

Key class attributes
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Attribute
     - Meaning
   * - ``_name_template``
     - Machine-readable name / PRO.CATG tag, e.g. ``MASTER_DARK_{detector}``.
       Template placeholders (``{detector}``) are filled by tag parameters.
   * - ``_title_template``
     - Human-readable title, e.g. ``"{detector} master dark"``.
   * - ``_description_template``
     - Verbose description (used in man pages and DRLD).
   * - ``_schema``
     - Dict mapping FITS extension names to their data type.
       ``None`` → header-only; ``Image`` → image data; ``Table`` → binary table.
   * - ``_frame_group``
     - CPL ``FrameGroup``: ``RAW``, ``CALIB``, or ``SCIENCE``.
   * - ``_frame_level``
     - CPL ``FrameLevel``: ``RAW``, ``INTERMEDIATE``, or ``FINAL``.
   * - ``_oca_keywords``
     - Set of FITS keywords used by OCA (Pipeline Association Rules) for
       frame classification.

The schema
----------

The ``_schema`` attribute defines the FITS extension structure of the file:

.. code-block:: python

    class MasterDark(ImageDataItem, abstract=True):
        _name_template       = r'MASTER_DARK_{detector}'
        _title_template      = r'{detector} master dark'
        _description_template = 'Master dark frame for {detector} data'
        _frame_group = cpl.ui.Frame.FrameGroup.CALIB
        _frame_level = cpl.ui.Frame.FrameLevel.FINAL

        _schema = {
            'PRIMARY':  None,     # header only — no image data
            'DET1.SCI': Image,    # combined dark image
            'DET1.ERR': Image,    # noise map
            'DET1.DQ':  Image,    # data quality / bad-pixel mask
        }

When a ``DataItem`` instance is created, the framework checks that every
:class:`~pymetis.engine.dataitems.Hdu` passed to the constructor has an
extension name listed in ``_schema`` and has the correct data type.

Individual extensions are accessed by name::

    hdu = product['DET1.SCI']   # returns an Hdu object
    image = product.load_data('DET1.SCI')  # loads the actual pixel array

The Hdu wrapper
---------------

:class:`~pymetis.engine.dataitems.Hdu` is a lightweight struct that wraps one
FITS extension:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Attribute
     - Content
   * - ``name``
     - FITS ``EXTNAME`` string (e.g. ``DET1.SCI``)
   * - ``klass``
     - ``Image``, ``Table``, or ``None`` (primary header)
   * - ``header``
     - ``cpl.core.PropertyList`` — the extension FITS header
   * - ``data``
     - Pixel data (``cpl.core.Image`` or ``cpl.core.Table``), or ``None`` if
       not yet loaded
   * - ``extno``
     - Integer HDU index within the FITS file

How DataItems are registered
-----------------------------

Every ``DataItem`` subclass automatically registers itself in a global
dictionary (:attr:`~pymetis.engine.core.parametrizable.ParametrizableItem._registry`)
keyed by its fully-resolved name.  The name is computed by filling all
template placeholders with the class's tag parameters (see :doc:`parametrization`).

For example::

    class MasterDark2rg(Detector2rgMixin, MasterDark):
        pass
    # After class creation:
    # DataItem._registry['MASTER_DARK_2RG'] == MasterDark2rg

When a frame arrives with tag ``MASTER_DARK_2RG``, the pipeline calls
``DataItem.find('MASTER_DARK_2RG')`` to retrieve the class and then
instantiates it.

Specialization via mixins
--------------------------

Generic ``DataItem`` classes use ``{placeholder}`` templates.  Concrete
classes are created by inheriting from both a mixin (which supplies the tag
value) and the generic class:

.. code-block:: python

    class Detector2rgMixin(Parametrizable, detector='2RG'):
        pass

    class MasterDark2rg(Detector2rgMixin, MasterDark):
        _schema = {
            'PRIMARY':  None,
            'DET1.SCI': Image,
            'DET1.ERR': Image,
            'DET1.DQ':  Image,
        }

    class MasterDarkIfu(DetectorIfuMixin, MasterDark):
        _schema = {
            'PRIMARY': None,
            **{f'DET{d:1d}.{e}': Image
               for d in [1, 2, 3, 4]
               for e in ['SCI', 'ERR', 'DQ']},
        }

The IFU variant overrides ``_schema`` because it has four detector quadrants,
each with three extensions.

For a complete reference of all registered METIS ``DataItem`` classes, see
:doc:`../data_formats/data_item_catalog`.

For step-by-step instructions on adding a new data item, see
:doc:`../how_to/add_data_item`.
