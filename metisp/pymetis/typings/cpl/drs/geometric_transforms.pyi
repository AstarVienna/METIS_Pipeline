"""
Functions to compute the shift-and-add operation on an image list.
"""
from __future__ import annotations
import cpl.core
import typing
__all__: list[str] = ['Combine', 'FIRST', 'INTERSECT', 'UNION', 'offset_combine', 'offset_fine', 'offset_saa']
class Combine:
    """
    The CPL Geometry combination modes for the various cpl.drs.geometric_transforms functions
    
    Members:
    
      INTERSECT : Combine using the intersection of the images.
    
      UNION : Combine using the union of the images.
    
      FIRST : Combine using the first image to aggregate the other ones.
    """
    FIRST: typing.ClassVar[Combine]  # value = <Combine.FIRST: 2>
    INTERSECT: typing.ClassVar[Combine]  # value = <Combine.INTERSECT: 0>
    UNION: typing.ClassVar[Combine]  # value = <Combine.UNION: 1>
    __members__: typing.ClassVar[dict[str, Combine]]  # value = {'INTERSECT': <Combine.INTERSECT: 0>, 'UNION': <Combine.UNION: 1>, 'FIRST': <Combine.FIRST: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
def offset_combine(ilist: cpl.core.ImageList, offs: cpl.core.Bivector, min_rej: typing.SupportsInt | typing.SupportsIndex, max_rej: typing.SupportsInt | typing.SupportsIndex, union_flag: Combine, refine: bool = False, search_hx: typing.SupportsInt | typing.SupportsIndex | None = None, search_hy: typing.SupportsInt | typing.SupportsIndex | None = None, measure_hx: typing.SupportsInt | typing.SupportsIndex | None = None, measure_hy: typing.SupportsInt | typing.SupportsIndex | None = None, anchors: cpl.core.Bivector | None = None, sigmas: cpl.core.Vector | None = None) -> typing.Any:
    """
            Images list recombination
    
            If offset refinement is enabled this function will detect sources in the
            first image (unless a list of positions has been provided by the user using
            the `anchors` parameter) then use cross correlation to refine the provided
            estimated image offsets from the `offs` parameters. If offset refinement is
            disabled the image offsets in `offs` are used as they are.
    
            Following the optional offset refinement each image is shifted by the
            corresponding offset before being added together to produce a combined image.
    
            The supported types are cpl.core.Type.DOUBLE, cpl.core.Type.FLOAT.
    
            The number of provided offsets shall be equal to the number of input images.
            The ith offset (:code:`offs.x`, :code:`offs_y`) is the offset that has to be
            used to shift the ith image to align it on the first one.
    
            If offset refinement is enabled (`refine`=True), `anchors` or `sigmas` must
            be given, with `anchors` taking precidence.
    
            Parameters
            ----------
            ilist : cpl.core.ImageList
                Input image list
            offs : cpl.core.Bivector
                List of offsets in x and y. Applied directly if `refine` is False,
                otherwise it will be refined using cross-correlation.
            union_flag : cpl.drs.geometric_transforms.Combine
                Combination mode: cpl.drs.geometric_transforms.Combine.UNION,
                cpl.drs.geometric_transforms.Combine.INTERSECT or
                cpl.drs.geometric_transforms.Combine.FIRST.
            search_hx : int
                Half-width of search area. This parameter must be set when `refine` is
                `True`, if `refine` is `False` it has no effect.
            search_hy : int
                Half-height of search area. This parameter must be set when `refine`
                is `True`, otherwise it has no effect.
            measure_hx : int
                Half-width of the measurement area. This parameter must be set when
                `refine` is `True`, otherwise it has no effect.
            measure_hy : int
                Half-height of the measurement area. This parameter must be set when
                `refine` is `True`, otherwise it has no effect.
            refine : bool, optional
                Set to True to enable offset refinement offsets
            anchors : cpl.core.Bivector, optional
                List of cross corelation points in the first image. Unused if `refine`
                is set to False
            sigmas : cpl.core.Vector, optional
                Positive, decreasing sigmas to apply for cross-correlation point
                detection. Unused if `refine` is set to False, or if `refine` is
                True but `anchors` is given.
    
            Return
            ------
            NamedTuple(cpl.core.Image, cpl.core.Image, int or None)
                NamedTuple in the format (combined, contribution, pisigma) where:
    
                - combined: the combined image
                - contribution: the contribution map
                - pisigma: Index of the sigma that was used. None if `sigmas` is not given
    
            Raises
            ------
            cpl.core.NullInputError
                if `sigmas` is not given when either refine set to True and anchors is
                also not given
            cpl.core.IllegalInputError
                if ilist is not uniform, or if `search_hx`, `search_hy`, `measure_hx`
                and `measure_hy` have not been set when `refine` is set to `True`.
            cpl.core.IncompatibleInputError
                if ilist and offs have different sizes
            cpl.core.DataNotFoundError
                if the shift and add of the images fails
    
            See Also
            --------
            cpl.drs.geometric_transformations.offset_fine : used to refine the offsets if refine is `True`
            cpl.drs.geometric_transformations.offset_saa : used for image recombination using the default kernel
    """
