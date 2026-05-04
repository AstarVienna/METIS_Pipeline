How to Add a New Recipe Implementation (Algorithm)
====================================================

This guide walks through creating a new ``RecipeImpl`` subclass.
Use this when you need to implement a new data reduction algorithm inside an
existing or new recipe.

We will build a minimal but complete example: a recipe that measures the
background level of a set of LM-band science frames.

.. note::

   You need to add a ``DataItem`` for each new output product your algorithm
   produces.  See :doc:`add_data_item` if you need a new product type.
   You also need a ``Recipe`` shell class for each new recipe; see :doc:`add_recipe`.

Prerequisites
-------------

* You understand :doc:`../concepts/architecture` and
  :doc:`../concepts/recipe_impl_pattern`.
* You know which input DataItem types your algorithm consumes and which output
  DataItem types it produces.
* The corresponding ``DataItem`` classes already exist in
  ``pymetis/instruments/metis/dataitems/``.

Step 1: Create the file
------------------------

New recipes live under ``src/pymetis/instruments/metis/recipes/``.  Name the
file after the recipe::

    src/pymetis/instruments/metis/recipes/metis_lm_measure_background.py

For new imaging-band recipes, the conventional sub-directory structure is::

    src/pymetis/instruments/metis/recipes/lm_img/metis_lm_measure_background.py

Step 2: Import what you need
-----------------------------

.. code-block:: python

    from typing import Any, Dict

    import cpl
    from cpl.core import Msg

    from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
    from pymetis.engine.inputs import SinglePipelineInput, MultiplePipelineInput
    from pymetis.engine.inputs.inputset import PipelineInputSet
    from pymetis.engine.qc import QcParameterSet

    from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
    from pymetis.instruments.metis.dataitems.img.lm_sci import LmSciRaw
    from pymetis.instruments.metis.dataitems.masterdark import MasterDark
    from pymetis.instruments.metis.dataitems.img.background import BackgroundMap  # your new product
    from pymetis.instruments.metis.qc.background import BackgroundMean, BackgroundRms

Step 3: Declare the InputSet
-----------------------------

Create a nested ``InputSet`` class listing all the frames this algorithm needs:

.. code-block:: python

    class MetisLmMeasureBackgroundImpl(MetisRecipeImpl):

        class InputSet(PipelineInputSet):

            class RawInput(MultiplePipelineInput):
                """Raw LM-band science frames to measure background from."""
                Item = LmSciRaw              # tag: LM_SCI_RAW
                _required = True

            class DarkInput(SinglePipelineInput):
                """Master dark for subtraction."""
                Item = MasterDark            # tag: MASTER_DARK_{detector}
                _required = True

Key rules:

* Each attribute must be a ``PipelineInput`` subclass.
* Set ``_required = False`` (or mix in ``OptionalInputMixin``) for frames that
  may be absent.
* Set ``_multiplicity = 'N'`` (or use ``MultiplePipelineInput``) for inputs
  where multiple frames of the same type are expected.

Step 4: Declare the ProductSet
--------------------------------

.. code-block:: python

        class ProductSet(PipelineProductSet):
            Background = BackgroundMap       # tag: LM_BACKGROUND_MAP_2RG (etc.)

Each attribute must be a ``DataItem`` subclass.  The framework will call
``DataItem.save()`` on each returned product after ``process()`` completes.

Step 5: Declare QC parameters
-------------------------------

.. code-block:: python

        class Qc(QcParameterSet):
            BackgroundMean = BackgroundMean
            BackgroundRms  = BackgroundRms

Step 6: Implement ``process()``
---------------------------------

``process()`` is the only method you are required to implement.  At entry,
``self.inputset`` is already validated and populated with ``DataItem`` objects.

.. code-block:: python

        def process(self) -> set[DataItem]:
            Msg.info(self.__class__.__qualname__, "Loading raw science frames")

            # Load structure (headers only) first — cheap
            self.inputset.raw.load_structure()

            # Load pixel data lazily — only when needed
            raw_images = self.inputset.raw.load_data('DET1.DATA')

            # Load the dark and subtract it
            dark = self.inputset.dark.load_data('DET1.SCI')
            dark_subtracted = cpl.core.ImageList(
                [img - dark for img in raw_images]
            )

            # Compute background statistics
            combined = dark_subtracted.collapse_median()
            bg_mean = combined.get_mean()
            bg_rms  = combined.get_stdev()

            # Store QC values
            self.qc.BackgroundMean.value = float(bg_mean)
            self.qc.BackgroundRms.value  = float(bg_rms)

            # Build the output HDUs
            primary_header = cpl.core.PropertyList()
            sci_hdu = Hdu(cpl.core.PropertyList(), combined, name='DET1.SCI')
            err_hdu = Hdu(cpl.core.PropertyList(),
                          cpl.core.Image.wrap(bg_rms * cpl.core.Image.new(combined.get_size_x(),
                                                                           combined.get_size_y())),
                          name='DET1.ERR')

            # Instantiate the product DataItem and return it
            product = self.ProductSet.Background(primary_header, sci_hdu, err_hdu)
            return {product}

Important rules for ``process()``:

* **Load data lazily**: call ``load_data()`` only when you actually need pixels,
  not in ``__init__`` or before ``process()`` is entered.
* **Return a set**: always return ``set[DataItem]`` — one element per output file.
* **No pixel manipulation outside ``process()``**: all FITS I/O and image arithmetic
  must happen inside this method (or in private helper methods it calls).

Step 7: Add DRLD metadata
--------------------------

Add the ``_algorithm``, ``_requirements``, and ``_templates`` class attributes
to the accompanying ``Recipe`` class (in the same file; see :doc:`add_recipe`).
These are used by the ``autorecipe`` Sphinx extension to generate the recipe
reference page and the DRLD JSON manifest automatically.

Step 8: Register the recipe
-----------------------------

No explicit registration is needed.  ``pyesorex`` discovers all subclasses of
``Recipe`` (``cpl.ui.PyRecipe``) at startup.

If you want the new recipe to appear in the Sphinx docs ``reference/`` section,
add an ``.. autorecipe::`` directive to the appropriate RST file under
``docs/reference/``.

Step 9: Add tests
------------------

See :doc:`testing` for how to write a recipe unit test.  At minimum, create a
test that:

* Constructs a minimal synthetic ``FrameSet`` with the correct tags.
* Instantiates ``MetisLmMeasureBackgroundImpl`` directly (bypassing ``pyesorex``).
* Calls ``process()`` and asserts that the returned set contains the expected
  product type.

Summary checklist
------------------

.. list-table::
   :header-rows: 0

   * - ☐ New file in ``recipes/`` (or sub-directory)
   * - ☐ ``InputSet`` with at least one ``PipelineInput`` member per input type
   * - ☐ ``ProductSet`` with at least one ``DataItem`` member per output
   * - ☐ ``Qc`` class with QC parameter members (may be empty)
   * - ☐ ``process()`` returns ``set[DataItem]``
   * - ☐ ``Recipe`` shell class with ``Impl`` pointing to your ``RecipeImpl``
   * - ☐ ``_algorithm``, ``_requirements``, ``_templates`` filled in on ``Recipe``
   * - ☐ ``.. autorecipe::`` added to ``docs/reference/``
   * - ☐ Tests written and passing
