import cpl

from .. import MetisRecipe
from impl.img.metis_lm_basic_reduce import MetisLmBasicReduceImpl


class MetisLmBasicReduce(MetisRecipe):
    """
    Apart from our own recipe implementation we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the

    - seven required attributes as below (copyright may be omitted as it is provided in the base class),
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally define the implementation class, which we have just written
    """
    # Fill in recipe information
    _name = "metis_lm_basic_reduce"
    _version = "0.1"
    _author = "Chi-Hung Yan"
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
            name="basic_reduction.stacking.method",
            context="basic_reduction",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])
    implementation_class = MetisLmBasicReduceImpl
