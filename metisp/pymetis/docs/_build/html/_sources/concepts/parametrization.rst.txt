The Tag / Parametrization System
=================================

Many pipeline components in ``pymetis`` exist in multiple variants: one for
each detector (``2RG``, ``GEO``, ``IFU``), one for each band (``LM``, ``N``),
one per reduction target (``SCI``, ``STD``), etc.

Rather than duplicating classes, ``pymetis`` uses a **tag-based parametrization
system** that lets you define a generic class once and then derive concrete
variants via lightweight mixins.

Core concepts
-------------

**Tag parameters**
    Short key-value pairs that identify a variant, e.g.
    ``detector='2RG'``, ``band='LM'``, ``target='SCI'``.

**Name template**
    A string with ``{placeholder}`` slots, e.g.
    ``MASTER_DARK_{detector}``.  Filling all placeholders gives the final
    machine-readable name / PRO.CATG tag.

**Specialization** (static, at class-definition time)
    A mixin injects a tag value at class creation.
    ``MasterDark2rg(Detector2rgMixin, MasterDark)`` fills ``{detector}`` = ``2RG``,
    producing the name ``MASTER_DARK_2RG``.

**Promotion** (dynamic, at runtime from loaded data)
    When a frame arrives whose tag is partially known (e.g. the target type
    ``SCI`` vs ``STD`` can only be determined from the data header),
    ``promote()`` looks up the fully-resolved name in the global registry and
    returns the correct class.

How specialization works
------------------------

Every class that participates in the tag system inherits from
:class:`~pymetis.engine.core.parametrizable.Parametrizable`.

Mixins inject tag values by passing keyword arguments to the class definition:

.. code-block:: python

    class Detector2rgMixin(Parametrizable, detector='2RG'):
        pass

    class BandLmMixin(Parametrizable, band='LM'):
        pass

    class TargetSciMixin(Parametrizable, target='SCI'):
        pass

Concrete classes inherit from a combination of mixins and a generic base:

.. code-block:: python

    class MasterFlat2rgLm(Detector2rgMixin, BandLmMixin, MasterFlat):
        pass
    # name() → 'MASTER_FLAT_2RG_LM'

Python's MRO ensures that each mixin's ``_tag_parameters`` dict is merged
correctly.  Tags defined further up the inheritance chain can be overridden
by more derived classes.

Querying tag parameters
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

    MasterDark2rg.tag_parameters()
    # → {'detector': '2RG'}

    MasterFlat2rgLm.tag_parameters()
    # → {'detector': '2RG', 'band': 'LM'}

    MasterDark.name()
    # → 'MASTER_DARK_{detector}'   (placeholder still present → abstract)

    MasterDark2rg.name()
    # → 'MASTER_DARK_2RG'          (fully resolved → concrete)

The global registry
-------------------

Every ``ParametrizableItem`` subclass (``DataItem``, ``QcParameter``, …)
is registered in a class-level dict:

.. code-block:: python

    DataItem._registry
    # → {'MASTER_DARK_2RG': MasterDark2rg,
    #    'MASTER_DARK_GEO': MasterDarkGeo,
    #    'MASTER_DARK_IFU': MasterDarkIfu,
    #    ...}

Registration is automatic and happens in ``__init_subclass__`` at import time.
To look up a class by tag name::

    cls = DataItem.find('MASTER_DARK_2RG')   # returns MasterDark2rg or None

How promotion works
-------------------

Promotion is used when the final tag value can only be determined at runtime
(typically from the header of a loaded frame).

After the ``InputSet`` has loaded its frames, ``RecipeImpl.__init__`` calls::

    self.promote(**self.inputset.tag_matches)

``tag_matches`` is a dict like ``{'target': 'SCI'}`` inferred from the loaded
frames.  ``promote()`` propagates this to the ``ProductSet`` and ``Qc``
containers, which swap their generic product/QC classes for the matching
specialized subclasses from the registry.

.. code-block:: text

    # Before promotion:
    self.ProductSet.MasterFlat  ->  MasterFlat   (has {band}, {detector} templates)

    # After promote(band='LM', detector='2RG'):
    self.ProductSet.MasterFlat  ->  MasterFlat2rgLm   (fully resolved)

Partial specialization
-----------------------

It is valid for a class to have some — but not all — placeholders filled.
Such a class is considered **partially specialized** (and therefore abstract).
Attempting to instantiate it raises a ``TypeError``.

Partial specialization is useful for defining intermediate abstract bases, e.g.
an LM-band base class where the detector is still generic:

.. code-block:: python

    class MasterFlatLm(BandLmMixin, MasterFlat, abstract=True):
        pass
    # name() → 'MASTER_FLAT_{detector}_LM'

Available METIS tags
--------------------

.. list-table::
   :header-rows: 1
   :widths: 20 30 50

   * - Tag key
     - Values
     - Mixins
   * - ``detector``
     - ``2RG``, ``GEO``, ``IFU``
     - ``Detector2rgMixin``, ``DetectorGeoMixin``, ``DetectorIfuMixin``
   * - ``band``
     - ``LM``, ``N``, ``IFU``
     - ``BandLmMixin``, ``BandNMixin``, ``BandIfuMixin``
   * - ``target``
     - ``SCI``, ``STD``
     - ``TargetSciMixin``, ``TargetStdMixin``
   * - ``source``
     - ``LAMP``, ``TWILIGHT``
     - ``SourceLampMixin``, ``SourceTwilightMixin``
   * - ``cgrph``
     - coronagraph variant
     - Various coronagraph mixins

All mixins live in ``pymetis.instruments.metis.mixins``.
