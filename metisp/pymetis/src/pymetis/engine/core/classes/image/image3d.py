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
from typing import Optional, Self

import hdrl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Type as CplType,
                      PropertyList as CplPropertyList,
                      Mask as CplMask,
                      Msg)
from hdrl.core import (Image as HdrlImage,
                       ImageList as HdrlImageList,)

from .base import EnhancedImageBase
from ..dataquality import DataQuality


class EnhancedImage3D(EnhancedImageBase):
    """
    A 3D enhanced image: a `HdrlImageList` (a stack of data + error planes)
    paired with a *single* 2D `Mask` describing the whole stack.

    The stack is the right home for genuine lists of frames as well as things
    like per-pixel coefficient tables (e.g. a linearity polynomial stack).
    """

    def __init__(
            self,
            images: CplImageList,
            errors: Optional[CplImageList] = None,
            dq: Optional[CplImage | CplMask | DataQuality] = None,
            *,
            prefix: str,
            header_image: Optional[CplPropertyList] = None,
            header_error: Optional[CplPropertyList] = None,
            header_dq: Optional[CplPropertyList] = None,
    ):
        if not isinstance(images, CplImageList):
            raise ValueError(f"Unsupported image type {type(images)}; expected a CplImageList")
        if len(images) == 0:
            raise ValueError("Cannot build an EnhancedImage3D from an empty ImageList")

        dim = self._dimensions(images)
        if errors is None:
            # HdrlImageList (unlike HdrlImage) will not accept a None error, so
            # a matching zero-filled error stack is always synthesised.
            errors = self._zeros_like(images)
        else:
            if (dims := self._dimensions(errors)) != dim:
                raise hdrl.core.IncompatibleInputError(
                    f"{self.__class__.__name__} '{prefix}': error layer dimensions {dims} "
                    f"do not match the image dimensions {dim}"
                )
            if len(errors) != len(images):
                raise hdrl.core.IncompatibleInputError(
                    f"{self.__class__.__name__} '{prefix}': error stack depth {len(errors)} "
                    f"does not match the image stack depth {len(images)}"
                )

        self.image = HdrlImageList(images, errors)
        self._finalize(prefix=prefix, dim=dim, dq=dq,
                       header_image=header_image, header_error=header_error, header_dq=header_dq)

    @classmethod
    def from_hdrl(
            cls,
            images: HdrlImageList,
            dq: Optional[CplImage | CplMask | DataQuality] = None,
            *,
            prefix: str,
            header_image: Optional[CplPropertyList] = None,
            header_error: Optional[CplPropertyList] = None,
            header_dq: Optional[CplPropertyList] = None,
    ) -> Self:
        """Pseudo-constructor: create directly from a HDRL image list + mask.

        `HdrlImageList` exposes no data/error accessor, so the CPL stacks are
        rebuilt by iterating its (live) `HdrlImage` planes."""
        data = CplImageList([plane.image for plane in images])
        errors = CplImageList([plane.error for plane in images])
        return cls(data, errors, dq, prefix=prefix,
                   header_image=header_image, header_error=header_error, header_dq=header_dq)

    def _sci_data(self) -> CplImageList:
        return CplImageList([plane.image for plane in self.image])

    def _err_data(self) -> CplImageList:
        return CplImageList([plane.error for plane in self.image])

    def _hdrl_planes(self) -> list[HdrlImage]:
        return list(self.image)

    @classmethod
    def _layer_type(cls) -> type:
        return CplImageList
