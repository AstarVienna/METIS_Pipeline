The Recipe / RecipeImpl Pattern
================================

Every pipeline recipe in ``pymetis`` is split into two classes:

* **Recipe** — a thin shell that satisfies the ``pyesorex`` plugin interface.
  Contains only metadata and a reference to the implementation.
* **RecipeImpl** — contains all the reduction logic, nested input/output
  declarations, and the abstract ``process()`` method.

Why two classes?
-----------------

``pyesorex`` discovers recipes by scanning Python modules for subclasses of
``cpl.ui.PyRecipe``.  It instantiates every class it finds, including abstract
base classes.  This makes it impossible to use ``ABC`` / ``abstractmethod`` on
the ``Recipe`` class itself.

The ``RecipeImpl`` side has no such constraint.  It derives from ``ABC``, so
``process()`` is a proper abstract method that Python enforces.

An additional benefit: ``RecipeImpl`` can be instantiated and tested independently
of ``pyesorex``, which makes unit testing significantly easier.

The Recipe class
-----------------

.. code-block:: python

    class MetisDetDark(Recipe):
        _name        = "metis_det_dark"
        _version     = "0.1"
        _author      = "Hugo Buddelmeijer, A*"
        _email       = "hugo@buddelmeijer.nl"
        _synopsis    = "Create master dark"
        _description = "Prototype to create a METIS masterdark for {detector}"

        # DRLD traceability fields (used by the autorecipe Sphinx extension
        # and the DRLD JSON manifest generator):
        _matched_keywords: set[str] = set()
        _algorithm = """
            - Group files by detector and DIT
            - Call metis_determine_dark for each set of files
            - Call metis_update_dark_mask to flag deviant pixels
        """
        _requirements: set[str] = set()   # e.g. {"METIS-5997"}
        _templates: set[str] = set()       # e.g. {"METIS_cal_DetDark"}

        parameters = ParameterList([
            ParameterEnum(
                name="metis_det_dark.stacking.method",
                context="metis_det_dark",
                description="Stacking method for combining input images",
                default="average",
                alternatives=("add", "average", "median", "sigclip"),
            ),
        ])

        # The only connection between Recipe and RecipeImpl:
        Impl = MetisDetDarkImpl

Notable mandatory attributes (checked by ``pyesorex``):

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Attribute
     - Purpose
   * - ``_name``
     - Recipe identifier used in SOF files and EDPS workflow definitions
   * - ``_version``
     - Version string
   * - ``_author``, ``_email``
     - Contact information
   * - ``_copyright``
     - Licence string (defaults to ``GPL-3.0-or-later``)
   * - ``_synopsis``
     - One-line summary shown by ``pyesorex --recipes``
   * - ``_description``
     - Full description shown by ``pyesorex --man-page``

Additional pymetis-specific attributes:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Attribute
     - Purpose
   * - ``_matched_keywords``
     - FITS header keywords used to match/group input frames
   * - ``_algorithm``
     - Human-readable algorithm description (used in the man page and DRLD)
   * - ``_requirements``
     - Set of requirement IDs from the METIS requirements database
   * - ``_templates``
     - Set of observation template names that trigger this recipe

The RecipeImpl class
---------------------

.. code-block:: python

    class MetisDetDarkImpl(PersistenceCorrectionMixin, RawImageProcessor, MetisRecipeImpl):

        class InputSet(RawImageProcessor.InputSet):
            class RawInput(RawInput):
                Item = DarkRaw             # which DataItem class this input wraps

            class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
                pass                       # optional: present only if supplied

            class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
                pass

        class ProductSet(PipelineProductSet):
            MasterDark = MasterDark        # which DataItem class to produce

        class Qc(QcParameterSet):
            DarkMean   = DarkMean
            DarkMedian = DarkMedian
            DarkRms    = DarkRms
            # ... more QC params

        def process(self) -> set[DataItem]:
            # Load pixel data (lazy — only happens here, not in __init__)
            raw_images = self.inputset.raw.load_data('DET1.DATA')

            # ... reduction logic ...

            product = self.ProductSet.MasterDark(primary_header, *hdus)
            return {product}

The three nested classes
~~~~~~~~~~~~~~~~~~~~~~~~~

``InputSet``, ``ProductSet``, and ``Qc`` are discovered introspectively — you
declare them as nested class attributes and the framework picks them up
automatically.  No registration step is needed.

``InputSet``
    One ``PipelineInput`` attribute per frame type consumed by the recipe.
    See :doc:`inputs` for details.

``ProductSet``
    One ``DataItem`` attribute per output file produced by the recipe.
    The framework calls ``DataItem.save()`` on each after ``process()`` returns.

``Qc``
    One ``QcParameter`` attribute per QC value computed by the recipe.
    QC values are written to the product primary header.

The ``process()`` method
~~~~~~~~~~~~~~~~~~~~~~~~~

This is the only method you **must** implement.  Its contract is:

* At entry, ``self.inputset`` has been populated and validated.
* Pixel data must be loaded lazily inside ``process()`` via ``load_data(extension)``.
* Return a ``set[DataItem]`` — one instance per output file.
* The framework handles saving, provenance headers, and FrameSet construction.

MRO and prefab mixins
~~~~~~~~~~~~~~~~~~~~~

``MetisDetDarkImpl`` inherits from three classes:

* ``PersistenceCorrectionMixin`` — adds persistence correction logic
* ``RawImageProcessor`` — provides ``combine_images_with_error()``, a shared
  ``InputSet`` base with common calibration inputs, and other utilities
* ``MetisRecipeImpl`` — the METIS-specific base (sets ``instrument = "METIS/1"``)

Python's method resolution order (MRO) ensures that mixin methods are found
before base-class methods, so ``RawImageProcessor.InputSet`` is the correct
starting point for ``InputSet`` inheritance.

See :doc:`../how_to/add_algorithm` for a step-by-step guide to writing a new
``RecipeImpl``.
