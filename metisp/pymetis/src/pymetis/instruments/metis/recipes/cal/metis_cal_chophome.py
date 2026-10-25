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
"""

import cpl

from pymetis.drl.combine import combine_images
from pymetis.drl.pinhole import locate_pinhole
from pymetis.engine.core.parameter import ParameterList, ParameterEnum, ParameterRange
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.functions.dummy import create_dummy_header

from pymetis.instruments.metis.mixins import BandLmMixin, Detector2rgMixin
from pymetis.instruments.metis.inputs import (RawInput, GainMapInput, PersistenceMapInput, BadPixMapInput,
                                              PinholeTableInput, LinearityInput, OptionalInputMixin)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor
from pymetis.instruments.metis import qc
from pymetis.instruments.metis import dataitems


class MetisCalChophomeImpl(BandLmMixin, Detector2rgMixin, RawImageProcessor, MetisRecipeImpl):  # TODO replace parent class?
    """Implementation class for metis_cal_chophome"""

    class InputSet(RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            Item = dataitems.LmChophomeRaw

        class WcuOffInput(RawInput):
            Item = dataitems.LmWcuOffRaw

        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            pass

        class PinholeTableInput(OptionalInputMixin, PinholeTableInput):
            pass

        raw: RawInput
        wcu_off: WcuOffInput
        gain_map: GainMapInput
        linearity: LinearityInput
        persistence_map: PersistenceMapInput
        bad_pix_map: BadPixMapInput
        pinhole_table: PinholeTableInput

    class ProductSet(PipelineProductSet):
        Combined = dataitems.LmChophomeCombined
        Background = dataitems.LmChophomeBackground

    class Qc(QcParameterSet):
        Xcen = qc.chophome.CalChophomeXcen
        XcenStdev = qc.chophome.CalChophomeXcenStdev
        Ycen = qc.chophome.CalChophomeYcen
        YcenStdev = qc.chophome.CalChophomeYcenStdev
        Fwhm = qc.chophome.CalChophomeFwhm
        Snr = qc.chophome.CalChophomeSnr
        OffX = qc.chophome.CalChophomeOffx
        OffY = qc.chophome.CalChophomeOffy
    def process(self) -> set[DataItem]:
        """This function processes the input images

        - stack the wcu_off images into background_img
        - subtract background_img from raw_images and stack
        - locate pinhole on the combined image
        """

        stackmethod = self.parameters[f"{self.name}.stacking.method"].value
        hwidth = self.parameters[f"{self.name}.halfwindow"].value

        background_hdr, background_img = self.compute_background(method=stackmethod)

        combined_hdr = cpl.core.PropertyList()
        raw_images = self.inputset.raw.load_data(extension='DET1.DATA')
        self.inputset.raw.use()

        # I think there's a bit of confusion about different sources of persistence maps that's
        # causing the workflow to crash. For now, I'll comment out the two lines, to be
        # uncommented after the workflow demonstration in Koln
        # persistence_map = self.inputset.persistence_map.load_data(extension='DET1.DATA')
        # self.inputset.persistence_map.use()
        raw_images.subtract_image(background_img)

        combined_img = combine_images(raw_images, stackmethod)

        # Locate the pinhole image
        pinhole_loc = locate_pinhole(combined_img, hwidth)

        combined_hdr.append(
            self.collect_qc_parameters(
                self.Qc.Xcen(pinhole_loc['xcen']),
                self.Qc.Ycen(pinhole_loc['ycen']),
                self.Qc.XcenStdev(pinhole_loc['xcenstd']),
                self.Qc.YcenStdev(pinhole_loc['ycenstd']),
                self.Qc.Fwhm(pinhole_loc['fwhm_x']),
                # FixMe: FWHM should be 2D?
                self.Qc.Snr(pinhole_loc['snr']),
                # FixMe: compute the chopper offsets; None marks a parameter that is not available yet
                self.Qc.OffX(None),
                self.Qc.OffY(None),
            )
        )

        return {
            self.ProductSet.Combined(combined_hdr,
                                 Hdu(create_dummy_header(), combined_img, name='IMAGE')),
            self.ProductSet.Background(background_hdr,
                                   Hdu(create_dummy_header(), background_img, name='IMAGE')),
        }

    def compute_background(self, *, method):
        background_hdr = cpl.core.PropertyList()

        bg_images = self.inputset.wcu_off.load_data(extension='DET1.DATA')
        background_img = combine_images(bg_images, method)
        # TODO: define usedframes
        return background_hdr, background_img


class MetisCalChophome(Recipe):
    """Determine chopper home position
    """
    # Recipe information
    _name: str = "metis_cal_chophome"
    _version: str = "0.1"
    _author: str = "Oliver Czoske, A*"
    _email: str = "oliver.czoske@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Determine the chopper home position from LM-imaging of the WCU pinhole mask."
    _description: str = """
        (nothing yet)
    """

    _matched_keywords: frozenset[str] = frozenset({'DET.DIT', 'DET.NDIT'})
    _algorithm = """
    The position of the pinhole image on the detector is measured from the
    stacked background-subtracted images. The measured position is compared
    to the WFS metrology to give the chopper home position.
    """

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            cli_alias="stacking.method",
            default="average",
            alternatives=("add", "average", "median"),
        ),
        ParameterRange(
            name=f"{_name}.halfwindow",
            context=_name,
            description="Half size of window for centroid determination [pix]",
            cli_alias="halfwindow",
            default=15,
            min=1,
            max=1024
        ),
    ])

    Impl = MetisCalChophomeImpl
