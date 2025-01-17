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
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct, TargetSpecificProduct
from pymetis.inputs import RawInput
from pymetis.inputs.common import MasterDarkInput, LinearityInput, PersistenceMapInput, GainMapInput, MasterFlatInput
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisLmImgBasicReduceImpl(DarkImageProcessor):
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
        # but does not know about the tags yet. So here we define tags for the raw input:
        class RawInput(RawInput):
            _tags = re.compile(r"LM_IMAGE_(?P<target>SCI|STD)_RAW")

        # Now we need a master dark. Since nothing is changed and the tag is always the same,
        # we just point to the provided MasterDarkInput.
        MasterDarkInput = MasterDarkInput

        # Also one master flat is required. Again, we use a prefabricated class, but reset the tags
        class MasterFlat(MasterFlatInput):
            _tags = re.compile(r"MASTER_IMG_FLAT_(?P<source>LAMP|TWILIGHT)_(?P<band>LM)")


        # Alternatively, we could directly use MasterFlatInput(tags=re.compile(r"...")) in __init__,
        # or go fully manual and specify it as
        # SinglePipelineInput(tags=re.compile(r"..."), title="master flat", group=cpl.ui.Frame.FrameGroup.CALIB)

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.master_flat = self.MasterFlat(frameset)
            self.linearity = LinearityInput(frameset)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.gain_map = GainMapInput(frameset)

            # We need to register the inputs (just to be able to say `for x in self.inputs:`)
            self.inputs += [self.master_flat, self.linearity, self.persistence, self.gain_map]

    class Product(TargetSpecificProduct):
        """
        The second big part is defining the products. For every product we create a separate class
        which defines the tag, group, level and frame type. Here we only have one kind of product,
        so its name is `Product` (or fully qualified, `MetisLmImgBasicReduceImpl.Product`).
        But feel free to be more creative with names.
        """
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return rf"LM_{self.target:s}_REDUCED"

        @property
        def output_file_name(self):
            return f"{self.category}.fits"

        @property
        def tag(self) -> str:
            return rf"{self.category}"


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

        for index, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file}...")

            Msg.debug(self.__class__.__qualname__, f"Loading image {frame.file}")
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
        combined_image = self.combine_images(images, self.parameters["metis_lm_img_basic_reduce.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        self.target = "SCI" # hardcoded for now
        self.products = {
            fr'OBJECT_REDUCED_{self.detector}':
                self.Product(self, header, combined_image, target=self.target),
        }

        return self.products


class MetisLmImgBasicReduce(MetisRecipe):
    """
    Apart from our own recipe implementation we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the

    - seven required attributes as below
        - copyright may be omitted as it is provided in the base class and probably will remain the same everywhere,
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally point to the implementation class, which we have just written
    """
    # Fill in recipe information
    _name = "metis_lm_img_basic_reduce"
    _version = "0.1"
    _author = "A*"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic science image data processing"
    _description = (
            "The recipe combines all science input files in the input set-of-frames using\n"
            + "the given method. For each input science image the master bias is subtracted,\n"
            + "and it is divided by the master flat."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_basic_reduce.stacking.method",
            context="metis_lm_img_basic_reduce",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])
    implementation_class = MetisLmImgBasicReduceImpl
