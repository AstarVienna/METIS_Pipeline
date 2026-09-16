"""
FFT operations via fftw wrappers
"""
from __future__ import annotations
import cpl.core
import typing
__all__: list[str] = ['BACKWARD', 'FIND_EXHAUSTIVE', 'FIND_MEASURE', 'FIND_PATIENT', 'FORWARD', 'Mode', 'NOSCALE', 'fft_image', 'fft_imagelist']
class Mode:
    """
    Members:
    
      FORWARD
    
      BACKWARD
    
      NOSCALE
    
      FIND_MEASURE
    
      FIND_PATIENT
    
      FIND_EXHAUSTIVE
    """
    BACKWARD: typing.ClassVar[Mode]  # value = <Mode.BACKWARD: 4>
    FIND_EXHAUSTIVE: typing.ClassVar[Mode]  # value = <Mode.FIND_EXHAUSTIVE: 64>
    FIND_MEASURE: typing.ClassVar[Mode]  # value = <Mode.FIND_MEASURE: 16>
    FIND_PATIENT: typing.ClassVar[Mode]  # value = <Mode.FIND_PATIENT: 32>
    FORWARD: typing.ClassVar[Mode]  # value = <Mode.FORWARD: 2>
    NOSCALE: typing.ClassVar[Mode]  # value = <Mode.NOSCALE: 8>
    __members__: typing.ClassVar[dict[str, Mode]]  # value = {'FORWARD': <Mode.FORWARD: 2>, 'BACKWARD': <Mode.BACKWARD: 4>, 'NOSCALE': <Mode.NOSCALE: 8>, 'FIND_MEASURE': <Mode.FIND_MEASURE: 16>, 'FIND_PATIENT': <Mode.FIND_PATIENT: 32>, 'FIND_EXHAUSTIVE': <Mode.FIND_EXHAUSTIVE: 64>}
    def __and__(self, other: typing.Any) -> typing.Any:
        ...
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __ge__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __gt__(self, other: typing.Any) -> bool:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __invert__(self) -> typing.Any:
        ...
    def __le__(self, other: typing.Any) -> bool:
        ...
    def __lt__(self, other: typing.Any) -> bool:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __or__(self, other: typing.Any) -> typing.Any:
        ...
    def __rand__(self, other: typing.Any) -> typing.Any:
        ...
    def __repr__(self) -> str:
        ...
    def __ror__(self, other: typing.Any) -> typing.Any:
        ...
    def __rxor__(self, other: typing.Any) -> typing.Any:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    def __xor__(self, other: typing.Any) -> typing.Any:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
def fft_image(other: cpl.core.Image, transform: Mode, find: Mode = None, scale: bool = True) -> cpl.core.Image:
    """
        Perform a FFT operation on an image
    
        Parameters
        ----------
        - other: The frameset from which the product frames are taken.
        - transform: cpl.drs.fft.FORWARD or cpl.drs.fft.FORWARD
        - find: based on enum, time spent searching (cpl.drs.fft.FIND_MEASURE,
                cpl.drs.fft.FIND_PATIENT, cpl.drs.fft.FIND_EXHAUSTIVE)
        - scale: true or false, whether or not to transform without scaling (only
                 effects backwards transforms)
    
        Return
        ------
        output image of the FFT operation
    
        Notes
        -----
        This function performs an FFT on an image, using FFTW. CPL may be configured
        without this library, in this case an otherwise valid call will set and throw
        UnsupportedModeError.
    
        The input and output images must match in precision level. Integer images are
        not supported.
    
        In a forward transform the input image may be non-complex. In this case a
        real-to-complex transform is performed. This will only compute the first
        nx/2 + 1 columns of the transform. In this transform it is allowed to pass
        an output image with nx/2 + 1 columns.
    
        Similarly, in a backward transform the output image may be non-complex. In
        this case a complex-to-real transform is performed. This will only transform
        the first nx/2 + 1 columns of the input. In this transform it is allowed to
        pass an input image with nx/2 + 1 columns.
    
        Per default the backward transform scales (divides) the result with the
        number of elements transformed (i.e. the number of pixels in the result
        image). This scaling can be turned off with CPL_FFT_NOSCALE.
    
        If many transformations in the same direction are to be done on data of the
        same size and type, a reduction in the time required to perform the
        transformations can be achieved by passing cpl.drs.FIND_MEASURE to the find
        param.
    
        For a larger number of transformations a further reduction may be achived
        cpl.drs.FIND_PATIENT and for an even larger number of
        transformations a further reduction may be achived with the flag
        cpl.drs.FIND_EXHAUSTIVE.
    
        If many transformations are to be done then a reduction in the time required
        to perform the transformations can be achieved by using cpl_fft_imagelist().
    
        Raises
        ------
        cpl.core.IllegalInputError
          if the mode is illegal
        cpl.core.TypeMismatchError
          if the image types are incompatible with each other
        cpl.core.UnsupportedModeError
          if FFTW has not been installed
    """
def fft_imagelist(other: cpl.core.ImageList, transform: Mode, find: Mode = None, scale: bool = True) -> cpl.core.ImageList:
    """
        Perform a FFT operation on the images in an imagelist
    
        Parameters
        ----------
        other : cpl.core.ImageList
          Input imagelist to transform from
        transform : cpl.drs.fft.Mode
          cpl.drs.fft.FORWARD or cpl.drs.fft.FORWARD
        find : cpl.drs.fft.Mode or None, default=None
          based on enum, time spent searching (cpl.drs.fft.FIND_MEASURE, cpl.drs.fft.FIND_PATIENT, cpl.drs.fft.FIND_EXHAUSTIVE)
        scale : bool, default=True
          true or false, whether or not to transform without scaling (only effects backwards transforms)
    
        Returns
        -------
        cpl.core.ImageList
          output imagelist to store transformed images
    
        Notes
        -----
        Convenience function for running cpl.drs.fft.image() on all images in the input imagelist
    """
BACKWARD: Mode  # value = <Mode.BACKWARD: 4>
FIND_EXHAUSTIVE: Mode  # value = <Mode.FIND_EXHAUSTIVE: 64>
FIND_MEASURE: Mode  # value = <Mode.FIND_MEASURE: 16>
FIND_PATIENT: Mode  # value = <Mode.FIND_PATIENT: 32>
FORWARD: Mode  # value = <Mode.FORWARD: 2>
NOSCALE: Mode  # value = <Mode.NOSCALE: 8>
