"""
Checks that every registered recipe and every declared product must pass.

They are driven by the recipe registry, so a recipe is covered the moment it is
imported in `instruments/metis/recipes/__init__.py` -- no per-recipe test module is
needed for the structural and metadata conventions. The data-dependent checks run
on `<recipe>.sof` from `$SOF_DIR` and skip for recipes whose SOFs are named
differently (per band or target); those recipes keep their own test module under
`tests/recipes/`, alongside any recipe-specific tests.
"""
import ast
import inspect
import os
import pprint
import re
import subprocess
import textwrap
from pathlib import Path

import cpl
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the recipe registry)
from pymetis.engine.dataitems import DataItem, ImageDataItem, TableDataItem
from pymetis.engine.inputs import PipelineInput, PipelineInputSet
from pymetis.engine.recipes import Recipe

RECIPES = sorted(Recipe._registry.values(), key=lambda recipe: recipe._name)
PRODUCTS = [(recipe, attr, product)
            for recipe in RECIPES
            for attr, product in recipe.Impl.ProductSet.list_classes()]

root = Path(os.path.expandvars("$SOF_DIR"))


@pytest.fixture(params=RECIPES, ids=lambda recipe: recipe._name)
def recipe(request) -> type[Recipe]:
    return request.param


@pytest.fixture
def sof(recipe) -> str:
    """ The default SOF of a recipe; recipes with per-band or per-target SOFs skip. """
    sof = f"{recipe._name}.sof"
    if not (root / sof).exists():
        pytest.skip(f"no default SOF {sof} -- covered by the recipe's own test module")
    return sof


@pytest.fixture
def frameset(recipe, sof, load_frameset) -> cpl.ui.FrameSet:
    return cpl.ui.FrameSet(load_frameset(sof))


@pytest.mark.recipe
@pytest.mark.metadata
class TestRecipeMetadata:
    def test_author_name_conforms_to_standard(self, recipe):
        """ TBD what the standard actually means. """
        assert re.match(r"^([\w\- ]+, )?A\*(, ASIAA)?$", recipe._author), \
            f"{recipe._name}: author name {recipe._author!r} is not in the standard format"

    def test_matched_keywords_are_defined(self, recipe):
        assert recipe._matched_keywords is not None, \
            f"{recipe._name} does not have matched keywords defined"

    def test_algorithm_is_described(self, recipe):
        assert recipe._algorithm is not None, \
            f"{recipe._name} does not have an algorithm description"

    def test_parameters_have_the_recipe_as_context(self, recipe):
        for param in recipe.parameters:
            assert param.context == recipe._name, \
                f"Parameter context of {param.name} differs from recipe name {recipe._name}"

    def test_parameter_names_start_with_the_recipe_name(self, recipe):
        for param in recipe.parameters:
            assert param.name.startswith(recipe._name), \
                f"Parameter name {param.name} does not start with {recipe._name}"

    def test_description_can_be_built(self, recipe):
        assert recipe._build_description() is not None

    def test_the_man_page_is_the_recipes_own_and_stable(self, recipe):
        """ 24 recipes used to show the base class's placeholder text, and building the
        description twice nested the man page into itself. """
        assert "If you see this in a recipe" not in recipe._description
        assert recipe._build_description() == recipe._build_description()

    def test_the_impl_names_its_instrument(self, recipe):
        # cpl.dfs.save_propertylist needs it; a prefab base that skips MetisRecipeImpl loses it (Review III, R2)
        assert isinstance(recipe.Impl.instrument, str) and recipe.Impl.instrument

    def test_recipe_can_be_instantiated(self, recipe):
        assert isinstance(recipe(), cpl.ui.PyRecipe)

    def test_has_a_product_set(self, recipe):
        assert recipe.Impl.ProductSet is not None, f"{recipe._name} has no ProductSet"
        assert isinstance(recipe._list_products(), list)

    def test_has_a_qc_parameter_set(self, recipe):
        assert recipe.Impl.Qc is not None, f"{recipe._name} has no QcParameterSet"
        assert isinstance(recipe._list_qc_parameters(), list)

    @pytest.mark.inputset
    def test_input_set_is_a_concrete_pipeline_input_set(self, recipe):
        assert issubclass(recipe.Impl.InputSet, PipelineInputSet), \
            f"{recipe._name}: InputSet is not derived from PipelineInputSet"
        assert not inspect.isabstract(recipe.Impl.InputSet), \
            f"{recipe._name}: InputSet is abstract"

    @pytest.mark.pyesorex
    def test_pyesorex_can_display_the_man_page(self, recipe):
        output = subprocess.run(['pyesorex', '--man-page', recipe._name, '--log-level', 'DEBUG'],
                                capture_output=True)
        assert output.returncode == 0, \
            f"`pyesorex --man-page {recipe._name}` exited with {output.returncode}: {output.stderr!r}"


