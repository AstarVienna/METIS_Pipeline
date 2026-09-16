"""
High-level functions to compute detector features.
"""
from __future__ import annotations
import cpl.core
import typing
__all__: list[str] = ['get_bias_window', 'get_noise_ring', 'get_noise_window', 'interpolate_rejected']
def get_bias_window(bias_image: cpl.core.Image, zone_def: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex] | None = None, ron_hsize: typing.SupportsInt | typing.SupportsIndex = -1, ron_nsamp: typing.SupportsInt | typing.SupportsIndex = -1) -> tuple[float, float]:
    """
        Compute the bias in a rectangle.
    
        This function is meant to compute the bias level from an image by means of a
        MonteCarlo approach. The input image would normally be a bias frame although
        no check is done on that, it is up to the caller to feed in the right kind of
        frame.
    
        Parameters
        ----------
        bias_image: cpl.core.Image
            Input image, normally a bias frame
        zone_def: tuple(int, int, int, int), optional
            Tuple to describe the window where the bias is to be computed in the
            format (xmin, xmax, ymin, ymax), using PyCPL notation where the bottom
            left pixel is (0,0)
        ron_hsize: int, optional
            to specify half size of squares default 4
        ron_nsamp: int, optional
            to specify the nb of samples, default 1000
    
        Returns
        -------
        tuple(float, float)
            The bias in the frame and the error of the bias in the format (bias, error)
    
        Raises
        ------
        cpl.core.IllegalInputError
            if the specified window (zone_def) is invalid
    
        Notes
        -----
        The algorithm will create typically 100 9x9 windows on the frame, scattered
        optimally using a Poisson law. In each window, the mean of all pixels in the
        window is computed and this value is stored.
    
        The output `bias` is the median of all computed means, and the error is the
        standard deviation of the means.
    """
def get_noise_ring(diff_image: cpl.core.Image, zone_def: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsFloat | typing.SupportsIndex, typing.SupportsFloat | typing.SupportsIndex], ron_hsize: typing.SupportsInt | typing.SupportsIndex = -1, ron_nsamp: typing.SupportsInt | typing.SupportsIndex = -1) -> tuple[float, float]:
    """
        Compute the noise in a ring.
    
        This function is meant to compute the noise in a frame by means of a
        MonteCarlo approach. The input is a frame, usually a difference between two
        frames taken with the same settings for the acquisition system, although no
        check is done on that, it is up to the caller to feed in the right kind of
        frame.
    
        If the input image is the difference of two bias frames taken with the same settings
        then the returned noise measure will be sqrt(2) times the image sensor read noise
    
        Parameters
        ----------
        diff_image: cpl.core.Image
            Input image, usually a difference frame.
        zone_def: tuple(int, int, float, float)
            Tuple to describe the window where the bias is to be computed in the
            format (x, y, r1, r2). The first two intergers specify the centre position
            of the ring as x, y, using PyCPL notation where the bottom left is (0,0).
            Floats r1 and r2 specify the ring start and end radiuses.
        ron_hsize: int, optional
            to specify half size of squares default 4
        ron_nsamp: int, optional
            to specify the nb of samples, default 1000
    
        Returns
        -------
        tuple(float, float)
            The noise in the frame and the error of the noise in the format (noise, error).
    
        Raises
        ------
        cpl.core.IllegalInputError
            if the internal radius (r1) is bigger than the external one (r2) in `zone_def`
        cpl.core.DataNotFoundError
            If an insufficient number of samples were found inside the ring
    
        Notes
        -----
        The algorithm will create typically 100 9x9 windows on the frame, scattered
        optimally using a Poisson law. In each window, the standard deviation of all
        pixels in the window is computed and this value is stored. The `output` noise
        is the median of all computed standard deviations, and the error is the
        standard deviation of the standard deviations.
    
        See Also
        --------
        cpl.drs.detector.get_noise_window : Computes noise using a rectangle.
    """
def get_noise_window(diff_image: cpl.core.Image, zone_def: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex] | None = None, ron_hsize: typing.SupportsInt | typing.SupportsIndex = -1, ron_nsamp: typing.SupportsInt | typing.SupportsIndex = -1) -> tuple[float, float]:
    """
        Compute the noise in a rectangle.
    
        This function is meant to compute the noise in a frame by means of a
        MonteCarlo approach. The input is a frame, usually a difference between two
        frames taken with the same settings for the acquisition system, although no
        check is done on that, it is up to the caller to feed in the right kind of
        frame.
    
        If the input image is the difference of two bias frames taken with the same settings
        then the returned noise measure will be sqrt(2) times the image sensor read noise
    
        Parameters
        ----------
        diff_image: cpl.core.Image
            Input image, usually a difference frame.
        zone_def: tuple(int, int, int, int), optional
            Tuple to describe the window where the bias is to be computed in the format (xmin, xmax, ymin, ymax), using PyCPL notation where the bottom left is (0,0)
        ron_hsize: int, optional
            to specify half size of squares, default 4
        ron_nsamp: int, optional
            to specify the nb of samples, default 1000
    
        Returns
        -------
        tuple(float, float)
            The noise in the frame and the error of the noise in the format (noise, error).
    
        Raises
        ------
        cpl.core.IllegalInputError
            if the specified window (zone_def) is invalid
    
        Notes
        -----
        The algorithm will create typically 100 9x9 windows on the frame, scattered
        optimally using a Poisson law. In each window, the standard deviation of all
        pixels in the window is computed and this value is stored.
    
        The output `noise` is the median of all computed standard deviations, and the error is the
        standard deviation of the standard deviations.
    
        See Also
        --------
        cpl.drs.detector.get_noise_ring : Computes noise using a ring.
    """
def interpolate_rejected(to_clean: cpl.core.Image) -> None:
    """
        Interpolate any bad pixels in an image in place
    
        Parameters
        ----------
        to_clean: cpl.core.Image
            The image to clean
    
        Raises
        ------
        cpl.core.DataNotFoundError
            if all pixels are bad
    
        Notes
        -----
        The value of a bad pixel is interpolated from the good pixels among the
        8 nearest. (If all but one of the eight neighboring pixels are bad, the
        interpolation becomes a nearest neighbor interpolation). For integer
        images the interpolation in done with floating-point and rounded to the
        nearest integer.
    
        If there are pixels for which all of the eight neighboring pixels are bad,
        a subsequent interpolation pass is done, where the already interpolated
        pixels are included as source for the interpolation.
    
        The interpolation passes are repeated until all bad pixels have been
        interpolated. In the worst case, all pixels will be interpolated from a
        single good pixel.
    """
