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

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput
from pymetis.inputs.common import MasterDarkInput, LinearityInput, PersistenceMapInput, GainMapInput, MasterFlatInput
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisPupilImagingImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        """
        The first step of writing a recipe is to define an InputSet: the singleton class
        that wraps all the recipe inputs. It encapsulates the entire input and

        - defines which tags to look for, optionally with placeholders like `{det}`, that can be set globally
        - which of the inputs are optional (just set `required = False`)
        - provides mechanism for verification that the required frames are actually present

        Inputs are twofold:

        - children of SinglePipelineInput, which expect exactly one frame to be present (or at most one if optional).
            They will warn is multiple frames are found and keep the last one.
        - children of MultiplePipelineInput, which expect multiple frames with the same tag (usually RAWs).
            Again, the frame set may be empty if `required` is set to False.

        You may instantiate your inputs directly as SinglePipelineInput or MultiplePipelineInput with appropriate tags,
        or use or extend one of the predefined classes (see `pymetis.inputs.common`).

        The input is automatically verified (see the base InputSet class and its Single and Multiple children)
        and an exception is raised whenever something is amiss.
        """

        # This InputSet class derives from DarkImageProcessor.InputSet, which in turn inherits from
        # RawImageProcessor.InputSet. It already knows that it wants a RawInput and MasterDarkInput class,
        # but does not know about the tags yet. So here we define tags for the raw input
        class Raw(RawInput):
            _tags = re.compile("LM_PUPIL_RAW")

        # Also one master flat is required. We use a prefabricated class
        class MasterFlat(MasterFlatInput):
            _tags = re.compile("MASTER_IMG_FLAT_LAMP_LM")

        # We could define the master dark explicitly too, but we can use a prefabricated class instead.
        # That already has its tags defined (for master darks it's always "MASTER_DARK_{det}"), so we just define
        # the detector and band. Those are now available for all Input classes here.
        # Of course, we could be more explicit and define them directly.

        RawInput = Raw
        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.master_flat = self.MasterFlat(frameset,
                                               tags=re.compile("MASTER_IMG_FLAT_(?P<target>LAMP|TWILIGHT)_(?P<band>LM|N)"),
                                               band="LM", det=self.detector)
            self.linearity = LinearityInput(frameset, det=self.detector)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.gain_map = GainMapInput(frameset, det=self.detector)

            # We need to register the inputs (just to be able to do `for x in self.inputs:`)
            self.inputs |= {self.master_flat, self.linearity, self.persistence, self.gain_map}

    class Product(PipelineProduct):
        """
        The second big part is defining the products. For every product we create a separate class
        which defines the tag, group, level and frame type.
        """
        tag: str = "LM_PUPIL_IMAGING_REDUCED"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return self.tag

        @property
        def output_file_name(self):
            return f"{self.category}.fits"

    def prepare_flat(self, flat: cpl.core.Image, bias: cpl.core.Image | None):
        """ Flat field preparation: subtract bias and normalize it to median 1 """
        Msg.info(self.__class__.__qualname__, "Preparing flat field")
        
        #import pdb ; pdb.set_trace()
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

    def process_images(self) -> Dict[str, PipelineProduct]:
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions, or use functions from the parent classes:
        for instance combine_images is a helper function that takes a frameset and a method and returns
        a single combined frame that is used throughout the pipeline.
        """

        Msg.info(self.__class__.__qualname__, f"Starting processing image attribute.")

        flat = cpl.core.Image.load(self.inputset.master_flat.frame.file, extension=0)
        bias = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)
        gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector}")

        flat = self.prepare_flat(flat, bias)
        images = self.prepare_images(self.inputset.raw.frameset, flat, bias)
        combined_image = self.combine_images(images, self.parameters["pupil_imaging.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        self.products = {
            fr'{self.detector}_PUPIL_REDUCED':
                self.Product(self, header, combined_image, detector=self.detector),
        }

        return self.products


class MetisPupilImaging(MetisRecipe):
    """
    Apart from our own recipe implementation we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the

    - seven required attributes as below (copyright may be omitted as it is provided in the base class),
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally define the implementation class, which we have just written
    """
    # Fill in recipe information
    _name = "metis_pupil_imaging"
    _version = "0.1"
    _author = "Jennifer Karr"
    _email = "jkarr@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic processing of pupil images"
    _description = (
            "This recipe performs basic reduction (dark current subtraction, flat fielding,\n"
            "optional bias subtraction, persistance and linearity corrections) on engineering\n"
            "images of the pupil masks. This recipe is not expected to be used by observers\n"
            "during regular use."
    )

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
