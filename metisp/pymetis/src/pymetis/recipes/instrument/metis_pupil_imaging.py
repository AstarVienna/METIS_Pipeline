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

from typing import Dict

import cpl
from cpl.core import Msg
import re

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct, BandSpecificProduct
from pymetis.inputs import RawInput
from pymetis.inputs.common import MasterDarkInput, LinearityInput, PersistenceMapInput, GainMapInput, MasterFlatInput
from pymetis.prefab.darkimage import DarkImageProcessor

class MetisPupilImagingImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        """
        Define the input sets and tags. 
        Here, we define dark, flat, linearity, persistence and gain map
        and the tags for PUPIL_RAW

        TODO; currently works for LM band, need to set up to work for both LM and N with proper filtering.
        
        """

        class Raw(RawInput):
            _tags = re.compile(r"(?P<band>LM|N)_PUPIL_RAW")

        # Also one master flat is required. We use a prefabricated class
        class MasterFlat(MasterFlatInput):
            _tags = re.compile(r"MASTER_IMG_FLAT_LAMP_(?P<band>LM|N)")


        RawInput = Raw
        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.master_flat = self.MasterFlat(frameset,
                                               tags=re.compile("MASTER_IMG_FLAT_(?P<target>LAMP|TWILIGHT)_(?P<band>LM|N)"),
                                               det=self.detector)
            self.linearity = LinearityInput(frameset, det=self.detector)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.gain_map = GainMapInput(frameset, det=self.detector)

            # We need to register the inputs (just to be able to do `for x in self.inputs:`)
            self.inputs |= {self.master_flat, self.linearity, self.persistence, self.gain_map}
            # ToDo This is not correct; need to handle both LM and N.
            self.band = 'LM'

    class Product(BandSpecificProduct):
        """
        Define the output product, here a reduced pupil image.
        """
        _tag = r"LM_PUPIL_IMAGING_REDUCED"
        _group = cpl.ui.Frame.FrameGroup.PRODUCT
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self):
            return rf"{self.band}_PUPIL_IMAGING_REDUCED"

        @property
        def output_file_name(self):
            return f"{self.category}.fits"

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

        product = self.Product(self, header, combined_image, band='LM') # ToDo Hardcoded for now

        return [product]


class MetisPupilImaging(MetisRecipe):
    """
    Wrapper for the recipe for pyesorex, defining neessary attributes and parameters, plus the implementation class. 
    """
    # Fill in recipe information
    _name = "metis_pupil_imaging"
    _version = "0.1"
    _author = "Jennifer Karr"
    _email = "jkarr@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic processing of pupil images"
    _description = """
       This recipe performs basic reduction (dark current subtraction, flat fielding,\n
       optional bias subtraction, persistance and linearity corrections) on engineering\n
       images of the pupil masks. This recipe is not expected to be used by observers\n
       during regular use.

       INPUTS
           {band}_PUPIL_RAW
           LINEARITY_{det}
           GAIN_MAP_{det}
           PERSISTENCE_MAP
           MASTER_DARK_{det}
           MASTER_IMG_FLAT_LAMP_{band}
 
           where band/det is LM or N and 2RG or GEO

        OUTPUT
           {band}_PUPIL_REDUCED

        Algorithm
           Apply dark current and flat field corrections and optionally bias subtraction,
           persistence and linearity corrections. 
        """
    

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
