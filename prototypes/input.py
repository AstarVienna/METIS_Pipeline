from abc import ABCMeta, abstractmethod

import cpl
from cpl.core import Msg


class PipelineInput(metaclass=ABCMeta):
    """
        The Input class is a singleton subclass for a recipe.
        It reads and filters the input FrameSet, categorizes the frames by their metadata
        and stores them in its own attributes.
        Also provides verification mechanisms and methods for extraction of additional information from the frames.
    """

    def __init__(self, frameset: cpl.ui.FrameSet):
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

            Hence, this method only provides the final resolution of unknown tags (emit a warning)
            and should be always called as a last resort.
        """
        # If we got all the way up here, no one recognizes this frame, warn!
        Msg.warning(self.__class__.__name__,
                    f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

    @abstractmethod
    def verify(self) -> None:
        """
            Verify that the loaded frameset is valid and conforms to the specification.
            It would be also good to do this with some schema, ideally `schema` (but that might make Lars unhappy).
            Returns None if OK, otherwise an exception is raised.
            Optionally also extract additional information, such as detector names.

            Real world recipes should rather print a message (also to have it in the log file)
            and exit gracefully, but this should be handled upstream in the recipe
            or maybe in the `run` method.
        """

