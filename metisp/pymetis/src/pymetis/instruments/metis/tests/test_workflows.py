"""
The EDPS workflows against the recipes they run.

Every workflow task is loaded offline (`pymetis.engine.workflows`) and bound to its recipe's
InputSet: the tags a task feeds must be accepted, the main input must land on a primary
input, optional / required and one / many must agree, every recipe input must be fed, and
what consumers ask for must be a declared product. The workflow package is hand-written and
disagrees with the recipes in many places today; each disagreement is a strict xfail below,
listed per test, so that fixing a workflow line (or a recipe) flips exactly that entry to an
unexpected pass and the list has to shrink.
"""
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the recipe registry)
from pymetis.engine.recipes import Recipe

pytest.importorskip('edps')
from pymetis.engine import workflows as wf  # noqa: E402

if wf.workflows_directory() is None:
    pytest.skip("the metis workflow package is not available (set $METIS_WORKFLOWS)", allow_module_level=True)

WORKFLOWS = ('metis.metis_wkf', 'metis.metis_lm_app_wkf', 'metis.metis_lm_ravc_wkf')


def _bindings() -> dict[str, wf.TaskBinding]:
    """ One binding per task name; a task defined in several modules is the same object everywhere. """
    out = {}
    for module in WORKFLOWS:
        workflow = wf.load_workflow(module)
        tasks = wf.tasks(workflow)
        for task in tasks:
            out.setdefault(task.name, wf.bind(task, tasks, module))
    return out


BINDINGS = _bindings()


@pytest.fixture(params=sorted(BINDINGS), ids=lambda name: name)
def binding(request) -> wf.TaskBinding:
    return BINDINGS[request.param]


def expect_failure(request, binding, names: frozenset[str], reason: str) -> None:
    if binding.task in names:
        request.applymarker(pytest.mark.xfail(strict=True, reason=reason))


# Snapshot of the disagreements on 2026-09-29; remove a task from a set once its workflow or recipe is fixed.

FEEDS_UNACCEPTED_TAGS = frozenset({          # a fed tag no input of the recipe covers
    'metis_ifu_dark', 'metis_lm_img_dark', 'metis_n_img_dark', 'metis_lm_lss_dark', 'metis_n_lss_dark',   # LINEARITY
    'metis_lm_img_distortion', 'metis_n_img_distortion',                    # MASTER_IMG_FLAT_LAMP_*
    'metis_lm_lss_lingain', 'metis_n_lss_lingain',                          # *_WCU_OFF_RAW
    'metis_lm_lss_sci', 'metis_n_lss_sci', 'metis_lm_lss_std',              # LSS_TRACE, ATM_LINE_CAT
    'metis_lm_lss_mf_model', 'metis_n_lss_mf_model',                        # *_LSS_STD_1D
})
OPTIONALITY_DISAGREES = frozenset({          # min_ret vs OptionalInputMixin (mostly PERSISTENCE_MAP, GAIN_MAP)
    'metis_chophome_imaging', 'metis_ifu_distortion', 'metis_ifu_wavecal',
    'metis_lm_img_basic_reduce_sci', 'metis_lm_img_basic_reduce_sky', 'metis_lm_img_basic_reduce_std',
    'metis_lm_img_dark', 'metis_lm_img_distortion', 'metis_lm_img_flat',
    'metis_lm_lss_adc_slitloss', 'metis_lm_lss_dark', 'metis_lm_lss_mf_model', 'metis_lm_lss_rsrf', 'metis_lm_lss_sci',
    'metis_lm_lss_trace', 'metis_lm_lss_wave',
    'metis_n_adc_slitloss', 'metis_n_img_chopnod_sci', 'metis_n_img_chopnod_std', 'metis_n_img_dark',
    'metis_n_img_distortion', 'metis_n_img_flat',
    'metis_n_lss_dark', 'metis_n_lss_mf_model', 'metis_n_lss_rsrf', 'metis_n_lss_sci', 'metis_n_lss_trace',
})
MULTIPLICITY_DISAGREES = frozenset({         # the recipe takes N frames, the workflow associates at most one
    'metis_chophome_imaging', 'metis_ifu_rsrf', 'metis_ifu_sci_reduce', 'metis_ifu_std_reduce',
    'metis_lm_img_distortion', 'metis_n_img_distortion',
    'metis_lm_lss_adc_slitloss', 'metis_lm_lss_rsrf', 'metis_lm_lss_trace', 'metis_lm_lss_wave',
    'metis_n_adc_slitloss', 'metis_n_lss_rsrf', 'metis_n_lss_trace',
})
REQUIRED_INPUT_NOT_FED = frozenset({
    'metis_ifu_postprocess',                 # IFU_SCI_CUBE_CALIBRATED: main input given without a class list
    'metis_lm_lss_mf_model', 'metis_n_lss_mf_model',     # *_LSS_SCI_1D
    'metis_lm_lss_wave',                     # BADPIX_MAP_2RG is required there
    'metis_lm_app_post', 'metis_lm_ravc_post',           # the throughput curve
})
OPTIONAL_INPUT_NOT_FED = frozenset({         # mostly the bad-pixel map, associated only in the IFU workflow
    'metis_chophome_imaging', 'metis_lm_app_post', 'metis_lm_ravc_post', 'metis_pupil_imaging',
    'metis_lm_img_basic_reduce_sci', 'metis_lm_img_basic_reduce_sky', 'metis_lm_img_basic_reduce_std',
    'metis_lm_img_dark', 'metis_lm_img_distortion', 'metis_lm_img_flat', 'metis_lm_img_lingain',
    'metis_lm_lss_adc_slitloss', 'metis_lm_lss_dark', 'metis_lm_lss_lingain', 'metis_lm_lss_rsrf', 'metis_lm_lss_sci',
    'metis_lm_lss_std', 'metis_lm_lss_trace',
    'metis_n_adc_slitloss', 'metis_n_img_chopnod_sci', 'metis_n_img_chopnod_std', 'metis_n_img_dark',
    'metis_n_img_distortion', 'metis_n_img_flat', 'metis_n_img_lingain',
    'metis_n_lss_dark', 'metis_n_lss_lingain', 'metis_n_lss_rsrf', 'metis_n_lss_sci', 'metis_n_lss_std', 'metis_n_lss_trace',
})
RECIPES_WITHOUT_A_TASK = frozenset({'metis_lm_adi_app'})


