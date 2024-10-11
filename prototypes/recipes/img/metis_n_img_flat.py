import cpl

from prototypes.base import MetisRecipe
from prototypes.flat import MetisBaseImgFlatImpl


class MetisNImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band = "N"
        detector = "GEO"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "N"



class MetisNImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_n_img_flat"
    _version = "0.1"
    _author = ["Kieran Leschinski", "Chi-Hung Yan", "Hugo Buddelmeijer", "Gilles PPL Otten", "Martin Baláž"]
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master flat for N band detectors"
    _description = (
        "Prototype to create a METIS master flat."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
           name=f"{_name}.stacking.method",
           context=_name,
           description="Name of the method used to combine the input images",
           default="average",
           alternatives=("add", "average", "median"),
        ),
    ])
    implementation_class = MetisNImgFlatImpl
