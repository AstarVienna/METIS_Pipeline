"""
CPL DRS submodule
  
  This module provides standard implementations of instrument independent,
  higher level data processing functions for general non-linear fitting, image
  fourier transformation, point pattern matching, world coordinate system
  transformation, etc.
  
"""
from __future__ import annotations
import cpl.core
import typing
from . import detector
from . import fft
from . import fit
from . import geometric_transforms
from . import photom
from . import ppm
from . import wlcalib
__all__: list[str] = ['Aperture', 'Apertures', 'WCS', 'WCSLibError', 'detector', 'fft', 'fit', 'geometric_transforms', 'photom', 'ppm', 'wlcalib']
class Aperture:
    """
    
            Returned from a Apertures' __getitem__ method or iterator. Used to access
            each Aperture record individually.
    
            Not instantiatable on its own.
        
    """
    @property
    def bottom(self) -> int:
        """
        The bottommost y position in an aperture
        """
    @property
    def bottom_x(self) -> int:
        """
                    The x position of the bottommost y position in an aperture. An aperture may
                    have multiple bottommost x positions, in which case one of these is returned.
        """
    @property
    def centroid_x(self) -> float:
        """
        The X-centroid of an aperture
        """
    @property
    def centroid_y(self) -> float:
        """
                    The Y-centroid of an aperture. For a concave aperture the centroid may
                    not belong to the aperture.
        """
    @property
    def flux(self) -> float:
        """
        The flux of an aperture
        """
    @property
    def left(self) -> int:
        """
        The leftmost x position in an aperture
        """
    @property
    def left_y(self) -> int:
        """
                    The y position of the leftmost x position in an aperture. An aperture may
                    have multiple leftmost y positions, in which case one of these is returned.
        """
    @property
    def max(self) -> float:
        """
        The maximum value of an aperture
        """
    @property
    def maxpos_x(self) -> int:
        """
        The X-position of the aperture maximum value
        """
    @property
    def maxpos_y(self) -> int:
        """
        The Y-position of the aperture maximum value
        """
    @property
    def mean(self) -> float:
        """
        The mean value of an aperture
        """
    @property
    def median(self) -> float:
        """
        The median value of an aperture
        """
    @property
    def min(self) -> float:
        """
        The minimum value of an aperture
        """
    @property
    def minpos_x(self) -> int:
        """
        The X-position of the aperture minimum value
        """
    @property
    def minpos_y(self) -> int:
        """
        The Y-position of the aperture minimum value
        """
    @property
    def npix(self) -> int:
        """
        The number of pixels of an aperture
        """
    @property
    def pos_x(self) -> float:
        """
        average X-position of an aperture
        """
    @property
    def pos_y(self) -> float:
        """
        average Y-position of an aperture
        """
    @property
    def right(self) -> int:
        """
        The rightmost x position in an aperture
        """
    @property
    def right_y(self) -> int:
        """
                    The y position of the rightmost x position in an aperture. An aperture may
                    have multiple rightmost y positions, in which case one of these is returned.
        """
    @property
    def stdev(self) -> float:
        """
                    The standard deviation value of an aperture
        
                    Raises
                    ------
                    cpl.core.DataNotFoundError
                        if the aperture comprises of less than two pixels
        """
    @property
    def top(self) -> int:
        """
        The topmost y position in an aperture
        """
    @property
    def top_x(self) -> int:
        """
                    The x position of the topmost y position in an aperture. An aperture may
                    have multiple topmost x positions, in which case one of these is returned.
        """
