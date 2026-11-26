"""
Checks that every registered recipe and every declared product must pass.

They are driven by the recipe registry, so a recipe is covered the moment it is
imported in `instruments/metis/recipes/__init__.py` -- no per-recipe test module is
needed for the structural and metadata conventions. The data-dependent checks replay
the jobs of an EDPS run over the simulated data (see `tests/data.py`): every job EDPS
ran for a recipe is a test case, so a recipe is exercised once for each band, target
and setup the workflow feeds it. A module under `tests/recipes/` is needed only for
recipe-specific tests.
"""
import ast
import inspect
import os
import pprint
import re
import subprocess
import tempfile
import textwrap
import types

import cpl
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the recipe registry)
from pymetis.engine.dataitems import DataItem, ImageDataItem, TableDataItem
from pymetis.engine.inputs import PipelineInput, PipelineInputSet
from pymetis.engine.recipes import Recipe
from tests.data import EDPS_DATA, EdpsJob, edps_jobs

RECIPES = sorted(Recipe._registry.values(), key=lambda recipe: recipe._name)
PRODUCTS = [(recipe, attr, product)
            for recipe in RECIPES
            for attr, product in recipe.Impl.ProductSet.list_classes()]



@pytest.fixture(params=RECIPES, ids=lambda recipe: recipe._name)
def recipe(request) -> type[Recipe]:
    return request.param


def edps_job_params() -> list:
    """
    One test case per EDPS job, named after its task (`metis_lm_img_dark`, numbered when
    the task ran several jobs). A registered recipe without any job gets one skipped case,
    so the report shows which recipes the EDPS run did not reach.
    """
    params = []
    for recipe in RECIPES:
        jobs = [job for job in edps_jobs() if job.recipe == recipe._name]
        if not jobs:
            params.append(pytest.param(
                None, id=recipe._name,
                marks=pytest.mark.skip(reason=f"no EDPS job ran {recipe._name} under {EDPS_DATA}")))
        tasks = [job.task for job in jobs]
        for job in jobs:
            number = [other for other in jobs if other.task == job.task].index(job) + 1
            params.append(pytest.param(job, id=job.task if tasks.count(job.task) == 1 else f"{job.task}#{number}"))
    return params


def load_frameset(job: EdpsJob) -> cpl.ui.FrameSet:
    frameset = cpl.ui.FrameSet()
    for file, tag in job.frames():
        frameset.append(cpl.ui.Frame(file, tag=tag))
    return frameset


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
        'metis_lm_img_distortion','metis_lm_img_flat_twilight','metis_lm_img_flat_lamp',
        'metis_lm_img_sci_postprocess', 'metis_lm_img_std_process',
        'metis_lm_lss_rsrf', 'metis_lm_lss_sci', 'metis_lm_lss_std', 'metis_lm_lss_trace', 'metis_lm_lss_wave',
        'metis_n_img_chopnod', 'metis_n_img_distortion', 'metis_n_img_flat_lamp', 'metis_n_img_flat_twilight', 'metis_n_img_std_process',
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


@pytest.fixture(scope='module', params=edps_job_params())
def job(request) -> EdpsJob:
    job = request.param
    if missing := job.missing():
        pytest.skip(f"{len(missing)} frame(s) of {job.sof} no longer exist, e.g. {missing[0]}; "
                    f"run EDPS over the simulated data again")
    return job

@pytest.fixture(scope='module')
def run(job):
    """ The recipe of the job, run in-process on its frames; products go to a scratch directory. """
    recipe = Recipe._registry[job.recipe]()
    home = os.getcwd()
    with tempfile.TemporaryDirectory(prefix=f"{job.task}-") as directory:
        os.chdir(directory)
        try:
            products = recipe.run(load_frameset(job), {})
        finally:
            os.chdir(home)
        yield types.SimpleNamespace(recipe=recipe, impl=recipe.implementation, products=products)


