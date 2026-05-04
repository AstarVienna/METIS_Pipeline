Testing Recipes
================

``pymetis`` uses ``pytest`` for all tests.  Recipe tests live under::

    src/pymetis/instruments/metis/tests/recipes/

Framework tests (testing the engine layer itself) are under::

    src/pymetis/tests/

Running the tests
-----------------

From the repository root::

    pytest metisp/pymetis/src/

To run only the METIS recipe tests::

    pytest metisp/pymetis/src/pymetis/instruments/metis/tests/

To run with verbose output::

    pytest -v metisp/pymetis/src/pymetis/instruments/metis/tests/recipes/

Writing a recipe unit test
----------------------------

Recipe tests should test the ``RecipeImpl`` directly (bypassing ``pyesorex``),
using synthetic FITS data created in memory.

The typical pattern:

.. code-block:: python

    import pytest
    import cpl
    from pymetis.instruments.metis.recipes.metis_det_dark import (
        MetisDetDark, MetisDetDarkImpl,
    )
    from pymetis.instruments.metis.dataitems.masterdark import MasterDark2rg


    def make_dark_frameset(n_frames: int = 3) -> cpl.ui.FrameSet:
        """Build a minimal synthetic FrameSet of raw dark frames."""
        frames = cpl.ui.FrameSet()
        for i in range(n_frames):
            # Create an in-memory FITS frame (no file I/O)
            frame = cpl.ui.Frame(
                file=f"/dev/null",   # pyesorex stub; real tests use tmp files
                tag="DARK_2RG_RAW",
                group=cpl.ui.Frame.FrameGroup.RAW,
                level=cpl.ui.Frame.FrameLevel.RAW,
                frameType=cpl.ui.Frame.FrameType.IMAGE,
            )
            frames.append(frame)
        return frames


    class TestMetisDetDark:

        def test_product_type(self, tmp_path):
            """The recipe must produce a MasterDark2rg product."""
            recipe = MetisDetDark()
            frameset = make_dark_frameset()
            result_frameset = recipe.run(frameset, {})

            assert len(list(result_frameset)) == 1
            product_tag = list(result_frameset)[0].tag
            assert product_tag == MasterDark2rg.name()

        def test_stacking_method_parameter(self, tmp_path):
            """The stacking.method parameter must be respected."""
            recipe = MetisDetDark()
            frameset = make_dark_frameset()
            recipe.run(frameset, {"metis_det_dark.stacking.method": "median"})
            assert recipe.implementation.stacking_method == "median"

        def test_fails_without_raw_input(self):
            """An empty FrameSet must raise before process() is called."""
            recipe = MetisDetDark()
            with pytest.raises(Exception):
                recipe.run(cpl.ui.FrameSet(), {})

Testing DataItem schemas
-------------------------

To verify that a new ``DataItem`` schema is correctly enforced:

.. code-block:: python

    import pytest
    from pymetis.instruments.metis.dataitems.masterdark import MasterDark2rg
    from pymetis.engine.dataitems import Hdu
    import cpl
    from cpl.core import Image


    def test_schema_correct_types():
        """MasterDark2rg accepts the correct HDU types."""
        h = cpl.core.PropertyList()
        img = Image.new(64, 64)
        product = MasterDark2rg(
            h,
            Hdu(h, img, name='DET1.SCI'),
            Hdu(h, img, name='DET1.ERR'),
            Hdu(h, img, name='DET1.DQ'),
        )
        assert product['DET1.SCI'].klass == Image


    def test_schema_rejects_unknown_extension():
        """Adding an extension not in the schema raises ValueError."""
        h = cpl.core.PropertyList()
        img = Image.new(64, 64)
        with pytest.raises(ValueError, match="Unknown HDU"):
            MasterDark2rg(h, Hdu(h, img, name='UNKNOWN_EXT'))

Test fixtures and conftest
---------------------------

The file ``src/pymetis/conftest.py`` contains shared pytest fixtures.
Add reusable fixtures there (e.g. synthetic raw frames for each detector type).

Continuous integration
-----------------------

All tests are run by the CI pipeline on every pull request.  Make sure your
tests pass locally before opening a PR::

    pytest metisp/pymetis/src/ -x   # stop on first failure
