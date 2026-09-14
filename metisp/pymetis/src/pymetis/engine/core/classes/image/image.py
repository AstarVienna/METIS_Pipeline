"""
This file is part of an A* Pipeline.
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

from pymetis.engine.core.classes.dataquality import DataQuality

from .base import EnhancedImageBase


class EnhancedImage(EnhancedImageBase):
    """
    A single 2D enhanced image: a `HdrlImage` (data + error) plus a 2D `Mask`.

    Note that this is not to replace an ImageList! For a genuine stack of frames
    use :class:`EnhancedImage3D`; for per-pixel coefficient tables and such,
    that stack is likewise the right home.
    """

    def __init__(
            self,
            image: CplImage,
            error: Optional[CplImage] = None,
            dq: Optional[CplImage | CplMask | DataQuality] = None,
            *,
            prefix: str,
            header_image: Optional[CplPropertyList] = None,
            header_error: Optional[CplPropertyList] = None,
            header_dq: Optional[CplPropertyList] = None,
    ):
        """
        Parameters
        ---

        image: CplImage
            The science image data in the form of a 2D matrix
        error: CplImage
            The computed or estimated standard deviation of the value in the `image`.
            If not provided, set to zero.
        dq:
            The data quality layer for the image.
            If not provided, set to "all good".

        prefix: str
            Prefix to be used in the HDUs when saved to a FITS file.
        header_image: CplPropertyList
            Header for the science data part
        header_error: CplPropertyList
            Header for the error data part
        header_dq: CplPropertyList
            Header for the data quality layer
        """
        if not isinstance(image, CplImage):
            raise ValueError(f"Unsupported image type {type(image)}; expected a CplImage")

        dim = self._dimensions(image)
        if error is None:
            error = self._zeros_like(image)
        elif (dims := self._dimensions(error)) != dim:
            raise hdrl.core.IncompatibleInputError(
                f"{self.__class__.__name__} '{prefix}': error layer dimensions {dims} "
                f"do not match the image dimensions {dim}"
            )

        self.image = HdrlImage(image, error)
        self._finalize(prefix=prefix, dim=dim, dq=dq,
                       header_image=header_image, header_error=header_error, header_dq=header_dq)

    @classmethod
    def from_hdrl(
            cls,
            image: HdrlImage,
            dq: Optional[CplImage | CplMask | DataQuality] = None,
            *,
            prefix: str,
            header_image: Optional[CplPropertyList] = None,
            header_error: Optional[CplPropertyList] = None,
            header_dq: Optional[CplPropertyList] = None,
    ) -> Self:
        """Pseudo-constructor: create directly from a HDRL image + mask."""
        return cls(image.image, image.error, dq, prefix=prefix,
                   header_image=header_image, header_error=header_error, header_dq=header_dq)

    def _sci_data(self) -> CplImage:
        return self.image.image

    def _err_data(self) -> CplImage:
        return self.image.error

    def _hdrl_planes(self) -> list[HdrlImage]:
        return [self.image]

    @classmethod
    def _layer_type(cls) -> type:
        return CplImage