class Apertures:
    """
    
        Compute statistics on selected apertures.
    
        The aperture object contains a list of zones in an image. It is typically
        used to contain the results of an objects detection, or if one wants to work
        on a very specific zone in an image.
    
        Can be built either with the constructor with a reference and labellised image,
        or via the various static `extract_*` functions.
    
        Each individual Aperture statistic can be accessed either via the `get_*`
        methods (using 1 indexing) or by indexing the Apertures themselves (e.g.
        apt[0], 0 indexing), which will return an `Aperture` object, with the properties
        corresponding to the individual Aperture statistics.
    
        Parameters
        ----------
        reference : cpl.core.Image
            Reference image
        labelized : cpl.core.Image
            Labelized image (of type cpl.core.Type.INT). Must contain at least one pixel
            for each value from 1 to the maximum value in the image.
    
        Raises
        ------
        cpl.core.TypeMismatchError
            if labelized is not of cpl.core.Type.INT
        cpl.core.IllegalInputError
            if labelized has a negative value or zero maximum
        cpl.core.IncompatibleInputError
            if lab and inImage have different sizes.
    
        Notes
        -----
        For the centroiding computation of an aperture, if some pixels have
        values lower or equal to 0, all the values of the aperture are locally
        shifted such as the minimum value of the aperture has a value of
        epsilon. The centroid is then computed on these positive values. In
        principle, centroid should always be computed on positive values, this
        is done to avoid raising an error in case the caller of the function
        wants to use it on negative values images without caring about the
        centroid results. In such cases, the centroid result would be
        meaningful, but slightly depend on the hardcoded value chosen for
        epsilon (1e-10).
    
        See Also
        --------
        cpl.core.Image.labelise_create : Can be used for creating `labelized`.
    """
    @staticmethod
    def extract(source_image: cpl.core.Image, sigmas: cpl.core.Vector) -> typing.Any:
        """
            Simple detection of apertures in an image
        
            Aperture detection on the image is performed using each value in `sigmas`
            until at least one is found.
        
            Parameters
            ----------
            source_image : cpl.core.Image
                The image to process
            sigmas : cpl.core.Vector
                Detection levels. Positive, decreasing sigmas to apply
        
            Returns
            -------
            cpl.drs.Apertures, int
                The detected apertures (cpl.drs.Apertures) and the index of the sigma that
                was used (int)
        
            Raises
            ------
            cpl.core.DataNotFoundError
                if the apertures could not be detected
        
            See Also
            --------
            cpl.drs.Apertures.extract_sigma :
                Used on the image for aperture detection. Also provides detailed explaination
                of individual sigmas.
        """
    @staticmethod
    def extract_mask(source_image: cpl.core.Image, selection: typing.Any) -> Apertures:
        """
            Simple detection of apertures in an image from a user supplied selection mask
        
            The values selected for inclusion in the apertures must have the non-zero value
            in the selection mask, and must not be flagged as bad in the bad pixel map of
            the image.
        
            Parameters
            ----------
            source_image : cpl.core.Image
                The image to process. Can be of type cpl.core.Type.DOUBLE,
                cpl.core.Type.FLOAT, or cpl.core.Type.INT
            sigmas : cpl.core.Vector
                Detection levels. Positive, decreasing sigmas to apply
        
            Returns
            -------
            cpl.drs.Apertures
                The detected apertures
        
            Raises
            ------
            cpl.core.IncompatibleInputError
                if`source_image`and selection have different sizes
            cpl.core.TypeMistmatchError
                if`source_image`is of a complex type
            cpl.core.DataNotFoundError
                if the selection mask is empty
        """
    @staticmethod
    def extract_sigma(source_image: cpl.core.Image, selection: typing.SupportsFloat | typing.SupportsIndex) -> Apertures:
        """
            Simple detection of apertures in an image using a provided sigma
        
            Sigma is used to calculate the threshold for the aperture detection. This
            threshold is calculated using the median plus the average distance to the median
            times sigma.
        
            Parameters
            ----------
            source_image : cpl.core.Image
                The image to process
            sigma : float
                Detection level. Used as a variable to calculate the threshold for detection.
        
            Returns
            -------
            cpl.drs.Apertures
                The detected apertures
        
            Raises
            ------
            cpl.core.IllegalInputError
                if sigma is non-positive
            cpl.core.TypeMismatchError
                if`source_image`is of a complex type
            cpl.core.DataNotFoundError
                if the apertures could not be detected
        
            Notes
            -----
            In order to avoid (the potentially many) detections of small objects the mask
            of detected pixels is subjected to a 3x3 morphological opening filter.
        """
    @staticmethod
    def extract_window(source_image: cpl.core.Image, sigmas: cpl.core.Vector, area: tuple) -> typing.Any:
        """
            Simple detection of apertures in an image window
        
            Aperture detection on the window is performed using each value in `sigmas` until
            at least one is found.
        
            Parameters
            ----------
            source_image : cpl.core.Image
                The image to process
            sigmas : cpl.core.Vector
                Detection level. Positive, decreasing sigmas to apply
            area : tuple(int, int, int, int)
                Rectangle of the window in the format (llx, lly, urx, ury) where:
                
                    - llx : Lower left x position
                    - lly : Lower left y position
                    - urx : Upper right x position
                    - ury : Upper right y position
        
                Position indices are zero based.
        
            Returns
            -------
            cpl.drs.Apertures, int
                The detected apertures (cpl.drs.Apertures) and the index of the sigma that
                was used (int)
        
            Raises
            ------
            cpl.core.DataNotFoundError
                if the apertures could not be detected
        
            See Also
            --------
            cpl.drs.Apertures.extract_sigma :
                Used on the window for aperture detection. Also provides detailed
                explaination of individual sigmas.
        """
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Aperture:
        ...
    def __init__(self, reference: cpl.core.Image, labelized: cpl.core.Image) -> None:
        ...
    def __iter__(self) -> typing.Any:
        ...
    def __len__(self) -> int:
        ...
    def __next__(self) -> Aperture:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump the Apertures contents to a file, stdout or a string.
                  
                This function is mainly intended for debug purposes.
        
                Parameters
                ----------
                filename : str, optional
                    file path to dump apertures contents to
                mode : str, optional
                    File mode to save the file, default 'w' overwrites contents.
                show : bool, optional
                    Send apertures contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the apertures contents.
        """
    def get_bottom(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the bottommost y position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the bottommost y position in the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_bottom_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the x position of the bottommost y position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the bottommost x position of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_centroid_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the X-centroid of an aperture
        
            For a concave aperture the centroid may not belong to the aperture.
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            float
                The X-centroid of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_centroid_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the Y-centroid of an aperture
        
            For a concave aperture the centroid may not belong to the aperture.
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            float
                The Y-centroid of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_flux(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the flux of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The flux of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_left(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the leftmost x position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the leftmost x position of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_left_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the y position of the leftmost x position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the y position of the leftmost x position
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_max(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the maximum value of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The maximum value of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_maxpos_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the X-position of the aperture maximum value
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The X-position of the aperture maximum value
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_maxpos_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the Y-position of the aperture maximum value
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The Y-position of the aperture maximum value
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_mean(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the mean value of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The mean value of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_median(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the median value of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The median value of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_min(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the minimum value of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The minimum value of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_minpos_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the X-position of the aperture minimum value
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The X-position of the aperture minimum value
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_minpos_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the Y-position of the aperture minimum value
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The Y-position of the aperture minimum value
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_npix(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the number of pixels of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The number of pixels of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_pos_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the average X-position of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            float
                The average X-position of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_pos_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the average Y-position of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            float
                The average Y-position of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_right(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the rightmost x position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the rightmost x position in an aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_right_y(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the y position of the rightmost x position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the y position of the rightmost x position
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_stdev(self, idx: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
            Get the standard deviation value of an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                The standard deviation value of the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
            cpl.core.DataNotFOundError
                if the aperture comprises of less than two pixels
        """
    def get_top(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the topmost y position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the topmost y position in the aperture
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def get_top_x(self, idx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
            Get the x position of the topmost y position in an aperture
        
            Parameters
            ----------
            idx : int
                The aperture index (1 for the first one)
        
            Returns
            -------
            int
                the x position of the topmost y position or negative on error
        
            Raises
            ------
            cpl.core.IllegalInputError
                if idx is non-positive
            cpl.core.AccessOutOfRangeError
                if idx is greater than the number of apertures
        """
    def sort_by_flux(self) -> None:
        """
        Sort apertures by decreasing aperture flux and apply changes
        """
    def sort_by_max(self) -> None:
        """
        Sort apertures by decreasing peak value and apply changes
        """
    def sort_by_npix(self) -> None:
        """
        Sort apertures by decreasing size (in pixels) and apply changes
        """
class WCS:
    """
    
            WCS(cpl.core.PropertyList plist)
            
            Create a WCS object by parsing a propertylist.
    
            Notes
            -----
            The WCS object is created reading the WCS keyword information from the
            property list `plist` which is used to setup a WCSLIB data structure. In
            addition a few ancillary items are also filled in.
    
            It is allowed to pass a :py:class:`cpl.core.PropertyList` with a valid WCS
            structure and ``NAXIS`` = 0. Such a propertylist can be created by the method
            :py:meth:`platesol`.
    
            Trying to use any function without first installing WCSLIB will result in a
            :py:exc:`cpl.core.NoWCSError`.
            
    """
    class platesol_fitmode:
        """
        Members:
        
          PLATESOL_4
        
          PLATESOL_6
        """
        PLATESOL_4: typing.ClassVar[WCS.platesol_fitmode]  # value = <platesol_fitmode.PLATESOL_4: 1>
        PLATESOL_6: typing.ClassVar[WCS.platesol_fitmode]  # value = <platesol_fitmode.PLATESOL_6: 0>
        __members__: typing.ClassVar[dict[str, WCS.platesol_fitmode]]  # value = {'PLATESOL_4': <platesol_fitmode.PLATESOL_4: 1>, 'PLATESOL_6': <platesol_fitmode.PLATESOL_6: 0>}
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
    class platesol_outmode:
        """
        Members:
        
          MV_CRVAL
        
          MV_CRPIX
        """
        MV_CRPIX: typing.ClassVar[WCS.platesol_outmode]  # value = <platesol_outmode.MV_CRPIX: 1>
        MV_CRVAL: typing.ClassVar[WCS.platesol_outmode]  # value = <platesol_outmode.MV_CRVAL: 0>
        __members__: typing.ClassVar[dict[str, WCS.platesol_outmode]]  # value = {'MV_CRVAL': <platesol_outmode.MV_CRVAL: 0>, 'MV_CRPIX': <platesol_outmode.MV_CRPIX: 1>}
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
    class trans_mode:
        """
        Members:
        
          PHYS2WORLD
        
          WORLD2PHYS
        
          WORLD2STD
        
          PHYS2STD
        """
        PHYS2STD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.PHYS2STD: 2>
        PHYS2WORLD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.PHYS2WORLD: 0>
        WORLD2PHYS: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.WORLD2PHYS: 1>
        WORLD2STD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.WORLD2STD: 3>
        __members__: typing.ClassVar[dict[str, WCS.trans_mode]]  # value = {'PHYS2WORLD': <trans_mode.PHYS2WORLD: 0>, 'WORLD2PHYS': <trans_mode.WORLD2PHYS: 1>, 'WORLD2STD': <trans_mode.WORLD2STD: 3>, 'PHYS2STD': <trans_mode.PHYS2STD: 2>}
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
    MV_CRPIX: typing.ClassVar[WCS.platesol_outmode]  # value = <platesol_outmode.MV_CRPIX: 1>
    MV_CRVAL: typing.ClassVar[WCS.platesol_outmode]  # value = <platesol_outmode.MV_CRVAL: 0>
    PHYS2STD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.PHYS2STD: 2>
    PHYS2WORLD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.PHYS2WORLD: 0>
    PLATESOL_4: typing.ClassVar[WCS.platesol_fitmode]  # value = <platesol_fitmode.PLATESOL_4: 1>
    PLATESOL_6: typing.ClassVar[WCS.platesol_fitmode]  # value = <platesol_fitmode.PLATESOL_6: 0>
    WORLD2PHYS: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.WORLD2PHYS: 1>
    WORLD2STD: typing.ClassVar[WCS.trans_mode]  # value = <trans_mode.WORLD2STD: 3>
    @staticmethod
    def platesol(ilist: cpl.core.PropertyList, cel: cpl.core.Matrix, xy: cpl.core.Matrix, niter: typing.SupportsInt | typing.SupportsIndex, thresh: typing.SupportsFloat | typing.SupportsIndex, fitmode: WCS.platesol_fitmode, outmode: WCS.platesol_outmode) -> cpl.core.PropertyList:
        """
            Do a 2d plate solution given physical and celestial coordinates
        
            Parameters
            ----------
            ilist : cpl.core.PropertyList
                The input property list containing the first pass WCS
            cel : cpl.core.Matrix
                The celestial coordinate matrix
            xy : cpl.core.Matrix
                The physical coordinate matrix
            niter : int
                The number of fitting iterations
            thresh : float
                The threshold for the fitting rejection cycle
            fitmode : cpl.drs.WCS.platesol_fitmode
                The fitting mode (see below)
            outmode : cpl.drs.WCS.platesol_outmode
                The output mode (see below)
        
            Returns
            -------
            cpl.core.PropertyList
                The output property list containing the new WCS
        
            Notes
            -----
            This function allows for the following type of fits:
        
            - cpl.drs.WCS.PLATESOL_4: Fit for zero point, 1 scale and 1 rotation.
            - cpl.drs.WCS.PLATESOL_6: Fit for zero point, 2 scales, 1 rotation, 1 shear.
        
            This function allows the zeropoint to be defined by shifting either the
            physical or the celestial coordinates of the reference point:
        
            - cpl.drs.WCS.MV_CRVAL: Keeps the physical point fixed and shifts the celestial
            - cpl.drs.WCS.MV_CRPIX: Keeps the celestial point fixed and shifts the physical
        
            The output property list contains WCS relevant information only.
        
            Raises
            ------
            cpl.core.UnspecifiedError
                If unable to parse the input propertylist into a proper FITS WCS or there
                are too few points in the input matrices for a fit.
            cpl.core.IncompatibleInputError
                If the matrices `cel` and `xy` have different sizes.
            cpl.core.UnsupportedModeError
                If either fitmode or outmode are specified incorrectly.
            cpl.core.DataNotFoundError
                If the threshold is so low that no valid points are found. If the threshold 
                is not positive, this error is certain to occur.
            cpl.core.IllegalInputError
                If the parameter niter is non-positive.
        """
    def __init__(self, arg0: cpl.core.PropertyList) -> None:
        ...
    def convert(self, from_: cpl.core.Matrix, transform: WCS.trans_mode) -> cpl.core.Matrix:
        """
            Convert between coordinate systems.
        
            Parameters
            ----------
            from : cpl.core.Matrix
                The input coordinate matrix
            transform : cpl.drs.WCS.trans_mode
                The transformation mode
        
            Returns
            -------
            cpl.core.Matrix
                The output coordinate matrix
        
            Raises
            ------
            cpl.drs.WCSLibError
                If any error occurs during conversion, retrieved from WCSLIB.
            cpl.core.UnspecifiedError
                If no rows or columns in the input matrix, or an unspecified
                error has occurred in the WCSLIB routine
            cpl.core.UnsupportedModeError
                If the input conversion mode is not supported
        
            Notes
            -----
            This function converts between several types of coordinates. These include:
            
            physical coordinates:
                The physical location on a detector (i.e. pixel coordinates)
            world coordinates:
                 The real astronomical coordinate system for the observations. This may
                 be spectral, celestial, time, etc.
            standard coordinates:
                These are an intermediate relative coordinate representation, defined as a
                distance from a reference point in the natural units of the world coordinate
                system. Any defined projection geometry will have already been included in the
                definition of standard coordinates.
        
            The supported conversion modes are:
        
            - cpl.drs.WCS.trans_mode.PHYS2WORLD: Converts from physical to world coordinates
            - cpl.drs.WCS.trans_mode.WORLD2PHYS: Converts from world to physical coordinates
            - cpl.drs.WCS.trans_mode.WORLD2STD: Converts from world to standard coordinates
            - cpl.drs.WCS.trans_mode.PHYS2STD: Converts from physical to standard coordinates
        """
    @property
    def _handle(self) -> typing.Any:
        """
        Opaque handle to the internal WCS representation.
        """
    @property
    def cd(self) -> cpl.core.Matrix:
        ...
    @property
    def crpix(self) -> list[float]:
        ...
    @property
    def crval(self) -> list[float]:
        ...
    @property
    def ctype(self) -> list[str]:
        ...
    @property
    def cunit(self) -> list[str]:
        ...
    @property
    def image_dims(self) -> list[int]:
        """
        Axis lengths of the image associated with a WCS.
        """
    @property
    def image_naxis(self) -> int:
        """
        Dimensionality of the image associated with a WCS.
        """
class WCSLibError(Exception):
    """
    
    Used to return errors from WCSLIB conversion functions.
    
    Contains error_list attribute containing a list of all errors found
    in the opertation for each row in the format:
    (matrix row, error enum string)
    
    This is not meant to be thrown in the Python environment.
    """
    def __init__(self, error_list, message):
        ...