@pytest.mark.recipe
@pytest.mark.external
class TestRecipeOnEdpsJobs:
    """
    Integration checks on the frames EDPS associated for each of its jobs. The recipe is
    run once per job (the simulated frames are full size) and the tests inspect that run.
    """

    def test_recipe_can_be_run_directly(self, run):
        assert isinstance(run.products, cpl.ui.FrameSet), \
            f"{run.recipe.name} did not return a FrameSet"

    @pytest.mark.metadata
    def test_recipe_has_a_valid_as_dict(self, run):
        assert isinstance(pprint.pformat(run.impl.as_dict(), width=200), str)

    #: EDPS tasks that hand their recipe frames it has no input for, with the tags left over
    #: in the run over the simulated data of 2026-11-23. Mostly a task associated with all
    #: products of its predecessor rather than the ones the recipe takes; for `metis_det_dark`
    #: it is the recipe that lacks the linearity input of its DRLD card. Marked
    #: `xfail`, and not strict: this is a warning list that depends on the data set EDPS
    #: ran on, so a task that has come to agree with its recipe shows as XPASS without
    #: failing the run, and should then be removed from the list.
    TASKS_WITH_UNUSED_FRAMES = {
        'metis_ifu_dark': {'LINEARITY_IFU'},
        'metis_lm_img_dark': {'LINEARITY_2RG'},
        'metis_lm_lss_dark': {'LINEARITY_2RG'},
        'metis_n_img_dark': {'LINEARITY_GEO'},
        'metis_n_lss_dark': {'LINEARITY_GEO'},
        'metis_lm_img_distortion': {'MASTER_IMG_FLAT_LAMP_LM'},
        'metis_n_img_distortion': {'MASTER_IMG_FLAT_LAMP_N'},
        'metis_lm_img_standard_flux': {'LM_STD_BKG', 'LM_STD_OBJECT_CAT'},
        'metis_lm_img_calib': {'LM_DISTORTION_MAP', 'LM_DIST_REDUCED', 'LM_SCI_BKG', 'LM_SCI_BKG_SUBTRACTED',
                               'LM_SCI_OBJECT_CAT', 'LM_STD_COMBINED'},
        'metis_n_img_calib': {'N_DISTORTION_MAP', 'N_DIST_REDUCED', 'N_STD_COMBINED'},
        'metis_lm_lss_trace': {'MEAN_LM_LSS_RSRF', 'MEDIAN_LM_LSS_RSRF'},
        'metis_lm_lss_wave': {'MEAN_LM_LSS_RSRF', 'MEDIAN_LM_LSS_RSRF'},
        'metis_lm_lss_std': {'LM_LSS_CURVE', 'LM_LSS_DIST_SOL', 'LM_LSS_TRACE', 'LM_LSS_WAVE_GUESS',
                             'MEAN_LM_LSS_RSRF', 'MEDIAN_LM_LSS_RSRF'},
        'metis_lm_lss_sci': {'ATM_LINE_CAT', 'LM_LSS_CURVE', 'LM_LSS_DIST_SOL', 'LM_LSS_STD_1D',
                             'LM_LSS_STD_OBJ_MAP', 'LM_LSS_STD_SKY_MAP', 'LM_LSS_TRACE', 'LM_LSS_WAVE_GUESS',
                             'MEAN_LM_LSS_RSRF', 'MEDIAN_LM_LSS_RSRF'},
        'metis_lm_lss_mf_model': {'LM_LSS_SCI_2D', 'LM_LSS_SCI_FLUX_2D', 'LM_LSS_SCI_OBJ_MAP', 'LM_LSS_SCI_SKY_MAP'},
        'metis_lm_lss_mf_correct': {'LM_LSS_SCI_1D', 'LM_LSS_SCI_2D', 'LM_LSS_SCI_FLUX_2D',
                                    'LM_LSS_SCI_OBJ_MAP', 'LM_LSS_SCI_SKY_MAP'},
        'metis_n_lss_trace': {'MEAN_N_LSS_RSRF', 'MEDIAN_N_LSS_RSRF'},
        'metis_n_lss_std': {'MEAN_N_LSS_RSRF', 'MEDIAN_N_LSS_RSRF'},
        'metis_n_lss_sci': {'ATM_LINE_CAT', 'MEAN_N_LSS_RSRF', 'MEDIAN_N_LSS_RSRF', 'N_LSS_STD_1D',
                            'N_LSS_STD_OBJ_MAP', 'N_LSS_STD_SKY_MAP', 'N_LSS_TRACE'},
        'metis_n_lss_mf_model': {'MASTER_N_RESPONSE', 'N_LSS_SCI_2D', 'N_LSS_SCI_FLUX_2D', 'N_LSS_SCI_OBJ_MAP',
                                 'N_LSS_SCI_SKY_MAP', 'N_LSS_STD_1D', 'N_LSS_STD_OBJ_MAP', 'N_LSS_STD_SKY_MAP',
                                 'STD_TRANSMISSION'},
        'metis_n_lss_mf_correct': {'N_LSS_SCI_1D', 'N_LSS_SCI_2D', 'N_LSS_SCI_FLUX_2D',
                                   'N_LSS_SCI_OBJ_MAP', 'N_LSS_SCI_SKY_MAP'},
    }

    def test_uses_all_input_frames(self, job, run, request):
        """ Every frame EDPS associates for the task is one the recipe has an input for. """
        # FixMe this currently does not actually track usage, just loading
        if job.task in self.TASKS_WITH_UNUSED_FRAMES:
            request.applymarker(pytest.mark.xfail(
                strict=False, reason="the EDPS task passes frames the recipe has no input for"))
        loaded = {frame.file for frame in run.impl.inputset.valid_frames}
        unused = sorted({tag for file, tag in job.frames() if file not in loaded})
        assert not unused, \
            f"{job.task} passes {job.recipe} frames it has no input for: {unused}"

    def test_all_inputs_carry_complete_items(self, run):
        for inp in run.impl.inputset.inputs:
            item = inp.Item
            assert item is not None, f"Input {inp} has no associated data item"
            assert isinstance(item.name(), str)
            assert isinstance(item.title(), str)
            assert isinstance(item.description(), str)
            assert isinstance(item.oca_keywords(), frozenset)
            assert all(isinstance(kw, str) for kw in item.oca_keywords())

    @pytest.mark.inputset
    def test_input_set_loads_and_validates(self, job):
        instance = Recipe._registry[job.recipe].Impl.InputSet(load_frameset(job))
        assert isinstance(instance.inputs, tuple)
        for name, attr in instance.__dict__.items():
            if isinstance(attr, PipelineInput):
                assert attr in instance.inputs, f"Input {name} is not registered in inputs"
        assert all(inp.Item is not None for inp in instance.inputs)
        assert instance.validate() is None

    @pytest.mark.pyesorex
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, job):
        """ A second full run of the recipe, through the pyesorex executable this time. """
        with tempfile.TemporaryDirectory(prefix=f"{job.task}-") as directory:
            output = subprocess.run(['pyesorex', job.recipe, job.sof, '--log-level', 'DEBUG'],
                                    capture_output=True, cwd=directory,
                                    env=os.environ | {'PYESOREX_OUTPUT_DIR': directory})
        assert output.returncode == 0, f"Pyesorex exited with {output.returncode}"
        assert output.stderr == b"", f"Pyesorex exited with non-empty stderr: {output.stderr}"
