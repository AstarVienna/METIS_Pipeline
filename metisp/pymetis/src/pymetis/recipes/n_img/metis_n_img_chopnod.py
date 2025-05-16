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
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.products import PipelineProduct, PipelineImageProduct, TargetSpecificProduct
from pymetis.classes.inputs import (RawInput, MasterDarkInput, MasterFlatInput,
                                    PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin)
from pymetis.classes.prefab.darkimage import DarkImageProcessor

class MetisNImgChopnodImpl(DarkImageProcessor):
    detector = '2RG'

    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        The first step of writing a recipe is to define an InputSet:
        the one-to-one class that wraps all the recipe inputs.
        It encapsulates the entire input and
        - defines which tags to look for, optionally with placeholders like `{det}` that can be set globally
        - defines which of the inputs are optional (just set `required = False`)
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

        # This InputSet class derives from `DarkImageProcessor.InputSet`,
        # which in turn inherits from `RawImageProcessor.InputSet`.
        # It already knows that it wants a RawInput and MasterDarkInput class
        # but does not know about the tags yet. So here we define tags for the raw input:
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"N_IMAGE_(?P<target>SCI|STD)_RAW")
            _description: str = "Raw exposure of a standard star in the N image mode."

        # Now we need a master dark frame.
        # Since nothing is changed and the tag is always the same, # we just point to the provided MasterDarkInput.
        # Note that we do not have to instantiate it explicitly anywhere, `MasterDarkInput` takes care of that for us.
        MasterDarkInput = MasterDarkInput

        # Also one master flat is required. Again, we use a prefabricated class but reset the tags
        class MasterFlatInput(MasterFlatInput):
            _tags: re.Pattern = re.compile(r"MASTER_IMG_FLAT_(?P<source>LAMP|TWILIGHT)_(?P<band>N)")
            _description: str = "Master flat frame for N image data."

    class ProductReduced(TargetSpecificProduct, PipelineImageProduct):
        """
        The second big part is defining the products. For every product, we create a separate class
        which defines the tag, group, level and frame type. Here we only have one kind of product,
        so its name is `Product` (or fully qualified, `MetisNImgChopnodImpl.Product`).
        But feel free to be more creative with names: it could be `MetisNImgChopnodImpl.ProductBasicReduced`.
        """
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}
        _description: str = "Science grade detrended exposure of the N image mode."


        @classmethod
        def tag(cls) -> str:
            return rf"N_{cls.target():s}_BKG_SUBTRACTED"

    class ProductBackground(TargetSpecificProduct, PipelineImageProduct):
        """
        The second big part is defining the products.
        For every product, we create a separate class which defines the tag, group, level and frame type.
        Here we only have one kind of product, so its name is `Product`
        (or fully qualified, `MetisNImgChopnodImpl.Product`).
        But feel free to be more creative with names: it could be `MetisNImgChopnodImpl.ProductBasicReduced`.
        """
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}
        _description: str = "Science grade detrended exposure of the N image mode."

        @classmethod
        def tag(cls) -> str:
            return rf"N_{cls.target():s}_BACKGROUND"

    def prepare_images(self,
                       raw_frames: cpl.ui.FrameSet) -> cpl.core.ImageList:
        prepared_images = cpl.core.ImageList()

        for index, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file}...")

            Msg.debug(self.__class__.__qualname__, f"Loading image {frame.file}")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            prepared_images.append(raw_image)

        return prepared_images

    def process_images(self) -> [PipelineProduct]:
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions or use functions from the parent classes:
        for instance, combine_images is a helper function that takes a frameset and a method and returns
        a single combined frame that is used throughout the pipeline.
        """

        Msg.info(self.__class__.__qualname__, f"Processing Images")
        Msg.info(self.__class__.__qualname__, f"Loading calibration files")

        flat = cpl.core.Image.load(self.inputset.master_flat.frame.file, extension=0)
        dark = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)
        gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)
        images = self.prepare_images(self.inputset.raw.frameset)

        combined_image = self.combine_images(images, self.parameters["metis_n_img_chopnod.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        self.target = self.inputset.tag_parameters['target']

        productR = self.ProductReduced(self, header, combined_image)
        productB = self.ProductBackground(self, header, combined_image)

        return [productR, productB]

    def _dispatch_child_class(self) -> type["MetisNImgChopnodImpl"]:
        return {
            'STD': MetisNStdImgChopnodImpl,
            'SCI': MetisNSciImgChopnodImpl,
        }[self.inputset.target]


class MetisNStdImgChopnodImpl(MetisNImgChopnodImpl):
    class ProductReduced(TargetStdMixin, MetisNImgChopnodImpl.ProductReduced): pass
    class ProductBackground(TargetStdMixin, MetisNImgChopnodImpl.ProductBackground): pass


class MetisNSciImgChopnodImpl(MetisNImgChopnodImpl):
    class ProductReduced(TargetSciMixin, MetisNImgChopnodImpl.ProductReduced): pass
    class ProductBackground(TargetSciMixin, MetisNImgChopnodImpl.ProductBackground): pass



class MetisNImgChopnod(MetisRecipe):
    """
    Apart from our own recipe implementation, we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the
    - seven required attributes as below
        - copyright may be omitted as it is provided in the base class and probably will remain the same everywhere,
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally point to the implementation class, which we have just written
    """
    # Fill in recipe information
    _name: str = "metis_n_img_chopnod"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Basic science image data processing"
    _description: str = (
            "The recipe combines all science input files in the input set-of-frames using\n"
            + "the given method. For each input science image the master dark is subtracted,\n"
            + "and it is divided by the master flat."
    )

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.FILTER'}
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

    implementation_class = MetisNImgChopnodImpl
