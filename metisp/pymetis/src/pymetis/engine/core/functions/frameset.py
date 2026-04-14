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
import os.path

import cpl


def preprocess_frameset(frameset: cpl.ui.FrameSet) -> dict[str, cpl.ui.FrameSet]:
    """
    Convert a FrameSet (which is essentially a `list[tuple[filename, tag]]`) to a mapping `{tag: FrameSet}`
    to make it more convenient for processing in Python.
    """
    result = {}

    for frame in frameset:
        if frame.tag in result:
            result[frame.tag] += [frame]
        else:
            result[frame.tag] = [frame]

    return {
        tag: cpl.ui.FrameSet(frames)
        for tag, frames in result.items()
    }


def represent_frameset(frameset: cpl.ui.FrameSet) -> dict[str, str]:
    """
    Convert a SOF (which is a `list[tuple[filename, tag]]`) to a mapping `filename: tag`
    """
    return {
        frame.filename: frame.tag
        for frame in frameset
    }


def from_filenames(frames: dict[str, str]) -> cpl.ui.FrameSet:
    """
    Create a CPL FrameSet from a mapping `{filename: tag}`
    """
    return cpl.ui.FrameSet([
        cpl.ui.Frame(os.path.expandvars(frame),
                     group=cpl.ui.Frame.FrameGroup.RAW,
                     level=cpl.ui.Frame.FrameLevel.NONE,
                     tag=tag)
        for frame, tag in frames.items()
    ])


def from_tags(**tagged: dict[str, list[str]]) -> cpl.ui.FrameSet:
    """
    Create a CPL FrameSet from kwargs `{tag: list[filename]}`
    """
    return cpl.ui.FrameSet([
        cpl.ui.Frame(os.path.expandvars(frame),
                     group=cpl.ui.Frame.FrameGroup.RAW,
                     level=cpl.ui.Frame.FrameLevel.NONE,
                     tag=tag)
        for tag, frames in tagged.items()
        for frame in frames
    ])