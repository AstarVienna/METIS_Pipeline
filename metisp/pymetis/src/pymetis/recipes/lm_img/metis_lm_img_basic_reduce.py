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

import os
import re

import copy
import cpl
from cpl.core import Msg

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems.img.raw import LmImageStdRaw
from pymetis.classes.dataitems.img.basicreduced import LmStdBasicReduced, LmSciBasicReduced, LmSkyBasicReduced
from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.mixins.target import TargetSkyMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.products import (PipelineProduct, TargetSpecificProduct,
                                      PipelineMultipleProduct)
from pymetis.classes.inputs import (RawInput, MasterDarkInput, MasterFlatInput,
                                    PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin)
from pymetis.classes.prefab.darkimage import DarkImageProcessor


class MetisLmImgBasicReduceImpl(DarkImageProcessor):
    detector = '2RG'

    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        The first step of writing a recipe is to define an InputSet: the one-to-one class
        that wraps all the recipe inputs. It encapsulates the entire input and
        - defines which tags to look for, optionally with placeholders like `{det}`, that can be set globally
        - which of the inputs are optional (just set `required = False`)
        - provides a mechanism for verification that the required frames are actually present

        Inputs are twofold:
        - Children of SinglePipelineInput, which expect exactly one frame to be present (or at most one if optional).
            They will warn if multiple frames are found and keep the last one.
        - Children of MultiplePipelineInput, which expect multiple frames with the same tag (usually RAWs).
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
            Item = LmImageStdRaw
            _tags: re.Pattern = re.compile(r"LM_IMAGE_(?P<target>SCI|SKY|STD)_RAW")
            _description: str = "Raw exposure of a standard star in the LM image mode."
            # FIXME (or better, fix the DRLD): SKY is not documented, but it is requested by other recipes.
            #    See https://github.com/AstarVienna/METIS_DRLD/issues/321

        # Now we need a master dark frame. Since nothing is changed and the tag is always the same,
        # we just point to the provided MasterDarkInput. Note that we do not have to instantiate
        # it explicitly anywhere, `MasterDarkInput` takes care of that for us.
        MasterDarkInput = MasterDarkInput

        # Also, one master flat is required. Again, we use a prefabricated class but reset the tags
        class MasterFlatInput(MasterFlatInput):
            _tags: re.Pattern = re.compile(r"MASTER_IMG_FLAT_(?P<source>LAMP|TWILIGHT)_LM")
            _description: str = "Master flat frame for LM image data."

    class ProductBasicReduced(TargetSpecificProduct, PipelineMultipleProduct):
        """
        The second big part is defining the products. For every product, we create a separate class
        which defines the tag, group, level and frame type. Here we only have one kind of product,
        so its name is `Product` (or fully qualified, `MetisLmImgBasicReduceImpl.Product`).
        But feel free to be more creative with names: it could be `MetisLmImgBasicReduceImpl.ProductBasicReduced`.
        """
        Item = LmStdBasicReduced

        def __init__(self,
                     recipe_impl: 'MetisRecipeImpl',
                     header: cpl.core.PropertyList,
                     *,
                     image: cpl.core.Image,
                     noise: cpl.core.Image,
                     mask: cpl.core.Image,
                     original_file_name: str):
            super().__init__(recipe_impl, header, image=image, noise=noise, mask=mask)
            self.original_file_name: str = original_file_name

        @classmethod
        def tag(cls) -> str:
            return rf"LM_{cls.target()}_BASIC_REDUCED"

        @property
        def output_file_name(self) -> str:
            """
            Form the output file name.
            By default, this should be just the category with ".fits" appended. Feel free to override if needed.
            """
            return f"{self.category}_{self.original_file_name}"

    def process_images(self) -> set[PipelineProduct]:
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions or use functions from the parent classes:
        for instance, `combine_images` is a helper function that takes a frameset and a method and returns
        a single combined frame that is used throughout the pipeline.
        """

        Msg.info(self.__class__.__qualname__, "Processing images")

        Msg.info(self.__class__.__qualname__, "Loading calibration files")

        flat = cpl.core.Image.load(self.inputset.master_flat.frame.file, extension=0)
        dark = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)
        gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector}")

        Msg.info(self.__class__.__qualname__, "Loading raw images")
        images = self.inputset.load_raw_images()
        Msg.info(self.__class__.__qualname__, "Pretending to correct crosstalk")
        Msg.info(self.__class__.__qualname__, "Pretending to correct for linearity")

        Msg.info(self.__class__.__qualname__, "Subtracting Dark")
        images.subtract_image(dark)

        Msg.info(self.__class__.__qualname__, "Flat fielding")
        images.divide_image(flat)

        Msg.info(self.__class__.__qualname__, "Pretending to remove masked regions")

        Msg.info(self.__class__.__qualname__, "Combining Images")

        # combined_image = self.combine_images(images,
        #                                      self.parameters["metis_lm_img_basic_reduce.stacking.method"].value)

        product_set: set[PipelineProduct] = set()
        for i, image in enumerate(images):
            frame = self.inputset.raw.frameset[i]

            Msg.info(self.__class__.__qualname__, f"Processing frame {frame.file}")

            header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.info(self.__class__.__qualname__, "Pretending to calculate noise")

            a = copy.deepcopy(image)
            noise = cpl.core.Image(image)
            noise.copy_into(image, 0, 0)
            noise.power(0.5)

            bmask = cpl.core.Image(a)
            bmask.copy_into(image, 0, 0)
            bmask.multiply_scalar(0)

            Msg.info(self.__class__.__qualname__, "Pretending to calculate bad pixels")
            Msg.info(self.__class__.__qualname__, "Actually Calculating QC Parameters")
            Msg.info(self.__class__.__qualname__, "Appending QC Parameters to header")

            header.append(cpl.core.Property("QC LM IMG MEDIAN", cpl.core.Type.DOUBLE,
                                            image.get_median(), "[ADU] median value of image"))
            header.append(cpl.core.Property("QC LM IMG STDEV", cpl.core.Type.DOUBLE,
                                            image.get_median(), "[ADU] stddev value of image"))
            header.append(cpl.core.Property("QC LM IMG MAX", cpl.core.Type.DOUBLE,
                                            image.get_median(), "[ADU] max value of image"))

            self.target = self.inputset.tag_parameters['target']

            product = self.ProductBasicReduced(self, header, image=image, noise=noise, mask=bmask,
                                               original_file_name=os.path.basename(frame.file))
            product_set |= {product}

        return product_set

    def _dispatch_child_class(self) -> type["MetisLmImgBasicReduceImpl"]:
        return {
            'STD': MetisLmStdBasicReduceImpl,
            'SCI': MetisLmSciBasicReduceImpl,
            'SKY': MetisLmSkyBasicReduceImpl,
        }[self.inputset.target]


# ToDo Generate these classes automatically!

class MetisLmStdBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class InputSet(MetisLmImgBasicReduceImpl.InputSet):
        class RawInput(MetisLmImgBasicReduceImpl.InputSet.RawInput):
            Item = LmImageStdRaw

    class ProductBasicReduced(TargetStdMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced):
        Item = LmStdBasicReduced


class MetisLmSciBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class ProductBasicReduced(TargetSciMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced):
        Item = LmSciBasicReduced


class MetisLmSkyBasicReduceImpl(MetisLmImgBasicReduceImpl):
    class ProductBasicReduced(TargetSkyMixin, MetisLmImgBasicReduceImpl.ProductBasicReduced):
        Item = LmSkyBasicReduced


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
        "the given method. For each input science image the master dark is subtracted,\n"
        "and it is divided by the master flat."
    )

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.FILTER'}
    _algorithm = """Remove crosstalk, correct non-linearity
    Analyse and optionally remove masked regions
    Subtract dark, divide by flat
    Remove blank sky pattern"""

    parameters = ParameterList([
        ParameterEnum(
            name=rf"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    implementation_class = MetisLmImgBasicReduceImpl
