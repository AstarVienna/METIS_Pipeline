"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Data-free tests of `metis_img_adi_cgrph`: one recipe for both bands and the RAVC / CVC
coronagraphs, whose products and QC parameters are resolved per run from the tags of the
input frames, never from their headers.
"""
import cpl
import pytest

from pymetis.instruments.metis.recipes.hci.metis_img_adi_cgrph import MetisImgAdiCgrphImpl as Impl

# The DRLD card (Recipes_ADI.tex, metis_img_adi_cgrph): `{band}_{cgrph}_...`
DRLD_PRODUCTS = ('SCI_CALIBRATED', 'SCI_CENTRED', 'CENTROID_TAB', 'SCI_SPECKLE', 'SCI_HIFILT',
                 'SCI_DEROTATED_PSFSUB', 'SCI_DEROTATED', 'SCI_CONTRAST_RADPROF', 'SCI_CONTRAST_ADI',
                 'SCI_THROUGHPUT', 'SCI_COVERAGE', 'SCI_SNR', 'PSF_MEDIAN')
DRLD_QC = ('SCI NEXP', 'SCI FWHM {nn}', 'SCI SNR MEAN', 'SCI SNR PEAK', 'SCI CONTRAST RAW LAMD', 'SCI CONTRAST ADI LAMD')


def frameset(directory, *tags: str) -> cpl.ui.FrameSet:
    """ One (empty) FITS file per tag; classification and validation never open them. """
    frames = cpl.ui.FrameSet()
    for i, tag in enumerate(tags):
        filename = str(directory / f"{tag.lower()}_{i}.fits")
        cpl.core.PropertyList().save(filename, cpl.core.io.CREATE)
        frames.append(cpl.ui.Frame(filename, tag=tag))
    return frames


class TestTagsFromTheFrames:
    @pytest.mark.parametrize("band, cgrph", [('LM', 'RAVC'), ('LM', 'CVC'), ('N', 'RAVC'), ('N', 'CVC')])
    def test_band_and_coronagraph_come_from_the_science_and_throughput_frames(self, tmp_path, band, cgrph):
        frames = frameset(tmp_path, f'{band}_SCI_CALIBRATED', f'{band}_SCI_CALIBRATED', f'{band}_{cgrph}_SCI_THROUGHPUT')
        inputset = Impl.InputSet(frames)
        inputset.validate()
        assert inputset.tag_matches == {'band': band, 'target': 'SCI', 'cgrph': cgrph}

    @pytest.mark.parametrize("band, cgrph", [('LM', 'RAVC'), ('N', 'CVC')])
    def test_products_resolve_to_the_catalogue_leaves_of_the_run(self, band, cgrph):
        products = Impl.ProductSet.promoted(band=band, cgrph=cgrph, target='SCI')
        names = {klass.name() for _, klass in products.list_classes()}
        assert names == {f'{band}_{cgrph}_{suffix}' for suffix in DRLD_PRODUCTS}
        assert all(not klass._abstract and '{' not in klass.name() for _, klass in products.list_classes())

    @pytest.mark.parametrize("band, cgrph", [('LM', 'RAVC'), ('N', 'CVC')])
    def test_qc_parameters_resolve_to_the_drld_names(self, band, cgrph):
        qc = Impl.Qc.promoted(band=band, cgrph=cgrph, target='SCI')
        names = {klass.name() for _, klass in qc.list_classes()}
        assert names == {f'QC {band} {cgrph} {suffix}' for suffix in DRLD_QC}

    def test_without_a_coronagraph_the_qc_parameters_cannot_be_promoted(self):
        """ The coronagraph is never guessed (from a header, say): the run fails loudly. """
        with pytest.raises(TypeError, match="cgrph"):
            Impl.Qc.promoted(band='LM', target='SCI')

    def test_a_set_without_the_throughput_curve_is_refused(self, tmp_path):
        """ The throughput curve is the only input whose tag names the coronagraph. """
        inputset = Impl.InputSet(frameset(tmp_path, 'LM_SCI_CALIBRATED'))
        with pytest.raises(cpl.core.DataNotFoundError, match="SCI_THROUGHPUT"):
            inputset.validate()

    def test_a_throughput_curve_of_the_other_band_is_refused(self, tmp_path):
        inputset = Impl.InputSet(frameset(tmp_path, 'N_SCI_CALIBRATED', 'LM_RAVC_SCI_THROUGHPUT'))
        with pytest.raises(cpl.core.IllegalInputError, match="band"):
            inputset.validate()
