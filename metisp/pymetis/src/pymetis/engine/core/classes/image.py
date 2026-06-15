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

import cpl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Type as CplType,
                      PropertyList as CplPropertyList,
                      Msg)

from pymetis.engine.dataitems import Hdu


class EnhancedImage:
    """
    A high-level image object that encapsulates a data layer, error layer and data quality layer.
    Should be independent of CPL and FITS quirks but support loading and saving.

    The three layers are stored as named FITS extensions
    ``<prefix>.SCI``, ``<prefix>.ERR`` and ``<prefix>.DQ``.
    The science and error layers carry floating-point pixels;
    the data quality layer is a bad-pixel mask and is
    expected to hold an integer type, where a non-zero value flags a bad pixel.
    """

    # Sensible defaults for suffixes. Might be overridden in a derived class.
    sci_suffix: ClassVar[str] = 'SCI'
    err_suffix: ClassVar[str] = 'ERR'
    dq_suffix: ClassVar[str] = 'DQ'

    def __init__(self,
                 image: CplImage | CplImageList,
                 error: Optional[CplImage | CplImageList] = None,
                 dq: Optional[CplImage | CplImageList] = None,
                 *,
                 prefix: str,
                 header_image: Optional[CplPropertyList] = None,
                 header_error: Optional[CplPropertyList] = None,
                 header_dq: Optional[CplPropertyList] = None):
        self.prefix = prefix

        # The error and data quality layers must describe the same pixels as
        # the science image, so their spatial dimensions have to match.
        # The number of planes (for an ImageList) may legitimately differ, e.g. a
        # stack of polynomial coefficients paired with a single 2D bad-pixel map.
        ref = self._dimensions(image)
        for name, layer in (('error', error), ('dq', dq)):
            if layer is not None and (dims := self._dimensions(layer)) != ref:
                raise ValueError(
                    f"EnhancedImage '{prefix}': {name} layer dimensions {dims} "
                    f"do not match the image dimensions {ref}"
                )

        self.image: Hdu = Hdu(
            header_image if header_image else CplPropertyList(),
            image,
            name=rf'{self.prefix}.{self.sci_suffix}'
        )
        self.error: Optional[Hdu] = Hdu(
            header_error if header_error else CplPropertyList(),
            error,
            name=rf'{self.prefix}.{self.err_suffix}'
        ) if error is not None else None
        self.dq: Optional[Hdu] = Hdu(
            header_dq if header_dq else CplPropertyList(),
            dq,
            name=rf'{self.prefix}.{self.dq_suffix}'
        ) if dq is not None else None

    @staticmethod
    def _dimensions(layer: CplImage | CplImageList) -> tuple[int, int]:
        """
        Return the (width, height) of a layer, which may be a single `Image`
        or an `ImageList` of equally-sized planes.
        """
        if isinstance(layer, CplImageList):
            if len(layer) == 0:
                raise ValueError("Cannot determine dimensions of an empty ImageList")
            layer = layer[0]
        return layer.width, layer.height

    def as_list(self) -> list[Hdu]:
        """Return the present HDUs, skipping any absent layers."""
        return [hdu for hdu in [self.image, self.error, self.dq] if hdu is not None]

    @staticmethod
    def _describe_layer(hdu: Hdu) -> str:
        """Describe a layer's name and type (with depth for ImageLists),
        e.g. 'DET1.SCI: ImageList[3]' or 'DET1.DQ: Image'. The dimensions are
        shared across layers and reported once by `__repr__`."""
        kind = (f'ImageList[{len(hdu.data)}]' if isinstance(hdu.data, CplImageList) else 'Image')
        return f'{hdu.name}: {kind}'

    def __repr__(self) -> str:
        width, height = self._dimensions(self.image.data)
        layers = ', '.join(self._describe_layer(hdu) for hdu in self.as_list())
        return f"<EnhancedImage {self.prefix!r} {width}×{height}: {layers}>"

    def save(self, filename: str) -> None:
        """
        Append all present layers as extensions to ``filename``.

        The file is expected to already exist with a primary HDU (this mirrors
        `DataItem.save_extensions`, which creates the primary header first and
        then appends each `Hdu`). Absent layers are simply skipped.
        """
        for hdu in self.as_list():
            hdu.save(filename)

    @classmethod
    def load(cls, filename: str, prefix: str) -> Self:
        """
        Reconstruct an `EnhancedImage` from the ``<prefix>.SCI/.ERR/.DQ``
        extensions of a FITS file. The science layer must be present; the
        error and data quality layers are optional.

        Each layer is read as an `Image` (NAXIS == 2) or an `ImageList`
        (NAXIS == 3), matching how `DataItem.load` infers the HDU class.
        """
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
            pixel_type: CplType = CplType.FLOAT
        ) -> tuple[Optional[CplImage | CplImageList], Optional[CplPropertyList]]:
            extname = f'{prefix}.{suffix}'
            if extname not in extensions:
                return None, None

            extno, header = extensions[extname]
            # FITS stores a 2D plane as NAXIS == 2 and a stack as NAXIS == 3.
            klass = CplImageList if header['NAXIS'].value == 3 else CplImage
            data = klass.load(filename, pixel_type, extno)
            return data, header

        image, header_image = read_layer(cls.sci_suffix)
        if image is None:
            raise cpl.core.DataNotFoundError(
                f"No '{prefix}.{cls.sci_suffix}' extension found in {filename}"
            )
        error, header_error = read_layer(cls.err_suffix)
        # The data quality layer is a mask, so load it as an integer to preserve
        # its semantics rather than coercing the flags to floats.
        dq, header_dq = read_layer(cls.dq_suffix, CplType.INT)

        return cls(image, error, dq,
                   prefix=prefix,
                   header_image=header_image,
                   header_error=header_error,
                   header_dq=header_dq)