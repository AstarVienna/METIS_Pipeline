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
import hdrl.core
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Type as CplType,
                      PropertyList as CplPropertyList,
                      Mask as CplMask,
                      Msg)
from hdrl.core import (Image as HdrlImage,
                       ImageList as HdrlImageList,)

from pymetis.engine.core.classes.instrument import InstrumentDescription
from pymetis.engine.core.classes.dataquality import DataQuality
from pymetis.engine.dataitems import Hdu


class EnhancedImageBase:
    """
    Common machinery for :class:`EnhancedImage` (a single 2D frame) and
    :class:`EnhancedImage3D` (a 3D stack).

    Both encapsulate a science (data) layer, an error layer and a single
    data quality layer, stored as named FITS extensions ``<prefix>.SCI``,
    ``<prefix>.ERR`` and ``<prefix>.DQ``. The science and error layers carry
    floating-point pixels; the data quality layer is a single 2D 31-bit
    bad-pixel mask (a `DataQuality`), where a non-zero value flags a bad pixel
    and each bit records a different reason (see the instrument description).

    The data quality layer is the *only* source of bad-pixel information: it is
    what gets saved, and what every operation starts from. The 1-bit bad-pixel
    masks living inside the HDRL planes are a scratch pad for HDRL, which
    understands nothing richer: before an operation, :meth:`reject` compresses
    the selected DQ bits down to 1 bit and *replaces* the planes' masks with the
    result; the operation then runs on HDRL's terms; afterwards the recipe may
    read the resulting mask back through :meth:`rejected` and record it under a
    flag bit of its choice (``ei.dq.add(ei.rejected(), MaskFlags.X)``) -- or
    deliberately drop it. The scratch masks themselves are never saved.

    The *only* thing that differs between the two is the science/error
    container: a `HdrlImage` (2D) versus a `HdrlImageList` (a stack of planes).
    The data quality mask stays 2D in both cases -- one bad-pixel map describes
    the whole stack. Subclasses supply the container-specific behaviour through
    :meth:`_sci_data`, :meth:`_err_data`, :meth:`_science_images` and
    :meth:`_layer_type`; everything else lives here.
    """

    # Defaults for suffixes. Might be overridden in a derived class if such need arises.
    sci_suffix: ClassVar[str] = 'SCI'
    err_suffix: ClassVar[str] = 'ERR'
    dq_suffix: ClassVar[str] = 'DQ'

    # ---- container-specific hooks (implemented by subclasses) ----

    def _sci_data(self) -> CplImage | CplImageList:
        """The science layer as a CPL object, ready to write to a FITS HDU."""
        raise NotImplementedError

    def _err_data(self) -> CplImage | CplImageList:
        """The error layer as a CPL object, ready to write to a FITS HDU."""
        raise NotImplementedError

    def _hdrl_planes(self) -> list[HdrlImage]:
        """The HDRL science planes whose bad-pixel mask can be rejected into.

        Rejection must go through the `HdrlImage` object itself: its ``.image``
        accessor returns a *fresh copy* each call, so mutating that would be a no-op.
        """
        raise NotImplementedError

    @classmethod
    def _layer_type(cls) -> type:
        """The CPL type of the science/error layers (`CplImage` or `CplImageList`)."""
        raise NotImplementedError

    # ---- shared construction tail ----

    def _finalize(
            self,
            *,
            prefix: str,
            dim: tuple[int, int],
            dq: Optional[CplImage | CplMask | DataQuality],
            header_image: Optional[CplPropertyList],
            header_error: Optional[CplPropertyList],
            header_dq: Optional[CplPropertyList],
    ) -> None:
        """Validate and wrap the data quality layer and set the headers, once
        the subclass has built ``self.image`` and knows the spatial ``dim``."""
        self.prefix = prefix

        # The data quality layer must describe the same pixels as the science image.
        if dq is not None and (dims := self._dimensions(dq)) != dim:
            raise hdrl.core.IncompatibleInputError(
                f"{self.__class__.__name__} '{prefix}': dq layer dimensions {dims} "
                f"do not match the image dimensions {dim}"
            )

        # `Mask` acts as a converting constructor (CplImage/CplMask/Mask) and
        # always copies, so the caller never shares a mutable mask with us; a
        # missing dq just supplies a zero bitfield to wrap.
        self.dq = DataQuality(dq if dq is not None else self._zero_dq(*dim))

        self.header_image = CplPropertyList() if header_image is None else header_image
        self.header_error = CplPropertyList() if header_error is None else header_error
        self.header_dq = CplPropertyList() if header_dq is None else header_dq

    # ---- representation ----

    def __repr__(self) -> str:
        layers = (
            (self.sci_suffix, self._sci_data()),
            (self.err_suffix, self._err_data()),
            (self.dq_suffix, self.dq.data),
        )
        width, height = self._dimensions(self._sci_data())
        described = ', '.join(
            self._describe_layer(f'{self.prefix}.{suffix}', data)
            for suffix, data in layers
        )
        return f"<{self.__class__.__name__} {self.prefix!r} {width}×{height}: {described}>"

    # ---- schema ----

    @classmethod
    def schema(cls, prefix) -> dict[str, type]:
        """Build a generic schema for the class, depending on the prefix."""
        return {
            f'{prefix}.{cls.sci_suffix}': cls._layer_type(),
            f'{prefix}.{cls.err_suffix}': cls._layer_type(),
            f'{prefix}.{cls.dq_suffix}': CplImage,   # the DQ layer is always a single 2D image
        }

    def get_schema(self) -> dict[str, type]:
        """ Schema for instances """
        return self.schema(self.prefix)

    # ---- IO ----

    def hdus(self) -> list[Hdu]:
        """Return the SCI/ERR/DQ layers as named HDUs, ready to save."""
        return [
            Hdu(self.header_image, self._sci_data(), name=f'{self.prefix}.{self.sci_suffix}'),
            Hdu(self.header_error, self._err_data(), name=f'{self.prefix}.{self.err_suffix}'),
            Hdu(self.header_dq, self.dq.data, name=f'{self.prefix}.{self.dq_suffix}'),
        ]

    def save(self, filename: str) -> None:
        """Save this image to a FITS file."""
        Msg.info(self.__class__.__qualname__,
                 f"Saving an {self.__class__.__name__} {self.prefix} to file '{filename}'")
        for hdu in self.hdus():
            hdu.save(filename)

    @classmethod
    def load(cls, filename: str, prefix: str) -> "EnhancedImageBase":
        """
        Reconstruct an `EnhancedImage` or `EnhancedImage3D` from the
        ``<prefix>.SCI``, ``<prefix>.ERR`` and ``<prefix>.DQ`` extensions of
        ``filename``, mirroring :meth:`save`.

        The concrete class is chosen from the science extension: a 2D image
        (NAXIS == 2) yields an `EnhancedImage`, a stack (NAXIS == 3) yields an
        `EnhancedImage3D`. The science layer must be present; the error and
        data quality layers are optional and default to zeros of the matching
        shape when absent.
        """
        Msg.info(cls.__qualname__, f"Loading from '{filename}' (prefix '{prefix}')")

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

        # Dispatch on the shape of the science layer.
        target = EnhancedImage3D if isinstance(image, CplImageList) else EnhancedImage

        # Error and DQ are optional; pass them through as None when absent and
        # let the constructor synthesise zeros of the right shape/type.
        error, header_error = read_layer(cls.err_suffix)
        # The data quality layer is read as an integer to preserve the flag bits
        # rather than coercing them to floats.
        dq, header_dq = read_layer(cls.dq_suffix, CplType.INT)

        return target(image, error, dq,
                      prefix=prefix,
                      header_image=header_image,
                      header_error=header_error,
                      header_dq=header_dq)

    # ---- data quality ----

    def reject(self, bits: InstrumentDescription.MaskFlags = InstrumentDescription.MaskFlags.ALL) -> None:
        """
        Extract the selected bits from the DQ layer and push them into the
        1-bit bad-pixel mask of every science plane. By default, all bits are active.

        This *replaces* the planes' masks (``reject_from_mask`` does not
        accumulate): whatever an earlier operation left in the scratch masks is
        overwritten, so read it back with :meth:`rejected` first if it matters.
        """
        mask = self.dq[bits]
        for plane in self._hdrl_planes():
            plane.reject_from_mask(mask)

    def rejected(self) -> CplMask:
        """
        The union of the 1-bit bad-pixel masks of every science plane, i.e.
        whatever HDRL has rejected so far.

        This is the read-back half of the scratch-pad contract (see the class
        docstring): after an HDRL operation, record its rejections under a flag
        bit of the recipe's choosing with ``ei.dq.add(ei.rejected(), bit)``.
        The masks are transient and never saved, so anything not recorded this
        way is gone once :meth:`reject` runs again or the image is written out.
        """
        combined: Optional[np.ndarray] = None
        for plane in self._hdrl_planes():
            # `.image` returns a fresh copy, but one that carries the plane's bpm.
            mask = np.asarray(plane.image.bpm).astype(bool)
            combined = mask if combined is None else (combined | mask)
        return CplMask(combined)

    # ---- static helpers ----

    @staticmethod
    def _dimensions(layer: CplImage | CplImageList | CplMask | DataQuality) -> tuple[int, int]:
        """
        Return the (width, height) of a layer, which may be a single `Image`
        (or `Mask`) or an `ImageList` of equally-sized planes.
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
        `layer`. Used to synthesise an absent error layer.
        """
        width, height = EnhancedImageBase._dimensions(layer)

        def plane() -> CplImage:
            return CplImage(np.zeros((height, width), dtype=np.float64))

        if isinstance(layer, CplImageList):
            return CplImageList([plane() for _ in range(len(layer))])
        return plane()

    @staticmethod
    def _zero_dq(width: int, height: int) -> CplImage:
        """Create an all-good (all-zero) integer data quality image of the given size."""
        return CplImage(np.zeros((height, width), dtype=np.int32), dtype=CplType.INT)

    @staticmethod
    def _describe_layer(name: str, data: CplImage | CplImageList) -> str:
        """Describe a layer's name and type (with depth for ImageLists),
        e.g. 'DET1.SCI: ImageList[3]' or 'DET1.DQ: Image'. The dimensions are
        shared across layers and reported once by `__repr__`."""
        kind = (f'ImageList[{len(data)}]' if isinstance(data, CplImageList) else 'Image')
        return f'{name}: {kind}'


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
