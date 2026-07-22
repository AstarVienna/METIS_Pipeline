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


from typing import Optional, ClassVar, Self

import numpy as np
import cpl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Type as CplType,
                      PropertyList as CplPropertyList,
                      Mask as CplMask,
                      Msg)
from cpl.hdrl.core import (Image as HdrlImage,
                           ImageList as HdrlImageList,)

from pymetis.engine.core.classes.mask import Mask
from pymetis.engine.dataitems import Hdu


class EnhancedImage:
    """
    A high-level image object that encapsulates a data layer, error layer and data quality layer.
    Should be independent of CPL and FITS quirks but still support all functions and IO.

    The three layers are stored as named FITS extensions
    ``<prefix>.SCI``, ``<prefix>.ERR`` and ``<prefix>.DQ``.
    The science and error layers carry floating-point pixels;
    the data quality layer is a 31-bit bad-pixel mask and is
    expected to hold an integer type, where a non-zero value flags a bad pixel.
    Each bit represents a different reason (as defined inside the instrument description).

    Note that this is not to replace an ImageList! The ImageList is supported only to accommodate
    things like per-pixel coefficient tables and such. For true lists of images,
    use `list[EnhancedImage]` (or ask for adding support).
    """

    # Defaults for suffixes. Might be overridden in a derived class if such need arises.
    sci_suffix: ClassVar[str] = 'SCI'
    err_suffix: ClassVar[str] = 'ERR'
    dq_suffix: ClassVar[str] = 'DQ'

    def __init__(self,
                 image: CplImage,
                 error: Optional[CplImage] = None,
                 dq: Optional[CplImage | CplMask | Mask] = None,
                 *,
                 prefix: str,
                 header_image: Optional[CplPropertyList] = None,
                 header_error: Optional[CplPropertyList] = None,
                 header_dq: Optional[CplPropertyList] = None,
    ):
        self.prefix = prefix

        # The error and data quality layers must describe the same pixels as
        # the science image, so their spatial dimensions have to match.
        dim = self._dimensions(image)
        for name, layer in (('error', error), ('dq', dq)):
            if layer is not None and (dims := self._dimensions(layer)) != dim:
                raise cpl.hdrl.core.IncompatibleInputError(
                    f"{self.__class__} '{prefix}': {name} layer dimensions {dims} "
                    f"do not match the image dimensions {dim}"
                )

        if isinstance(image, CplImage):
            self.image = HdrlImage(image, error)
        else:
            raise ValueError(f"Unsupported image type {type(image)}")

        # `Mask` acts as a converting constructor (CplImage/CplMask/Mask) and
        # always copies, so the caller never shares a mutable mask with us; a
        # missing dq just supplies a zero bitfield to wrap.
        self.dq = Mask(dq if dq is not None else self._zero_dq(*dim))

        # Finally set headers, if provided
        self.header_image = CplPropertyList() if header_image is None else header_image
        self.header_error = CplPropertyList() if header_error is None else header_error
        self.header_dq = CplPropertyList() if header_dq is None else header_dq

    @classmethod
    def from_hdrl(
            cls,
            image: HdrlImage,
            dq: Optional[CplImage | CplMask | Mask] = None,
            *,
            prefix: str,
            header_image: Optional[CplPropertyList] = None,
            header_error: Optional[CplPropertyList] = None,
            header_dq: Optional[CplPropertyList] = None,
    ) -> Self:
        """
        Pseudo-constructor: create directly from a HDRL image + mask.
        """
        return cls(image.image, image.error, dq, prefix=prefix,
                   header_image=header_image, header_error=header_error, header_dq=header_dq)

    def __repr__(self) -> str:
        # SCI/ERR live inside the HDRL image; DQ is the mask's backing image.
        layers = (
            (self.sci_suffix, self.image.image),
            (self.err_suffix, self.image.error),
            (self.dq_suffix, self.dq.data),
        )
        width, height = self._dimensions(self.image.image)
        described = ', '.join(
            self._describe_layer(f'{self.prefix}.{suffix}', data)
            for suffix, data in layers
        )
        return f"<EnhancedImage {self.prefix!r} {width}×{height}: {described}>"

    @classmethod
    def schema(cls, prefix) -> dict[str, type]:
        """
        Build a generic schema for EnhancedImage, depending on the prefix.
        """
        return {
            f'{prefix}.{cls.sci_suffix}': CplImage,
            f'{prefix}.{cls.err_suffix}': CplImage,
            f'{prefix}.{cls.dq_suffix}': CplImage,
        }

    def get_schema(self) -> dict[str, type]:
        """ Schema for instances """
        return self.schema(self.prefix)

    def save(self,
             filename: str) -> None:
        """
        Save this image to a FITS file.
        """
        Msg.info(self.__class__.__qualname__,
                 f"Saving an EnhancedImage {self.prefix} to file '{filename}'")

        image: Hdu = Hdu(
            self.header_image,
            self.image.image,
            name=rf'{self.prefix}.{self.sci_suffix}'
        )
        image.save(filename)

        error: Hdu = Hdu(
            self.header_error,
            self.image.error,
            name=rf'{self.prefix}.{self.err_suffix}'
        )
        error.save(filename)

        dq = Hdu(
            self.header_dq,
            self.dq.data,
            name=rf'{self.prefix}.{self.dq_suffix}'
        )
        dq.save(filename)

    @staticmethod
    def _dimensions(layer: CplImage):
        """
        Return the (width, height) of a layer, which may be a single `Image`
        or an `ImageList` of equally-sized planes.
        """
        if isinstance(layer, CplImageList):
            if len(layer) == 0:
                raise ValueError("Cannot determine dimensions of an empty ImageList")
            layer = layer[0]
        return layer.width, layer.height

    @staticmethod
    def _zeros_like(layer: CplImage | CplImageList) -> CplImage | CplImageList:
        """
        A zero-filled float layer matching the shape (and plane count) of
        `layer`. Used to synthesise an absent error layer on load.
        """
        width, height = EnhancedImage._dimensions(layer)

        def plane() -> CplImage:
            return CplImage(np.zeros((height, width), dtype=np.float64))

        if isinstance(layer, CplImageList):
            return CplImageList([plane() for _ in range(len(layer))])
        return plane()

    @staticmethod
    def _zero_dq(width: int, height: int) -> CplImage:
        """An all-good (all-zero) integer data quality image of the given size."""
        return CplImage(np.zeros((height, width), dtype=np.int32), dtype=CplType.INT)

    @staticmethod
    def _describe_layer(name: str, data: CplImage | CplImageList) -> str:
        """Describe a layer's name and type (with depth for ImageLists),
        e.g. 'DET1.SCI: ImageList[3]' or 'DET1.DQ: Image'. The dimensions are
        shared across layers and reported once by `__repr__`."""
        kind = (f'ImageList[{len(data)}]' if isinstance(data, CplImageList) else 'Image')
        return f'{name}: {kind}'

    @classmethod
    def load(cls, filename: str, prefix: str) -> Self:
        """
        Reconstruct an `EnhancedImage` from the ``<prefix>.SCI``, ``<prefix>.ERR``
        and ``<prefix>.DQ`` extensions of ``filename``, mirroring :meth:`save`.

        The science layer must be present. The error and data quality layers
        are optional; when their extension is absent they are initialised to
        zeros of the matching shape -- a float error image and an all-good
        (all-zero) integer data quality mask.

        Each layer is read as an `Image` (NAXIS == 2) or an `ImageList`
        (NAXIS == 3), matching how `DataItem.load` infers the HDU class.
        """
        Msg.info(cls.__qualname__,
                 f"Loading an EnhancedImage {prefix} from '{filename}'")

        # Map every extension name to its index and header.
        extensions: dict[str, tuple[int, CplPropertyList]] = {}
        index = 0

        while True:
            try:
                header = CplPropertyList.load(filename, index)
            except cpl.core.DataNotFoundError:
                break
            if index > 0:
                try:
                    extensions[header['EXTNAME'].value] = (index, header)
                except KeyError:
                    Msg.debug(cls.__qualname__,
                              f"HDU {index} in {filename} has no EXTNAME, skipping")
            index += 1

        def read_layer(
            suffix: str,
            pixel_type: CplType = CplType.FLOAT,
        ) -> tuple[Optional[CplImage | CplImageList], Optional[CplPropertyList]]:
            extname = f'{prefix}.{suffix}'
            if extname not in extensions:
                return None, None

            extno, header = extensions[extname]
            # FITS stores a 2D plane as NAXIS == 2 and a stack as NAXIS == 3.
            klass = CplImageList if header['NAXIS'].value == 3 else CplImage
            return klass.load(filename, pixel_type, extno), header

        image, header_image = read_layer(cls.sci_suffix)
        if image is None:
            raise cpl.core.DataNotFoundError(
                f"No '{prefix}.{cls.sci_suffix}' extension found in {filename}"
            )

        # `__init__` requires an error layer matching the image type and wraps
        # `dq` in a `Mask`, so synthesise zeros here when either extension is
        # missing rather than passing `None` through.
        error, header_error = read_layer(cls.err_suffix)
        if error is None:
            Msg.info(cls.__qualname__,
                     f"No error layer found in {filename}")
            error = cls._zeros_like(image)

        # The data quality layer is a mask, so read it as an integer to preserve
        # the flag bits rather than coercing them to floats. A missing extension
        # is left as None; `__init__` then supplies an all-good mask.
        dq, header_dq = read_layer(cls.dq_suffix, CplType.INT)

        return cls(image, error, dq,
                   prefix=prefix,
                   header_image=header_image,
                   header_error=header_error,
                   header_dq=header_dq)