@pytest.mark.product
@pytest.mark.metadata
class TestDeclaredProducts:
    @pytest.fixture(params=PRODUCTS, ids=lambda entry: f"{entry[0]._name}:{entry[1]}")
    def product(self, request) -> type[DataItem]:
        return request.param[2]

    def test_is_a_data_item_with_level_and_type(self, product):
        assert issubclass(product, DataItem)
        assert product.frame_level() is not None, f"{product.__qualname__} has no frame level"
        assert product.frame_type() is not None, f"{product.__qualname__} has no frame type"

    def test_a_product_is_never_raw_data(self, product):
        """ An item's frame group is its origin; the role its frames play in a consuming
        recipe (RAW or CALIB) belongs to that recipe's input declaration instead. """
        assert product.frame_group() != cpl.ui.Frame.FrameGroup.RAW, \
            f"{product.__qualname__} is produced by a recipe but declares itself RAW"

    def test_frame_type_matches_the_item_kind(self, product):
        if issubclass(product, ImageDataItem):
            assert product.frame_type() == cpl.ui.Frame.FrameType.IMAGE, \
                f"{product.__qualname__} is an ImageDataItem with frame type {product.frame_type()}"
        elif issubclass(product, TableDataItem):
            assert product.frame_type() == cpl.ui.Frame.FrameType.TABLE, \
                f"{product.__qualname__} is a TableDataItem with frame type {product.frame_type()}"