def offset_fine(ilist: cpl.core.ImageList, estimates: cpl.core.Bivector, anchors: cpl.core.Bivector, search_hx: typing.SupportsInt | typing.SupportsIndex, search_hy: typing.SupportsInt | typing.SupportsIndex, measure_hx: typing.SupportsInt | typing.SupportsIndex, measure_hy: typing.SupportsInt | typing.SupportsIndex) -> tuple[cpl.core.Bivector, cpl.core.Vector]:
    """
            Get the offsets by correlating the images
    
            The images in the input list must only differ from a shift. In order
            from the correlation to work, they must have the same level (check the
            average values of your input images if the correlation does not work).
    
            The supported image types are cpl.core.Type.DOUBLE and cpl.core.Type.FLOAT.
            The bad pixel maps are ignored by this function.
    
            Parameters
            ----------
            ilist : cpl.core.ImageList
                Input image list
            estimates : cpl.core.Bivector
                First-guess estimation of the offsets
            anchors : cpl.core.Bivector
                List of cross-correlation points
            search_hx : int
                Half-width of search area
            search_hy : int
                Half-height of search area
            measure_hx : int
                Half-width of the measurement area
            measure_hy : int
                Half-height of the measurement area
    
            Return
            ------
            tuple(cpl.core.Bivector, cpl.core.Vector)
                Tuple of the List of offsets and the list of cross-correlation quality
                factors, in the format (`offsets`, `quality_factors`).
    
            Notes
            -----
            The matching is performed using a 2d cross-correlation, using a minimal
            squared differences criterion. One measurement is performed per input anchor
            point, and the median offset is returned together with a measure of
            similarity for each plane.
    
            The images in the input list must only differ from a shift. In order
            from the correlation to work, they must have the same level (check the
            average values of your input images if the correlation does not work).
    
            The ith offset (:code:`offsets.x`, :code:`offsets.y`) in the returned
            `offsets` is the one that have to be used to shift the ith image to align
            it on the reference image (the first one).
    
            Raises
            ------
            cpl.core.IllegalInputError
                if ilist is not valid
    """
def offset_saa(ilist: cpl.core.ImageList, offs: cpl.core.Bivector, kernel: cpl.core.Kernel, rejmin: typing.SupportsInt | typing.SupportsIndex, rejmax: typing.SupportsInt | typing.SupportsIndex, union_flag: Combine) -> typing.Any:
    """
            Shift and add an images list to a single image
    
            The supported types are cpl.core.Type.DOUBLE, cpl.core.Type.FLOAT.
    
            The number of provided offsets shall be equal to the number of input images.
            The ith offset (offs_x, offs_y) is the offset that has to be used to shift
            the ith image to align it on the first one.
    
            The following kernel types are supported when being passed to `kernel`:
    
                - cpl.core.Kernel.DEFAULT: default kernel, currently cpl.core.Kernel.TANH
                - cpl.core.Kernel.TANH: Hyperbolic tangent
                - cpl.core.Kernel.SINC: Sinus cardinal
                - cpl.core.Kernel.SINC2: Square sinus cardinal
                - cpl.core.Kernel.LANCZOS: Lanczos2 kernel
                - cpl.core.Kernel.HAMMING: Hamming kernel
                - cpl.core.Kernel.HANN: Hann kernel
                - cpl.core.Kernel.NEAREST: Nearest neighbor kernel (1 when dist < 0.5, else 0)
    
            If the number of input images is lower or equal to 3, the rejection
            parameters are ignored.
            If the number of input images is lower or equal to 2*(rejmin+rejmax), the
            rejection parameters are ignored.
    
            Pixels with a zero in the contribution map are flagged as bad in the
            combined image.
    
            The return values ppos_x and ppos_y follow the PyCPL standard, where the
            lower-leftmost pixel of the output image is at (0, 0). Note that this
            differs from the corresponding CPL function, where the lower-leftmost
            pixel of the output image is at (1, 1).
    
            Parameters
            ----------
            ilist : cpl.core.ImageList
                Input image list
            offs : cpl.core.Bivector
                List of offsets in x and y
            kernel : cpl.core.Kernel
                Interpolation kernel to use for resampling. See extended summary for
                supported kernel types
            rejmin : int
                Number of minimum value pixels to reject in stacking
            rejmax : int
                Number of maximum value pixels to reject in stacking
            union_flag : cpl.drs.geometric_transforms.Combine
                Combination mode: cpl.drs.geometric_transforms.Combine.UNION,
                cpl.drs.geometric_transforms.Combine.INTERSECT or cpl.drs.geometric_transforms.Combine.FIRST
    
            Return
            ------
            NamedTuple(cpl.core.Image, cpl.core.Image, float, float)
                NamedTuple in the format (combined, contribution, ppos_x, ppos_y) where:
    
                - combined: the combined image
                - contribution: the contribution map
                - ppos_x: X-position of the first image in the combined image
                - ppos_y: Y-position of the first image in the combined image
    
                `ppos_x` and `ppos_y` represent the pixel coordinate in
                the created output image-pair `combined` and `contribution` where the
                lowermost-leftmost pixel of the first input image is located. So with
                cpl.drs.geometric_transforms.Combine.FIRST this will always be (0, 0).
    
            Raises
            ------
            cpl.core.IllegalInputError
                if ilist is not valid or rejmin or rejmax is negative
            cpl.core.IncompatibleInputError
                if ilist and offs have different sizes
            cpl.core.IllegalOutputError
                if cpl.drs.geometric_transforms.INTERSECT is used with non-overlapping images.
            cpl.core.InvalidTypeError
                if the passed image list type is not supported
            cpl.core.UnsupportedModeError
                if union_flag is not one of the supported modes.
    """
FIRST: Combine  # value = <Combine.FIRST: 2>
INTERSECT: Combine  # value = <Combine.INTERSECT: 0>
UNION: Combine  # value = <Combine.UNION: 1>
