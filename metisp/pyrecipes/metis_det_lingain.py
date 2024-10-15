import cpl

from . import MetisRecipe
from impl.metis_det_lingain import MetisDetLinGainImpl


class MetisDetLinGain(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_lingain"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Martin"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Measure detector non-linearity and gain"
    _description = (
        "Prototype to create a METIS linear gain map."
    )


    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_det_lingain.stacking.method",
            context="metis_det_lingain",
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("add", "average", "median"),
        ),
        cpl.ui.ParameterValue(
            name="metis_det_lingain.threshold.lowlim",
            context=_name,
            description="Thresholding threshold lower limit",
            default=0,
        ),
        cpl.ui.ParameterValue(
            name="metis_det_lingain.threshold.uplim",
            context=_name,
            description="Thresholding threshold upper limit",
            default=0,
        ),
    ])

    implementation_class = MetisDetLinGainImpl
