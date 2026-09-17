"""
Recipe parameters are declared once on the Recipe class; every run must work on its own
copy of them, and settings that name no parameter must be refused rather than ignored.
"""
import cpl
import pytest
from cpl.core import PropertyList as CplPropertyList

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the registries)
from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDark, MetisDetDarkImpl

METHOD = "metis_det_dark.stacking.method"


def frameset(directory) -> cpl.ui.FrameSet:
    """ A set of empty FITS files that satisfies metis_det_dark's inputs; nothing opens them
    before `process()`, which these tests never reach. """
    frames = cpl.ui.FrameSet()
    for i, tag in enumerate(('DARK_2RG_RAW', 'DARK_2RG_RAW', 'GAIN_MAP_2RG', 'LINEARITY_2RG')):
        filename = str(directory / f"{tag.lower()}_{i}.fits")
        CplPropertyList().save(filename, cpl.core.io.CREATE)
        frames.append(cpl.ui.Frame(filename, tag=tag))
    return frames


class TestSettingsAreScopedToTheRun:
    def test_two_runs_do_not_see_each_others_settings(self, tmp_path):
        recipe = MetisDetDark()
        median = MetisDetDarkImpl(recipe, frameset(tmp_path), {METHOD: "median"})
        sigclip = MetisDetDarkImpl(recipe, frameset(tmp_path), {METHOD: "sigclip"})
        assert median.parameters[METHOD].value == "median"
        assert sigclip.parameters[METHOD].value == "sigclip"

    def test_the_declaration_on_the_class_keeps_its_default(self, tmp_path):
        MetisDetDarkImpl(MetisDetDark(), frameset(tmp_path), {METHOD: "median"})
        assert MetisDetDark.parameters[METHOD].value == MetisDetDark.parameters[METHOD].default == "average"

    def test_a_run_without_settings_reads_the_defaults(self, tmp_path):
        MetisDetDarkImpl(MetisDetDark(), frameset(tmp_path), {METHOD: "median"})
        fresh = MetisDetDarkImpl(MetisDetDark(), frameset(tmp_path), {})
        assert fresh.parameters[METHOD].value == "average"

    def test_the_copy_keeps_the_parameter_kinds(self, tmp_path):
        impl = MetisDetDarkImpl(MetisDetDark(), frameset(tmp_path), {})
        assert [type(p).__name__ for p in impl.parameters] == [type(p).__name__ for p in MetisDetDark.parameters]
        assert impl.parameters[METHOD].alternatives == MetisDetDark.parameters[METHOD].alternatives


class TestUnknownSettings:
    def test_a_misspelled_parameter_is_refused(self, tmp_path):
        with pytest.raises(cpl.core.IllegalInputError, match="no parameter named 'metis_det_dark.stacking.methd'.*stacking.method"):
            MetisDetDarkImpl(MetisDetDark(), frameset(tmp_path), {"metis_det_dark.stacking.methd": "median"})