@pytest.mark.recipe
@pytest.mark.metadata
class TestTaskBinding:
    def test_the_recipe_is_registered(self, binding):
        assert binding.recipe in Recipe._registry, f"{binding.task} runs {binding.recipe}, which is not a registered recipe"
        assert not [u for u in binding.unknown if u.startswith('recipe ')]

    def test_every_fed_tag_is_a_catalogue_item(self, binding):
        assert not binding.unknown, f"{binding.task} feeds tags that are no data item: {binding.unknown}"

    def test_every_fed_tag_is_accepted_by_the_recipe(self, binding, request):
        expect_failure(request, binding, FEEDS_UNACCEPTED_TAGS, "workflow feeds a tag the recipe has no input for")
        assert not binding.unaccepted, f"{binding.task} feeds {binding.recipe} tags no input accepts: {binding.unaccepted}"

    def test_the_main_input_lands_on_a_primary_input(self, binding):
        assert not binding.main_not_primary, \
            f"{binding.task}: main input {binding.main_not_primary} is accepted only by calibration-role inputs of {binding.recipe}"

    def test_optional_and_required_agree(self, binding, request):
        expect_failure(request, binding, OPTIONALITY_DISAGREES, "min_ret and OptionalInputMixin disagree")
        assert not binding.optionality, f"{binding.task} / {binding.recipe}: {binding.optionality}"

    def test_one_and_many_agree(self, binding, request):
        expect_failure(request, binding, MULTIPLICITY_DISAGREES, "recipe takes N frames, workflow max_ret=1")
        assert not binding.multiplicity, f"{binding.task} / {binding.recipe}: {binding.multiplicity}"

    def test_every_required_input_is_fed(self, binding, request):
        expect_failure(request, binding, REQUIRED_INPUT_NOT_FED, "a required recipe input has no source in the workflow")
        assert not binding.missing_required, f"{binding.task} does not feed {binding.recipe}: {binding.missing_required}"

    def test_every_optional_input_is_fed(self, binding, request):
        expect_failure(request, binding, OPTIONAL_INPUT_NOT_FED, "an optional recipe input has no source in the workflow")
        assert not binding.missing_optional, f"{binding.task} does not feed {binding.recipe}: {binding.missing_optional}"

    def test_requested_products_are_declared(self, binding):
        assert not binding.products_not_declared, \
            f"{binding.task}: consumers request {binding.products_not_declared}, which {binding.recipe} does not produce"


@pytest.mark.recipe
@pytest.mark.metadata
@pytest.mark.parametrize("recipe_name", sorted(Recipe._registry))
def test_every_recipe_has_a_workflow_task(recipe_name, request):
    if recipe_name in RECIPES_WITHOUT_A_TASK:
        request.applymarker(pytest.mark.xfail(strict=True, reason="no workflow task runs this recipe yet"))
    assert recipe_name in {b.recipe for b in BINDINGS.values()}, f"no task of {WORKFLOWS} runs {recipe_name}"
