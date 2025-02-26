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

import re

import cpl
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.inputs import RawInput
from pymetis.inputs.common import GainMapInput, PersistenceMapInput, BadpixMapInput, PinholeTableInput
from pymetis.products.product import PipelineProduct
from pymetis.inputs.mixins import LinearityInputSetMixin
from pymetis.prefab.rawimage import RawImageProcessor


class MetisCalChophomeImpl(RawImageProcessor):  # TODO replace parent class?
    """Implementation class for metis_cal_chophome"""
    class InputSet(LinearityInputSetMixin, RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_CHOPHOME_RAW")
            _description = "Raw exposure of the LM image mode."

        class BackgroundInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")
            _description = "Raw data for dark subtraction in other recipes."

        GainMapInput = GainMapInput
        PersistenceMapInput = PersistenceMapInput
        BadpixMapInput = BadpixMapInput
        PinholeTableInput = PinholeTableInput

    class ProductCombined(PipelineProduct):
        """
        Final product: combined, background-subtracted images of the WCU source
        """
        _tag: str = "LM_CHOPHOME_COMBINED"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Combined, background-subtracted images of the WCU source."
        _oca_keywords: {str} = {'PRO.CATG'}

    class ProductBackground(PipelineProduct):
        """
        Intermediate product: the instrumental background (WCU OFF)
        """
        _tag: str = "LM_CHOPHOME_BACKGROUND"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Stacked background-subtracted images of pinhole mask. The chopper offset is in the header."
        _oca_keywords: {str} = {'PRO.CATG'}


    def process_images(self) -> [PipelineProduct]:
        """do something"""

        background_hdr = cpl.core.PropertyList()
        bg_images = self.load_images(self.inputset.background.frameset)
        background_img = self.combine_images(bg_images, "median")
        # TODO: define usedframes

        combined_hdr = cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)
        raw_images.subtract_image(background_img)
        combined_img = self.combine_images(raw_images, "median")

        return [
            self.ProductCombined(self, combined_hdr, combined_img),
            self.ProductBackground(self, background_hdr, background_img),
        ]

    def load_images(self, frameset: cpl.ui.FrameSet) -> cpl.core.ImageList:
        """Load an imagelist from a FrameSet

        This is a temporary implementation that should be generalized to the
        entire pipeline package. It uses cpl functions - these should be
        replaced with hdrl functions once they become available, in order
        to use uncertainties and masks.
        """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(frameset):
            Msg.info(self.__class__.__qualname__,
                     f"Processing input frame #{idx}: {frame.file!r}...")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output


class MetisCalChophome(MetisRecipe):
    """Determine chopper home position
    """
    # Recipe information
    _name: str = "metis_cal_chophome"
    _version: str = "0.1"
    _author: str = "Oliver Czoske, A*"
    _email: str = "oliver.czoske@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Determine the chopper home position from LM-imaging of the WCU pinhole mask."
    _description: str = """\
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """The position of the pinhole image on the detector is measured from the stacked
    background-subtracted images. The measured position is compared to the WFS
    metrology to give the chopper home position.

    Remove detector signature
    Remove median background
    Apply flatfield
    Detect reference source from WCU via centroid peak detection
    Calculate mirror offset"""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])     # no parameters defined in DRLD

    implementation_class = MetisCalChophomeImpl
