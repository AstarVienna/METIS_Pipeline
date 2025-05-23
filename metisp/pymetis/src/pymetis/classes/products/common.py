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
from abc import ABC

import cpl

from pymetis.classes.products.image import PipelineImageProduct
from pymetis.classes.products.product import PipelineProduct


class TargetSpecificProduct(PipelineProduct, ABC):
    """Products specific to a target. Usually, but not necessarily, (SCI|STD|SKY) and (LAMP|TWILIGHT)"""
    _target: str = None

    @classmethod
    def target(cls) -> str:
        """Return the internal target, or a placeholder for manpage."""
        return cls._target or "{target}"

    @classmethod
    def verbose(cls) -> str:
        return {
            'SCI': 'science',
            'STD': 'standard',
        }.get(cls.target(), '{target}')


class DetectorSpecificProduct(PipelineProduct, ABC):
    """Products specific to a detector. Usually, but not necessarily, (2RG|GEO|IFU)"""
    _detector: str = None

    @classmethod
    def detector(cls) -> str:
        """Return the internal detector, or a placeholder for manpage."""
        return cls._detector or "{detector}"


class BandSpecificProduct(PipelineProduct, ABC):
    """Products specific to a band. Usually, but not necessarily, (LM|N)"""
    _band: str = None

    @classmethod
    def band(cls) -> str:
        """Return the internal band, or a placeholder for manpage."""
        return cls._band or "{band}"


# FixMe move this to some prefab
class ProductBadpixMapDet(DetectorSpecificProduct, PipelineImageProduct):
    group = cpl.ui.Frame.FrameGroup.CALIB  # ToDo TBC
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE

    _oca_keywords = {'PRO.CATG'}

    @classmethod
    def description(cls) -> str:
        return rf"Bad pixel map for {cls.detector()}. Also contains detector masks."

    @classmethod
    def tag(cls):
        return rf"BADPIX_MAP_{cls.detector()}"

        # SKEL: copy product keywords from header
    def add_properties(self):
        super().add_properties()
        self.properties.append(self.header)
