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

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import RawInput
from pymetis.classes.inputs import GainMapInput, PersistenceMapInput, BadpixMapInput, PinholeTableInput
from pymetis.classes.inputs import LinearityInput
from pymetis.classes.products import PipelineProduct
from pymetis.classes.prefab import RawImageProcessor


class MetisCalChophomeImpl(RawImageProcessor):  # TODO replace parent class?
    """Implementation class for metis_cal_chophome"""
    class InputSet(RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_CHOPHOME_RAW")
            _description: str = "Raw exposure of the LM image mode."

        class BackgroundInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")
            _description: str = "Raw data for dark subtraction in other recipes."

        class GainMapInput(GainMapInput):
            _required = False     # CHECK Optional for functional development

        class LinearityInput(LinearityInput):
            _required = False    # CHECK Optional for functional development

        PersistenceMapInput = PersistenceMapInput

        class BadpixMapInput(BadpixMapInput):
            _required = False     # CHECK Optional for functional development

        class PinholeTableInput(PinholeTableInput):
            _required = False     # CHECK Is this needed for single pinhole?

    class ProductCombined(PipelineProduct):
        """
        Final product: combined, background-subtracted images of the WCU source
        """
        _tag: str = "LM_CHOPHOME_COMBINED"
        group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.PRODUCT
        level: cpl.ui.Frame.FrameLevel = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description: str = "Combined, background-subtracted images of the WCU source."
        _oca_keywords: {str} = {'PRO.CATG'}

    class ProductBackground(PipelineProduct):
        """
        Intermediate product: the instrumental background (WCU OFF)
        """
        _tag: str = "LM_CHOPHOME_BACKGROUND"
        group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.PRODUCT
        level: cpl.ui.Frame.FrameLevel = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description: str = "Stacked background-subtracted images of pinhole mask. The chopper offset is in the header."
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

        # TODO: locate the pinhole image
        pinhole_loc = locate_pinhole(combined_img)

        # Extract QC parameters   # FIXME these are not written to the product header
        combined_hdr.append(cpl.core.Property("QC XCEN", cpl.core.Type.DOUBLE,
                                              pinhole_loc["xcen"], "[pix] x position of pinhole"))
        combined_hdr.append(cpl.core.Property("QC YCEN", cpl.core.Type.DOUBLE,
                                              pinhole_loc["ycen"], "[pix] y position of pinhole"))
        combined_hdr.append(cpl.core.Property("QC FWHMX", cpl.core.Type.DOUBLE,
                                              pinhole_loc["fwhm_x"], "[pix] fwhm in x of pinhole"))
        combined_hdr.append(cpl.core.Property("QC FWHMY", cpl.core.Type.DOUBLE,
                                              pinhole_loc["fwhm_y"], "[pix] fwhm in y of pinhole"))


        return [
            self.ProductCombined(self, header=combined_hdr, image=combined_img),
            self.ProductBackground(self, header=background_hdr, image=background_img),
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

    parameters = cpl.ui.ParameterList()
    # --stacking.method
    p = cpl.ui.ParameterEnum(
        name=f"{_name}.stacking.method",
        context=_name,
        description="Name of the method used to combine the input images",
        default="average",
        alternatives=("add", "average", "median", "sigclip"),
    )
    p.cli_alias = "stacking.method"
    parameters.append(p)

    # --halfwindow
    p = cpl.ui.ParameterRange(
        name=f"{_name}.halfwindow",
        context=_name,
        description="Half size of window for centroid determination [pix]",
        default=15,
        min=1,
        max=1024
    )
    p.cli_alias = "halfwindow"
    parameters.append(p)

    implementation_class = MetisCalChophomeImpl


def locate_pinhole(cimg: cpl.core.Image):
    """Locate the pinhole on cimg"""
    # Rough location: brightest pixel
    # -- this may not be robust enough
    # -- maybe use first guess based on chopper keywords?
    x0, y0 = cimg.get_maxpos()
    # TODO: turn window size into a recipe parameter
    xcen = cimg.get_centroid_x(window=(x0-15, y0-15, x0+15, y0+15))
    ycen = cimg.get_centroid_x(window=(x0-15, y0-15, x0+15, y0+15))
    fwhm_x, fwhm_y = cimg.get_fwhm(round(xcen), round(ycen))
    result = {"xcen": xcen, "ycen": ycen, "fwhm_x": fwhm_x, "fwhm_y": fwhm_y}
    return result
