"""
METIS pupil imaging recipe

This file contains the recipe for reducing pupil imaging raw data for the
METIS instrument. It will apply dark and flat corrections, and optionally
gain and persistance corrections. It can be directly via pyesorex,
or as part of an edps workflow.

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
from pymetis.classes.products import PipelineProduct
from pymetis.classes.inputs import RawInput
from pymetis.classes.inputs import MasterDarkInput, MasterFlatInput
from pymetis.classes.inputs import LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.prefab.darkimage import DarkImageProcessor

class MetisPupilImagingImpl(DarkImageProcessor):
    class InputSet(LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        Define the input sets and tags.
        Here, we define dark, flat, linearity, persistence and gain map
        and the tags for PUPIL_RAW

        TODO; currently works for LM band, need to set up to work for both LM and N with proper filtering.
        """

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"(?P<band>LM|N)_PUPIL_RAW")
            _description = "Raw exposure of the pupil in LM image mode" # FixMe N band

        MasterDarkInput = MasterDarkInput

        # Also one master flat is required. We use a prefabricated class
        class MasterFlatInput(MasterFlatInput):
            _tags: re.Pattern = re.compile(r"MASTER_IMG_FLAT_(?P<target>LAMP|TWILIGHT)_(?P<band>LM|N)")


    class ProductReduced(PipelineProduct):
        """
        Define the output product, here a reduced pupil image.
        """
        _tag = r"LM_PUPIL_IMAGING_REDUCED"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        band = "LM"
        _description = "Reduced pupil image in LM mode"
        _oca_keywords = {'PRO.CATG', 'DRS.PUPIL'}

        @classmethod
        def tag(cls):
            return rf"{cls.band}_PUPIL_IMAGING_REDUCED"

    def prepare_flat(self, flat: cpl.core.Image, bias: cpl.core.Image | None):
        """ Flat field preparation: subtract bias and normalize it to median 1 """
        Msg.info(self.__class__.__qualname__, "Preparing flat field")

        if flat is None:
            raise RuntimeError("No flat frames found in the frameset.")
        else:
            if bias is not None:
                flat.subtract(bias)
            median = flat.get_median()
            return flat

            # return flat.divide_scalar(median)

    def prepare_images(self,
                       raw_frames: cpl.ui.FrameSet,
                       bias: cpl.core.Image | None = None,
                       flat: cpl.core.Image | None = None) -> cpl.core.ImageList:
        prepared_images = cpl.core.ImageList()

        """Prepare the images; bias subtracting and flat fielding"""

        for index, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file!r}...")

            Msg.debug(self.__class__.__qualname__, f"Loading image {frame.file!r}")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            if bias:
                Msg.debug(self.__class__.__qualname__, "Bias subtracting...")
                raw_image.subtract(bias)

            if flat:
                Msg.debug(self.__class__.__qualname__, "Flat fielding...")
                raw_image.divide(flat)

            prepared_images.append(raw_image)

        return prepared_images

    def process_images(self) -> [PipelineProduct]:
        """
        Runner for processing images. Currently setup to do dark/bias/flat/gain plus combining images.
        TODO No actually processing is performed.
        """

        Msg.info(self.__class__.__qualname__, f"Starting processing image attribute.")

        flat = cpl.core.Image.load(self.inputset.master_flat.frame.file, extension=0)
        bias = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)
        gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector}")

        flat = self.prepare_flat(flat, bias)
        images = self.prepare_images(self.inputset.raw.frameset, flat, bias)
        combined_image = self.combine_images(images, self.parameters["metis_pupil_imaging.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        product = self.ProductReduced(self, header, combined_image) # FixMe Hardcoded band for now

        return [product]


class MetisPupilImaging(MetisRecipe):
    """
    Wrapper for the recipe for pyesorex, defining neessary attributes and parameters, plus the implementation class.
    """
    # Fill in recipe information
    _name: str = "metis_pupil_imaging"
    _version: str = "0.1"
    _author: str = "Jennifer Karr, A*"
    _email: str = "jkarr@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Basic processing of pupil images"
    _description: str = """
        This recipe performs basic reduction (dark current subtraction, flat fielding,
        optional bias subtraction, persistence and linearity corrections) on engineering
        images of the pupil masks. This recipe is not expected to be used by observers
        during regular use.""" # FixMe this is not shown anywhere now

    _matched_keywords: {str} = {'DRS.PUPIL'}
    _algorithm = """Apply dark current and flat field corrections."""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_pupil_imaging.stacking.method",
            context="metis_pupil_imaging",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        ),
    ])

    implementation_class = MetisPupilImagingImpl
