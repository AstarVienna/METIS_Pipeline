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
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterRange

from pymetis.classes.dataitems.chophome import LmChophomeRaw, LmChophomeCombined, LmChophomeBackground
from pymetis.classes.dataitems.gainmap import GainMap2rg
from pymetis.classes.dataitems.linearity.linearity import LinearityMap2rg
from pymetis.classes.dataitems.raw.wcuoff import LmWcuOffRaw
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import (RawInput, GainMapInput, PersistenceMapInput, BadpixMapInput,
                                    PinholeTableInput, LinearityInput, OptionalInputMixin)
from pymetis.classes.prefab import RawImageProcessor


class MetisCalChophomeImpl(RawImageProcessor):  # TODO replace parent class?
    """Implementation class for metis_cal_chophome"""
    class InputSet(RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            Item = LmChophomeRaw

        class BackgroundInput(RawInput):
            Item = LmWcuOffRaw

        class GainMapInput(OptionalInputMixin, GainMapInput):
            Item = GainMap2rg

        class LinearityInput(OptionalInputMixin, LinearityInput):
            Item = LinearityMap2rg

        PersistenceMapInput = PersistenceMapInput

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            pass

        class PinholeTableInput(OptionalInputMixin, PinholeTableInput):
            pass

    ProductCombined = LmChophomeCombined
    ProductBackground = LmChophomeBackground

    def process_images(self):
        """This function processes the input images

        - stack the wcu_off images into background_img
        - subtract background_img from raw_images and stack
        - locate pinhole on the combined image
        """

        stackmethod = self.parameters[f"{self.name}.stacking.method"].value
        hwidth = self.parameters[f"{self.name}.halfwindow"].value

        background_hdr = cpl.core.PropertyList()
        bg_images = self.load_images(self.inputset.background.frameset)
        background_img = self.combine_images(bg_images, stackmethod)
        # TODO: define usedframes

        combined_hdr = cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)
        raw_images.subtract_image(background_img)
        combined_img = self.combine_images(raw_images, stackmethod)

        # Locate the pinhole image
        pinhole_loc = locate_pinhole(combined_img, hwidth)

        if pinhole_loc["fwhm_x"] is None or pinhole_loc["fwhm_y"] is None:
            Msg.warning(self.__class__.__qualname__,
                        "detection of pinhole failed")
            pinhole_loc["fwhm_x"] = 999
            pinhole_loc["fwhm_y"] = 999

        # Extract QC parameters
        combined_hdr.append(cpl.core.Property("QC CAL CHOPHOME XCEN",
                                              cpl.core.Type.DOUBLE,
                                              pinhole_loc["xcen"],
                                              "[pix] x position of pinhole"))
        combined_hdr.append(cpl.core.Property("QC CAL CHOPHOME YCEN",
                                              cpl.core.Type.DOUBLE,
                                              pinhole_loc["ycen"],
                                              "[pix] y position of pinhole"))
        combined_hdr.append(cpl.core.Property("QC CAL CHOPHOME FWHMX",
                                              cpl.core.Type.DOUBLE,
                                              pinhole_loc["fwhm_x"],
                                              "[pix] fwhm in x of pinhole"))
        combined_hdr.append(cpl.core.Property("QC CAL CHOPHOME FWHMY",
                                              cpl.core.Type.DOUBLE,
                                              pinhole_loc["fwhm_y"],
                                              "[pix] fwhm in y of pinhole"))
        combined_hdr.append(cpl.core.Property("QC CAL CHOPHOME SNR",
                                              cpl.core.Type.DOUBLE,
                                              pinhole_loc["snr"],
                                              "signal-to-noise ratio of pinhole image"))

        return {
            self.ProductCombined(combined_hdr, combined_img),
            self.ProductBackground(background_hdr, background_img),
        }

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
    _description: str = """
        (nothing yet)
    """

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT'}
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

    implementation_class = MetisCalChophomeImpl


def locate_pinhole(cimg: cpl.core.Image, hwidth: int):
    """Locate the pinhole on cimg

    Parameters
    ----------
    - cimg  : cpl.core.Image
    - hwidth : int
          half-width of window around pixel with maximum value

    Returns
    -------
    A dictionary with parameters:
    - xcen, ycen : [pix] location of the centroid of the pinhole image
    - fwhm_x, fwhm_y: [pix] full-width at half maximum of the pinhole
                            image in x- and y-direction
    - snr: signal-to-noise ratio determined as total flux in window divided
           by pixel stdev times number of pixels in window


    Note
    ----
    The function uses a fairly rough algorithm. Its robustness depends on the
    pinhole image having high signal-to-noise without being saturated.
    """
    # Rough location: brightest pixel
    y0, x0 = cimg.get_maxpos()

    # Analyse window around maximum position
    llx = max(x0 - hwidth, 0)
    lly = max(y0 - hwidth, 0)
    urx = min(x0 + hwidth, cimg.shape[1]-1)
    ury = min(y0 + hwidth, cimg.shape[0]-1)
    win = (llx, lly, urx, ury)

    xcen = cimg.get_centroid_x(window=win)
    ycen = cimg.get_centroid_y(window=win)
    fwhm_y, fwhm_x = cimg.get_fwhm(y0, x0)

    # Signal-to-noise ration using flux over the window and pixel noise
    # over the image
    flux = cimg.get_flux(win)
    noise = cimg.get_stdev() * 2 * hwidth
    print("FLUX:", flux)
    print("NOISE:", noise)

    result = {"xcen": xcen, "ycen": ycen, "fwhm_x": fwhm_x, "fwhm_y": fwhm_y,
              "snr": flux / noise}
    return result
