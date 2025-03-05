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

from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.mixins.target import TargetSkyMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.products import TargetSpecificProduct
from pymetis.classes.products import PipelineProduct
from pymetis.classes.inputs import (RawInput, MasterDarkInput, MasterFlatInput,
                                    PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin)
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.headers.header import Header, ProCatg, InsOpti3Name, InsOpti9Name, InsOpti10Name, DrsFilter, \
    DetDit, DetNDit


class MetisLmImgBasicReduceImpl(DarkImageProcessor):
    detector = '2RG'

    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        The first step of writing a recipe is to define an InputSet: the one-to-one class
        that wraps all the recipe inputs. It encapsulates the entire input and

        - defines which tags to look for, optionally with placeholders like `{det}`, which can be set globally
        - which of the inputs are optional (just set `required = False`)
        - provides a mechanism for verification that the required frames are actually present

        Inputs are twofold:

        - children of SinglePipelineInput, which expect exactly one frame to be present (or at most one if optional).
            They will warn if multiple frames are found and keep the last one.
        - children of MultiplePipelineInput, which expect multiple frames with the same tag (usually RAWs).
            Again, the frame set may be empty if `required` is set to False.

        You may instantiate your inputs directly as SinglePipelineInput or MultiplePipelineInput with appropriate tags,
        or use or extend one of the predefined classes (see `pymetis.inputs.common`).

        The input is automatically verified (see the base InputSet class and its Single and Multiple children),
        and an exception is raised whenever something is amiss.
        """

        # This InputSet class derives from `DarkImageProcessor.InputSet`, which in turn inherits from
        # `RawImageProcessor.InputSet`. It already knows that it wants a RawInput and MasterDarkInput class,
        # but does not know about the tags yet. So here we define tags for the raw input:
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_IMAGE_(?P<target>SCI|SKY|STD)_RAW")
            _description: str = "Raw exposure of a standard star in the LM image mode."
            # FIXME (or better, fix the DRLD): SKY is not documented, but it is requested by other recipes.
            #    See https://github.com/AstarVienna/METIS_DRLD/issues/321

        # Now we need a master dark. Since nothing is changed and the tag is always the same,
        # we just point to the provided MasterDarkInput. Note that we do not have to instantiate
        # it explicitly anywhere, `MasterDarkInput` takes care of that for us.
        MasterDarkInput = MasterDarkInput

        # Also one master flat is required. Again, we use a prefabricated class, but reset the tags
        class MasterFlatInput(MasterFlatInput):
            _tags: re.Pattern = re.compile(r"MASTER_IMG_FLAT_(?P<source>LAMP|TWILIGHT)_(?P<band>LM)")
            _description: str = "Master flat frame for LM image data."

    class ProductBasicReduced(TargetSpecificProduct):
        """
        The second big part is defining the products. For every product we create a separate class
        which defines the tag, group, level and frame type. Here we only have one kind of product,
        so its name is `Product` (or fully qualified, `MetisLmImgBasicReduceImpl.Product`).
        But feel free to be more creative with names: it could be `MetisLmImgBasicReduceImpl.ProductBasicReduced`.
        """
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords: {Header} = {ProCatg, InsOpti3Name, InsOpti9Name, InsOpti10Name, DrsFilter}
        _description: str = "Science grade detrended exposure of the LM image mode."

        @classmethod
        def tag(cls) -> str:
            return rf"LM_{cls.target():s}_BASIC_REDUCED"



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

    def process_images(self) -> [PipelineProduct]:
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions, or use functions from the parent classes:
        for instance, combine_images is a helper function that takes a frameset and a method and returns
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

        self.target = self.inputset.tag_parameters['target']

        product = self.ProductBasicReduced(self, header, combined_image)

        return [product]

    def _dispatch_child_class(self) -> type["MetisLmImgBasicReduceImpl"]:
        return {
            'STD': MetisLmStdBasicReduceImpl,
            'SCI': MetisLmSciBasicReduceImpl,
            'SKY': MetisLmSkyBasicReduceImpl,
        }[self.inputset.target]


class MetisLmStdBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class ProductBasicReduced(TargetStdMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced): pass

class MetisLmSciBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class ProductBasicReduced(TargetSciMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced): pass

class MetisLmSkyBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class ProductBasicReduced(TargetSkyMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced): pass


class MetisLmImgBasicReduce(MetisRecipe):
    """
    Apart from our own recipe implementation, we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the

    - seven required attributes as below
        - copyright may be omitted as it is provided in the base class and probably will remain the same everywhere,
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally point to the implementation class, which we have just written
    """
    # Fill in recipe information
    _name: str = "metis_lm_img_basic_reduce"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Basic science image data processing"
    _description: str = (
            "The recipe combines all science input files in the input set-of-frames using\n"
            + "the given method. For each input science image the master bias is subtracted,\n"
            + "and it is divided by the master flat."
    )

    _matched_keywords: {Header} = {DetDit, DetNDit, DrsFilter}
    _algorithm = """Remove crosstalk, correct non-linearity
        Analyse and optionally remove masked regions
        Subtract dark, divide by flat
        Remove blank sky pattern"""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=rf"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    implementation_class = MetisLmImgBasicReduceImpl
