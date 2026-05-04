Pipeline Inputs
===============

Input frames are declared inside a ``RecipeImpl.InputSet`` using
:class:`~pymetis.engine.inputs.PipelineInput` subclasses.  Each
``PipelineInput`` attribute of the ``InputSet`` corresponds to one logical group
of input frames (e.g. "the raw darks" or "the bad-pixel map").

Input classes
-------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Class
     - Use when
   * - :class:`~pymetis.engine.inputs.SinglePipelineInput`
     - Exactly one frame of this type is expected (``_multiplicity = '1'``)
   * - :class:`~pymetis.engine.inputs.MultiplePipelineInput`
     - One or more frames of this type are expected (``_multiplicity = 'N'``)

Declaring inputs
----------------

.. code-block:: python

    class InputSet(RawImageProcessor.InputSet):

        class RawInput(MultiplePipelineInput):
            Item = DarkRaw           # DataItem subclass that describes the frame format

        class BadPixMapInput(OptionalInputMixin, SinglePipelineInput):
            Item = BadPixMap         # optional: will not fail if absent

        class LinearityInput(OptionalInputMixin, SinglePipelineInput):
            Item = LinearityMap

The ``Item`` class variable points to a :class:`~pymetis.engine.dataitems.DataItem`
subclass.  The framework uses ``Item.name()`` to match incoming frames by their
``PRO.CATG`` tag (or ``DPR TYPE`` for raw frames).

Making an input optional
~~~~~~~~~~~~~~~~~~~~~~~~~

Mix in ``OptionalInputMixin`` to declare that the frame type is not required.
The recipe will still work if no matching frame is present in the SOF file.

.. code-block:: python

    class PersistenceMapInput(OptionalInputMixin, SinglePipelineInput):
        Item = PersistenceMap

Accessing input data in ``process()``
--------------------------------------

Inside ``process()``, inputs are available through ``self.inputset``.
Attribute names match the nested class names (lower-cased by default)::

    # Load structure (headers) — cheap
    self.inputset.raw.load_structure()

    # Load pixel data for one extension — deferred, can be expensive
    images = self.inputset.raw.load_data('DET1.DATA')

    # Access the underlying DataItem
    item = self.inputset.raw.item

Lazy loading
~~~~~~~~~~~~

``load_structure()`` reads headers only; pixel data is not loaded into memory
until ``load_data(extension)`` is called.  This matters for recipes that
conditionally use certain calibration inputs (e.g. a persistence map that may or
may not be present): you can check ``self.inputset.persistence.item is not None``
before attempting to load data.

Validation
----------

The ``InputSet`` validates all inputs during ``RecipeImpl.__init__``:

* Required inputs without a matching frame raise an exception before ``process()``
  is ever called.
* Tag matching is strict: frame tags must exactly match ``Item.name()``.

The validation happens before the tag-based promotion step, so it runs against
the generic (partially-specialized) class names.

``PipelineInputSet``
--------------------

:class:`~pymetis.engine.inputs.PipelineInputSet` is the container that holds
the ``PipelineInput`` members.  It discovers its members introspectively
(using ``inspect.getmembers``), so no explicit registration is required.

Key methods:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Method
     - Behaviour
   * - ``validate()``
     - Checks that all required inputs are present; raises on failure
   * - ``specialize(**tags)``
     - Propagates tag specialization to all inputs and their ``Item`` classes
   * - ``promote(**tags)``
     - Promotes ``Item`` classes to their fully-specialized subclasses
       (called after the frameset is loaded)
   * - ``list_classes()``
     - Returns ``(name, cls)`` pairs for all ``PipelineInput`` members
       (used for man-page generation and DRLD introspection)
   * - ``tag_matches``
     - Dict of tag parameters inferred from the actual loaded frames
       (used to drive the ``RecipeImpl.promote()`` call)
