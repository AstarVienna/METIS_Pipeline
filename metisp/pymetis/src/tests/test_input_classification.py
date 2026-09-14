"""
Tests for the classification of SOF frames into a `PipelineInput`: promotion to the
specialized data item found in the frames, and the refusal of frames of several
different data items in one input.
"""
import cpl
import pytest
from cpl.core import PropertyList as CplPropertyList

import pymetis.instruments.metis.dataitems  # noqa: F401  (fills the data item registry)
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg, MasterDarkGeo
from pymetis.instruments.metis.inputs.common import MasterDarkInput


def frameset(directory, *tags: str) -> cpl.ui.FrameSet:
    """ One (empty) FITS file per tag; `cpl.ui.Frame` insists that its file exists,
    but classification itself never opens it. """
    frames = cpl.ui.FrameSet()
    for i, tag in enumerate(tags):
        filename = str(directory / f"{tag.lower()}_{i}.fits")
        CplPropertyList().save(filename, cpl.core.io.CREATE)
        frames.append(cpl.ui.Frame(filename, tag=tag))
    return frames


class TestInputClassification:
    def test_a_single_tag_promotes_the_item(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG'))
        assert inp.Item is MasterDark2rg

    def test_unrelated_tags_are_ignored(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_GEO', 'PERSISTENCE_MAP'))
        assert inp.Item is MasterDarkGeo

    def test_frames_of_several_data_items_are_refused(self, tmp_path):
        """ Two detectors' master darks can never feed one input; the old code
        loaded both in turn and silently kept whichever came last. """
        with pytest.raises(cpl.core.IllegalInputError, match='MASTER_DARK_2RG.*MASTER_DARK_GEO'):
            MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG', 'MASTER_DARK_GEO'))
