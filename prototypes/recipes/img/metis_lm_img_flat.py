import cpl

from prototypes.base.impl import MetisRecipe
from prototypes.prefabricates.flat import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band = "LM"
        detector = "2RG"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "LM"

    @property
    def detector_name(self) -> str:
        return "2RG"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Gilles Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master flat for L/M band detectors"
    _description = "Prototype to create a METIS Masterflat."

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])
    implementation_class = MetisLmImgFlatImpl
