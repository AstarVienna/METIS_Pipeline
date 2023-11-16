from typing import Any, Dict

# Import the required PyCPL modules
import cpl.core
import cpl.ui


class HelloWorld(cpl.ui.PyRecipe):
    _name = "hellouser"
    _version = "1.0"
    _author = "U.N. Owen"
    _email = "unowen@somewhere.net"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "PyCPL version of 'hello, world!' using parameters"
    _description = (
        "This is the PyCPL greets the user and lists the first N input files."
    )

    # The username and the number of input files listed should be configurable
    # by the user when executing this recipe. To make this possible, one needs
    # to define the corresponding parameters. There are 3 kinds of parameters
    # defined in CPL and PyCPL. Simple values, ranges and enumeration. The
    # following illustrates each of them.
    #
    # Parameters must have a unique name, are related to a context (typically the
    # leftmost part of the full parameter name), a description and a default value.
    # Ranges and enumerations have also additional attributes, which specify
    # the range boundaries or the enumeration alternatives.
    #
    # The defined parameters have to be stored in a parameter list object.
    def __init__(self) -> None:
        super().__init__()

        self.parameters = cpl.ui.ParameterList(
            (
                cpl.ui.ParameterValue(
                    name="hellouser.username",
                    context="hellouser",
                    description="User's name",
                    default="Unknown user",
                ),
                cpl.ui.ParameterRange(
                    name="hellouser.nfiles",
                    context="hellouser",
                    description="Number of input files to process.",
                    default=3,
                    min=1,
                    max=5,
                ),
                cpl.ui.ParameterEnum(
                    name="hellouser.reaction",
                    context="hellouser",
                    description="Reaction to the user.",
                    default="nice",
                    alternatives=("nice", "nasty"),
                ),
            )
        )

    # Again the run method has to be implemented. This time, in order to print the
    # message that the user requested, we have to query the settings argument for
    # the actual values the user may have set, for instance on the pyesorex command
    # line. Contrary to the first example, the messages are now printed using the
    # standard messaging format for recipes.
    def run(
        self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]
    ) -> cpl.ui.FrameSet:
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                cpl.core.Msg.warning(
                    self.name,
                    f"Settings includes {key}:{value} but {self} has no parameter named {key}.",
                )

        cpl.core.Msg.info(
            self.name,
            f"Hello, {self.parameters['hellouser.username'].value}, "
            + f"{self.parameters['hellouser.reaction'].value} to see you.",
        )

        cpl.core.Msg.info(
            self.name,
            f"The first {self.parameters['hellouser.nfiles'].value} input files are:",
        )

        for i, frame in enumerate(frameset):
            if i >= self.parameters["hellouser.nfiles"].value:
                break
            cpl.core.Msg.info(self.name, frame.file)

        # Return an empty cpl.ui.FrameSet because there are no recipe products
        return cpl.ui.FrameSet()
