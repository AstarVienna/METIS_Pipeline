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

from abc import abstractmethod

import cpl

from cpl.core import Msg


class PipelineInput:
    _title: str = None                      # No universal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _tags: [str] = None                     # No universal tags are provided
    _group: str = None                      # No sensible default, must be provided explicitly

    @property
    def title(self):
        return self._title

    @property
    def tags(self):
        return self._tags

    @property
    def required(self):
        return self._required

    @property
    def group(self):
        return self._group

    def __init__(self,
                 *,
                 title: str = None,
                 tags: [str] = None,
                 required: bool = None,
                 **kwargs):
        # First override the title, if provided in the constructor
        if title is not None:
            self._title = title
            Msg.debug(self.__class__.__qualname__, f"Overriding `title` to {self.title}")

        # Check if it is defined
        if self.title is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        # First override the tags if provided in the ctor
        if tags is not None:
            self._tags = tags
            Msg.debug(self.__class__.__qualname__, f"Overriding `tags` to {self.tags}")

        # Now expand the tags with provided context from **kwargs
        try:
            self._tags = [tag.format(**kwargs) for tag in self._tags]
        except KeyError as e:
            Msg.error(self.__class__.__qualname__, f"Could not substitute tag placeholders: {e}")
            raise e

        # Check if tags are defined...
        if not self.tags:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tags")

        # ...and that they are a list of strings (not a single string -- this leads to nasty errors)
        if not isinstance(self.tags, list):
            raise TypeError(f"Tags must be a list of string templates, got '{self.tags}'")
        for tag in self.tags:
            if not isinstance(tag, str):
                raise TypeError(f"Tags must be a list of string templates, got '{type(tag)}'")

        # Override `required` if requested
        if required is not None:
            self._required = required
            Msg.debug(self.__class__.__qualname__, f"Overriding `required` to {self.required}")

        # Check is frame_group is defined (if not, this gives rise to strange errors deep within CPL)
        if not self.group:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined group!")

    @abstractmethod
    def verify(self) -> None:
        """
        Verify that the input has all the required frames and that they are valid themselves.
        There is no default logic, implementation is deferred to derived classes.
        """

    def print_debug(self, *, offset: int = 0) -> None:
        """
        Print a short description of the tags with a small offset (spaces).
        """
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}Tags: {self.tags}")


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 required: bool = None,
                 **kwargs):
        self.frame: cpl.ui.Frame | None = None
        super().__init__(tags=tags, required=required, **kwargs)

        for frame in frameset:
            if frame.tag in self.tags:
                if self.frame is None:
                    Msg.debug(self.__class__.__qualname__,
                              f"Found a {self.title} frame: {frame.file}.")
                else:
                    Msg.warning(self.__class__.__qualname__,
                                f"Found another {self.title} frame: {frame.file}! "
                                f"Discarding previously loaded {self.frame.file}.")
                self.frame = frame

    def verify(self):
        self._verify_frame_present(self.frame)

    def _verify_frame_present(self,
                              frame: cpl.ui.Frame) -> None:
        """
        Verification shorthand: if a required frame is not present, i.e. `None`,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        If it is not required, emit a warning but continue.
        """
        if frame is None:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} frame found in the frameset.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"No {self.title} frame found, but not required.")
        else:
            Msg.debug(self.__class__.__qualname__,
                      f"Found a {self.title} frame {frame.file}")


class MultiplePipelineInput(PipelineInput):
    """
    A pipeline input that expects multiple frames, such as raw processor.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 required: bool = None,
                 **kwargs):                     # Any other args
        self.frameset: cpl.ui.FrameSet | None = cpl.ui.FrameSet()
        super().__init__(tags=tags, required=required, **kwargs)

        for frame in frameset:
            if frame.tag in self.tags:
                frame.group = self.group
                self.frameset.append(frame)
                Msg.debug(self.__class__.__qualname__,
                          f"Found a {self.title} frame: {frame.file}.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Ignoring {frame.file}: tag {frame.tag} not in {self.tags}.")


    def verify(self):
        self._verify_frameset_not_empty()
        self._verify_same_detector()

    def _verify_frameset_not_empty(self) -> None:
        """
        Verification shorthand: if a required frameset is not present or empty,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if (count := len(self.frameset)) == 0:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} frames found in the frameset.")
            else:
                Msg.debug(f"No {self.title} frames found but not required.")
        else:
            Msg.debug(self.__class__.__qualname__, f"OK: {count} frames found")

    def _verify_same_detector(self) -> None:
        """
        Verify whether all the raw frames originate from the same detector.

        Raises
        ------
        KeyError
            If the found detector name is not a valid detector name
        ValueError
            If dark frames from more than one detector are found

        Returns
        -------
        None:
            None on success
        """
        detectors = []
        for frame in self.frameset:
            header = cpl.core.PropertyList.load(frame.file, 0)
            try:
                det = header['ESO DPR TECH'].value
                try:
                    detectors.append({
                                         'IMAGE,LM': '2RG',
                                         'IMAGE,N': 'GEO',
                                         'IFU': 'IFU',
                                     }[det])
                except KeyError as e:
                    raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e
            except KeyError as e:
                Msg.warning(self.__class__.__qualname__, f"No detector (ESO DPR TECH) set!")


        # Check if all the raws have the same detector, if not, we have a problem
        if len(unique := list(set(detectors))) == 1:
            self._detector_name = unique[0]
        elif len(unique) == 0:
            Msg.warning(self.__class__.__qualname__,
                        f"No detectors specified (this is probably fine in skeleton stage)")
        else:
            # raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")
            Msg.warning(self.__class__.__qualname__,
                        f"Darks from more than one detector found: {set(detectors)}!")