@pytest.mark.recipe
@pytest.mark.metadata
class TestDeclaredQcParametersAreWritten:
    """
    A QC parameter declared in a recipe's `Qc` set is advertised by the man page and by the
    DRLD generator, so the recipe should actually produce it. This test checks statically,
    by walking the AST, that every declared parameter is instantiated somewhere in the
    recipe implementation (`self.Qc.<Name>(...)` in the Impl or one of its prefab bases). It fails for every
    skeleton recipe until the values are computed; the QC audit of 2026-09-17 counted
    141 of 178 declarations never written. Those recipes are listed below and marked
    `xfail(strict=True)`: once a recipe writes all of its QC parameters the test flips to
    an unexpected pass and the recipe has to be removed from the list.
    """

    RECIPES_WITH_UNWRITTEN_QC = frozenset({
        'metis_cal_chophome', 'metis_det_lingain',
        'metis_ifu_calibrate', 'metis_ifu_distortion', 'metis_ifu_postprocess', 'metis_ifu_reduce', 'metis_ifu_telluric',
        'metis_img_adi_cgrph', 'metis_lm_adi_app',
        'metis_lm_img_background', 'metis_lm_img_distortion', 'metis_lm_img_flat',
        'metis_lm_img_sci_postprocess', 'metis_lm_img_std_process',
        'metis_lm_lss_rsrf', 'metis_lm_lss_sci', 'metis_lm_lss_std', 'metis_lm_lss_trace', 'metis_lm_lss_wave',
        'metis_n_img_chopnod', 'metis_n_img_distortion', 'metis_n_img_flat', 'metis_n_img_std_process',
        'metis_n_lss_rsrf', 'metis_n_lss_sci', 'metis_n_lss_std', 'metis_n_lss_trace',
    })

    @staticmethod
    def computed_qc_parameters(recipe: type[Recipe]) -> set[str]:
        """
        Names of the QC parameters the recipe implementation instantiates, found by walking
        the AST of every class in the Impl's MRO that lives in pymetis. A call
        ``<anything>.Qc.<Name>(...)`` counts, unless its first argument is the literal
        ``None``: that is the placeholder a skeleton recipe wires in ("not available",
        skipped at collection). Comments and strings are invisible to the AST, so a
        commented-out call does not count.
        """
        computed: set[str] = set()
        for base in recipe.Impl.__mro__:
            try:
                path = inspect.getsourcefile(base)
                source = inspect.getsource(base)
            except (TypeError, OSError):
                continue
            if not path or 'pymetis' not in path:
                continue
            for node in ast.walk(ast.parse(textwrap.dedent(source))):
                if not (isinstance(node, ast.Call)
                        and isinstance(node.func, ast.Attribute)
                        and isinstance(node.func.value, ast.Attribute)
                        and node.func.value.attr == 'Qc'):
                    continue
                first = node.args[0] if node.args else None
                if isinstance(first, ast.Constant) and first.value is None:
                    continue
                computed.add(node.func.attr)
        return computed

    def test_every_declared_qc_parameter_is_instantiated(self, recipe, request):
        if recipe._name in self.RECIPES_WITH_UNWRITTEN_QC:
            request.applymarker(pytest.mark.xfail(
                strict=True, reason="skeleton recipe: declared QC parameters are not computed yet"))
        computed = self.computed_qc_parameters(recipe)
        unwritten = [f"{attr} ({klass.name()})"
                     for attr, klass in recipe._list_qc_parameters()
                     if attr not in computed]
        assert not unwritten, \
            (f"{recipe._name} declares {len(unwritten)} QC parameter(s) it never writes: "
             + ', '.join(unwritten))


@pytest.mark.recipe
@pytest.mark.external
class TestRecipeOnDefaultData:
    """ Integration checks on `<recipe>.sof`; see the `sof` fixture for the skip rule. """

    def test_recipe_can_be_run_directly(self, recipe, frameset):
        assert isinstance(recipe().run(frameset, {}), cpl.ui.FrameSet), \
            f"{recipe._name} did not return a FrameSet"

    @pytest.mark.metadata
    def test_recipe_has_a_valid_as_dict(self, recipe, frameset):
        instance = recipe()
        instance.run(frameset, {})
        assert isinstance(pprint.pformat(instance.implementation.as_dict(), width=200), str)

    @pytest.mark.pyesorex
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, recipe, sof):
        output = subprocess.run(['pyesorex', recipe._name, root / sof, '--log-level', 'DEBUG'],
                                capture_output=True)
        assert output.returncode == 0, f"Pyesorex exited with {output.returncode}"
        assert output.stderr == b"", f"Pyesorex exited with non-empty stderr: {output.stderr}"

    def test_every_sof_frame_is_claimed_by_an_input(self, recipe, frameset):
        """ `valid_frames` is the union of what the inputs matched, so a frame missing from it is a
        surplus line in the SOF that no input knows what to do with. Usage is a different statement,
        see the next test. """
        instance = recipe()
        instance.run(frameset, {})
        all_frames = {frame.file for frame in instance.implementation.inputset.frameset}
        claimed = {frame.file for frame in instance.implementation.inputset.valid_frames}
        assert claimed == all_frames, \
            f"Frames are present in the SOF file but claimed by no input: {all_frames - claimed}"

    def test_used_frames_come_from_the_sof_and_lead_with_the_primary_input(self, recipe, frameset):
        """ The product header's provenance (PRO REC1 RAWn / CALn) is `used_frames` in order: every
        used frame was in the SOF, something was used, and the first used frame belongs to the
        input whose role is RAW, since CPL DFS inherits the primary header from it. """
        instance = recipe()
        instance.run(frameset, {})
        inputset = instance.implementation.inputset
        used = [frame.file for frame in inputset.used_frames]
        assert used, f"{recipe._name} used no frames"
        assert set(used) <= {frame.file for frame in inputset.frameset}
        primary = inputset.inputs_by_role()[0]
        assert used[0] in {frame.file for frame in primary.valid_frames()}, \
            f"the first used frame {used[0]} is not from the primary input {primary}"

    def test_all_inputs_carry_complete_items(self, recipe, frameset):
        instance = recipe()
        instance.run(frameset, {})
        for inp in instance.implementation.inputset.inputs:
            item = inp.Item
            assert item is not None, f"Input {inp} has no associated data item"
            assert isinstance(item.name(), str)
            assert isinstance(item.title(), str)
            assert isinstance(item.description(), str)
            assert isinstance(item.oca_keywords(), frozenset)
            assert all(isinstance(kw, str) for kw in item.oca_keywords())

    @pytest.mark.inputset
    def test_input_set_loads_and_validates(self, recipe, frameset):
        instance = recipe.Impl.InputSet(frameset)
        assert isinstance(instance.inputs, tuple)        # declaration order, see PipelineInputSet
        for name, attr in instance.__dict__.items():
            if isinstance(attr, PipelineInput):
                assert attr in instance.inputs, f"Input {name} is not registered in inputs"
        assert all(inp.Item is not None for inp in instance.inputs)
        assert instance.validate() is None


