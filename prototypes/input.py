from abc import ABCMeta, abstractmethod

import cpl
from cpl.core import Msg


class PipelineInput(metaclass=ABCMeta):
    """
        The `PipelineInput` class is a singleton subclass for a recipe.
        It reads and filters the input FrameSet,
        categorizes the frames by their metadata,
        and stores them in its own attributes.
        It also provides verification mechanisms and methods
        for extraction of additional information from the frames.

        Every `RecipeImpl` should have exactly one `Input` class (possibly shared by more recipes though).
        Currently, we define them as internal classes of the corresponding `RecipeImpl`,
        but that might change if such a need arises.
    """

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        """ Filter the input frameset, capture frames that match criteria and assign them to own attributes. """

        for frame in frameset:
            self.categorize_frame(frame)

    @abstractmethod
    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        """
            Inspect a single frame and assign it to the proper attribute of the class.
            Every child class should try to recognize its own tags first and defer
            resolving the unknown ones to the parent class with something like
            ```
                match frame.tag:
                    case FOO: do_this()
                    case BAR: do_that()
                    case _: super().categorize_frame()
            ```

            Hence, this method's implementation here only provides the final resolution of unknown tags
            (emit a warning) and should **always** be called as a last resort.
        """
        # If we got all the way up here, no one recognized this frame, warn!
        Msg.warning(self.__class__.__qualname__,
                    f"Got a frame '{frame.file!r}' with unexpected tag {frame.tag!r}, ignoring.")

    @abstractmethod
    def verify(self) -> None:
        """
            Verify that the loaded frameset is valid and conforms to the specification.
            It would be also good to do this with some `schema` (but that might make Lars unhappy).
            Returns None if OK, otherwise an exception is raised.
            Optionally also extract additional information, such as detector names.

            Should raise an exception if anything goes wrong during initialization, otherwise returns None.

            Real world recipes should rather print a message (also to have it in the log file)
            and exit gracefully, but this should be handled upstream in the recipe
            or maybe in the `run` method.

            By default, nothing is done (there is no sensible default behaviour and
            every child class should implement its own checks).

            Returns
            -------
            None
        """

    @staticmethod
    def _verify_frame_present(frame: cpl.ui.Frame, title: str) -> None:
        """
        Verification shorthand: if a required frame is not present, i.e. `None`,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if frame is None:
            raise cpl.core.DataNotFoundError(f"No {title} found in the frameset.")

    @staticmethod
    def _verify_frameset_not_empty(frameset: cpl.ui.FrameSet, title: str) -> None:
        """
        Verification shorthand: if a required frameset is not present or empty,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if frameset is None or len(frameset) == 0:
            raise cpl.core.DataNotFoundError(f"No {title} found in the frameset.")
