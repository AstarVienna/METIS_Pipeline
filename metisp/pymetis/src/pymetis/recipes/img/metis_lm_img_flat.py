import cpl

from pymetis.base.recipe import MetisRecipe
from pymetis.prefab.flat import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band = "LM"
        detector = "2RG"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "LM"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = ["Kieran Leschinski", "Chi-Hung Yan", "Hugo Buddelmeijer", "Gilles PPL Otten", "Martin Baláž"]
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master flat for L/M band detectors"
    _description = "Prototype to create a METIS Masterflat for L/M band"

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
