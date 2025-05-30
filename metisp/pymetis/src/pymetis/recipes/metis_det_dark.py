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

from pymetis.classes.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import (RawInput, BadpixMapInput, PersistenceMapInput,
                                    LinearityInput, GainMapInput, OptionalInputMixin)
from pymetis.classes.inputs import PersistenceInputSetMixin
from pymetis.classes.products import PipelineProduct, PipelineImageProduct
from pymetis.classes.products import DetectorSpecificProduct


class MetisDetDarkImpl(RawImageProcessor):
    """
    Implementation class for `metis_det_dark`.
    """

    # We start by deriving the implementation class from `MetisRecipeImpl`, or in this case, one of its subclasses,
    # namely `RawImageProcessor, as this recipe processes raw images and we would like to reuse the functionality.

    # First of all, we need to define the input set. Since we are deriving from `RawImageProcessor`,
    # we need to reuse the `InputSet` class from it too. This automatically adds a `RawInput` for us.
    class InputSet(PersistenceInputSetMixin, RawImageProcessor.InputSet):
        """
        InputSet class for `metis_det_dark`.
        """

        # However, we still need to define the tags on the class level.
        # Therefore, we override the `_tags` attribute and also the description,
        # since this is specific to this raw input, not all raw inputs.
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"DARK_(?P<detector>2RG|GEO|IFU)_RAW")
            _description: str = "Raw data for creating a master dark."

        # Next, we define all other input classes using predefined ones.
        # Here we mark them as optional, but if we did not need that, we could have also said
        # ```PersistenceMapInput = PersistenceMapInput```
        # to tell the class that its persistence map input is just the global `PersistenceMapInput` class.
        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            pass

        # FixMe: these two should not be optional, but the current EDPS workflow does not supply them
        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

    # Next, we have to define all the product classes for this recipe. Here we only have one, the master dark frame.
    # Note that master darks might be obtained by different means for different detectors,
    # so we derive from `DetectorSpecificProduct`, not from `PipelineProduct`.
    # This provides a mechanism for ensuring the `detector` is defined and parsed.
    class ProductMasterDark(DetectorSpecificProduct, PipelineImageProduct):
        """
        Master dark frame product.
        """

        # We define the required attributes for CPL.
        level = cpl.ui.Frame.FrameLevel.FINAL
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        # The actual description depends on the detector, so we need to redefine it.
        # If it did not, we would just redefine `_description: str = "*describe* *describe*"`,
        # but here we have to override the wrapping @classmethod too.
        # Note that this is a @classmethod: it depends on the class and not on the instance,
        # and we can call it without loading any data. This is useful for `pyesorex --man-page metis_det_dark`.
        @classmethod
        def description(cls) -> str:
            return f"Master dark frame for '{cls.detector()}' detector data"

        # The same goes for `tag`.
        @classmethod
        def tag(cls) -> str:
            return rf"MASTER_DARK_{cls.detector()}"

    # At this point, we should have all inputs and outputs defined -- the "what" part of the recipe implementation.
    # Now we define the "how" part, or the actions to be performed on the data.
    # See the documentation of the parent's `process_images` function for more details.
    # Feel free to define other functions to break up the algorithm into more manageable chunks
    # and call them from within `process_images` as needed.
    def process_images(self) -> set[PipelineProduct]:
        method = self.parameters["metis_det_dark.stacking.method"].value
        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        raw_images = self.inputset.load_raw_images()
        combined_image = self.combine_images(raw_images, method)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        product = self.ProductMasterDark(self, header, combined_image)

        return {product}

    # For recipes that can further specialize based on the provided data, we need to provide a mechanism
    # to select the correct derived class.
    # Here, it depends on the detector.
    def _dispatch_child_class(self) -> type['MetisDetDarkImpl']:
        """
        Find the implementation class based on the detector specified in the inputset tags.

        Raises:
        KeyError
            If the detector obtained from the inputset is not found in the mapping.
        """
        return {
            '2RG': Metis2rgDarkImpl,
            'GEO': MetisGeoDarkImpl,
            'IFU': MetisIfuDarkImpl,
        }[self.inputset.detector]


# Finally, we provide the specialized classes as needed, with mixins or by overriding.
# In most cases adding mixins is enough, so while it is quite verbose, not much is really happening here.
# Note: in the future this might be reworked to utilize metaclasses or some other dark magic.
# For now, we have to do it manually. Pay extra attention to name the classes the same way,
# if you mess up, the resulting bugs are nasty and hard to find.
# Here, we define subclasses for different specializations based on detectors, and that is all.
# If needed, we could also override `process_images` or any other attributes here.
class Metis2rgDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(Detector2rgMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

    class GainMapInput(Detector2rgMixin, MetisDetDarkImpl.InputSet.GainMapInput):
        pass

    class ProductMasterDark(Detector2rgMixin, MetisDetDarkImpl.ProductMasterDark):
        pass


class MetisGeoDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(DetectorGeoMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

    class GainMapInput(DetectorGeoMixin, MetisDetDarkImpl.InputSet.GainMapInput):
        pass

    class ProductMasterDark(DetectorGeoMixin, MetisDetDarkImpl.ProductMasterDark):
        pass


class MetisIfuDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(DetectorIfuMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

        class GainMapInput(DetectorIfuMixin, MetisDetDarkImpl.InputSet.GainMapInput):
            pass

    class ProductMasterDark(DetectorIfuMixin, MetisDetDarkImpl.ProductMasterDark):
        pass


# This is the actual recipe class that is visible by `pyesorex`.
class MetisDetDark(MetisRecipe):
    # Fill in recipe information for `pyesorex`. These are required and checked by `pyesorex`.
    _name: str = "metis_det_dark"
    _version: str = "0.1"
    _author: str = "Hugo Buddelmeijer, A*"
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Create master dark"
    _description: str = (
        "Prototype to create a METIS masterdark."
    )

    # And also fill in information from DRLD. These are specific to METIS and are used to build the description
    # for the man page. Later we would like to be able to compare them directly to DRLD and test for that.
    _matched_keywords: set[str] = set()
    _algorithm: str = """
        - Group files by detector and DIT, based on header keywords
        - Call function metis_determine_dark for each set of files
        - Call metis_update_dark_mask to flag deviant pixels
    """

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    # Point the `implementation_class` to the *top* class of your recipe hierarchy.
    # Promotions should happen at instantiation time.
    implementation_class = MetisDetDarkImpl