@pytest.mark.recipe
@pytest.mark.metadata
class TestFlowchartSteps:
    """
    The Impl's `_steps` is what the DRLD flowchart is generated from: every step names input and
    product classes, and each must resolve to exactly one attribute of the recipe's InputSet or
    ProductSet. Recipes without a hand-drawn flowchart to transcribe have no steps yet; they are
    a strict xfail list, to shrink as authors declare them.
    """
    RECIPES_WITHOUT_STEPS = frozenset({         # no hand-drawn DRLD flowchart existed to transcribe (2026-09-29)
        'metis_lm_adc_slitloss', 'metis_n_adc_slitloss', 'metis_pupil_imaging', 'metis_n_img_restore',
        'metis_lm_lss_rsrf', 'metis_lm_lss_trace', 'metis_lm_lss_wave', 'metis_lm_lss_std', 'metis_lm_lss_sci',
        'metis_lm_lss_mf_model', 'metis_lm_lss_mf_calctrans', 'metis_lm_lss_mf_correct',
        'metis_n_lss_rsrf', 'metis_n_lss_trace', 'metis_n_lss_std', 'metis_n_lss_sci',
        'metis_n_lss_mf_model', 'metis_n_lss_mf_calctrans', 'metis_n_lss_mf_correct',
    })

    def test_steps_are_declared(self, recipe, request):
        if recipe._name in self.RECIPES_WITHOUT_STEPS:
            request.applymarker(pytest.mark.xfail(strict=True, reason="no _steps declared yet"))
        assert recipe.Impl._steps, f"{recipe._name} declares no _steps"

    def test_steps_name_real_inputs_and_products(self, recipe):
        """ Every class a step names resolves to exactly one attribute (none or several raises). """
        for step in recipe.Impl._steps:
            assert step.label.strip(), f"{recipe._name}: a step has no label"
            for klass in step.inputs:
                assert isinstance(klass, type), f"{recipe._name}: step {step.label!r} input {klass!r} is not a class"
                recipe.Impl.input_name(klass)
            for klass in step.products:
                assert isinstance(klass, type), f"{recipe._name}: step {step.label!r} product {klass!r} is not a class"
                recipe.Impl.product_name(klass)

    def test_each_product_is_produced_by_at_most_one_step(self, recipe):
        seen: dict[str, str] = {}
        for step in recipe.Impl._steps:
            for klass in step.products:
                attr = recipe.Impl.product_name(klass)
                assert attr not in seen, f"{recipe._name}: {attr} produced by {seen[attr]!r} and {step.label!r}"
                seen[attr] = step.label

