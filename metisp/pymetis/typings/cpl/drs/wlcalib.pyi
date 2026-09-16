"""
Wavelength calibration functions
"""
from __future__ import annotations
import collections.abc
import cpl.core
import typing
__all__: list[str] = ['SlitModel']
class SlitModel:
    """
    
            Line model to generate a spectrum.
    
            The model comprises these elements:
    
            - Slit Width
            - FWHM of transfer function
            - Truncation threshold of the transfer function
            - Catalog of lines (typically arc or sky)
    
            The units of the X-values of the lines is a length, it is assumed to be the
            same as that of the Y-values of the dispersion relation (e.g. meter), the
            units of slit width and the FWHM are assumed to be the same as the X-values
            of the dispersion relation (e.g. pixel), while the units of the produced
            spectrum will be that of the Y-values of the lines.
    
            The line profile is truncated at this distance [pixel] from its maximum:
    
            .. math::
    
                x_{\\mathrm{max}} = w/2 + k\\sigma
    
            where w is the slit width, k is the threshold and
            :math:`\\sigma = w_{\\mathrm{FWHM}}/(2\\sqrt{2\\log(2)})`
            where :math:`w_{\\mathrm{FWHM}}` is the Full Width at Half Maximum (FWHM)
            of the transfer function.
    
            The units of the X-values of the lines is a length, it is assumed to be the
            same as that of the Y-values of the dispersion relation (e.g. meter), the
            units of slit width and the FWHM are assumed to be the same as the X-values
            of the dispersion relation (e.g. pixel), while the units of the produced
            spectrum will be that of the Y-values of the lines.
    
            Parameters
            ----------
            catalog : cpl.core.Bivector
                the catalog of lines to be used by the spectrum filler
            wfwhm : float
                the FWHM of th etransfer function to be used by the spectrum filler
            wslit : float
                the slit width to be used by the spectrum filler
            spectrum_size : int
                The size of the spectrum, returned by the spectrum filler functions
            threshold : float
                The threshold for truncating the transfer function, default 5 (recommended).
          
    """
    def __init__(self, catalog: cpl.core.Bivector, wfwhm: typing.SupportsFloat | typing.SupportsIndex, wslit: typing.SupportsFloat | typing.SupportsIndex, spectrum_size: typing.SupportsInt | typing.SupportsIndex, threshold: typing.SupportsFloat | typing.SupportsIndex = 5.0) -> None:
        """
            Create a new line model to be initialized.
        
        
        
            Return
            ------
            cpl.drs.SlitModel
                Newly created line model
        
            Raises
            ------
            cpl.core.IllegalInputError
                if threshold, wfwhm or wslit is non-positive
        """
    def fill_line_spectrum(self, dispersion: cpl.core.Polynomial) -> cpl.core.Vector:
        """
            Generate a 1D spectrum from a model and a dispersion relation from the line intensities.
        
            Parameters
            ----------
            disp : cpl.core.Polynomial
                1D-Dispersion relation, at least of degree 1
        
            Returns
            -------
            cpl.core.Vector
                A vector of self.spectrum_size, containing the spectrum generated.
        
            Notes
            -----
            Each line profile is given by the convolution of the Dirac delta function
            with a Gaussian with :math:`sigma = w_{\\mathrm{FWHM}}/(2\\sqrt{2\\log(2)})` and
            a top-hat with the slit width as width. This continuous line profile is then
            integrated over each pixel, wherever the intensity is above the threshold
            set by the given model. For a given line the value on a given pixel
            requires the evaluation of two calls to erf().
        """
    def fill_line_spectrum_fast(self, dispersion: cpl.core.Polynomial) -> cpl.core.Vector:
        """
            Generate a 1D spectrum from a model and a dispersion relation from the line intensities, approximating the line profile for speed.
            
            The approximation preserves the position of the maximum, the symmetry and
            the flux of the line profile.
        
            The fast spectrum generation can be useful when the model spectrum includes
            many catalog lines.
        
            Parameters
            ----------
            disp : cpl.core.Polynomial
                1D-Dispersion relation, at least of degree 1
        
            Returns
            -------
            cpl.core.Vector
                A vector of self.spectrum_size, containing the spectrum generated.
        
            Notes
            -----
            Each line profile is given by the convolution of the Dirac delta function
            with a Gaussian with
        
            .. math::
        
                \\sigma = w_{\\mathrm{FWHM}}/(2\\sqrt{2\\log(2)}) and a
        
            top-hat with the slit width as width. This continuous line profile is then
            integrated over each pixel, wherever the intensity is above the threshold
            set by the given model. The use of a given line in a spectrum requires the 
            evaluation of four calls to erf().
        """
    def fill_logline_spectrum(self, dispersion: cpl.core.Polynomial) -> cpl.core.Vector:
        """
            Generate a 1D spectrum from a model and a dispersion relation from log(1 + the line intensities).
        
            Parameters
            ----------
            disp : cpl.core.Polynomial
                1D-Dispersion relation, at least of degree 1
        
            Returns
            -------
            cpl.core.Vector
                A vector of self.spectrum_size, containing the spectrum generated.
        
            Notes
            -----
            Each line profile is given by the convolution of the Dirac delta function
            with a Gaussian with ..math:: sigma = w_{\\mathrm{FWHM}}/(2\\sqrt{2\\log(2)}) and a
            top-hat with the slit width as width. This continuous line profile is then
            integrated over each pixel, wherever the intensity is above the threshold
            set by the given model. For a given line the value on a given pixel
            requires the evaluation of two calls to erf().
        """
    def fill_logline_spectrum_fast(self, dispersion: cpl.core.Polynomial) -> cpl.core.Vector:
        """
            Generate a 1D spectrum from a model and a dispersion relation from
            log(1 + the line intensities), approximating the line profile for speed.
        
            The approximation preserves the position of the maximum, the symmetry and the
            flux of the line profile.
        
            The fast spectrum generation can be useful when the model spectrum includes many
            catalog lines.
        
            Parameters
            ----------
            disp : cpl.core.Polynomial
                1D-Dispersion relation, at least of degree 1
        
            Returns
            -------
            cpl.core.Vector
                A vector of self.spectrum_size, containing the spectrum generated.
        
            Notes
            -----
            Each line profile is given by the convolution of the Dirac delta function
            with a Gaussian with :math:`\\sigma = w_{\\mathrm{FWHM}}/(2\\sqrt{2\\log(2)})` and a
            top-hat with the slit width as width. This continuous line profile is then
            integrated over each pixel, wherever the intensity is above the threshold
            set by the given model. The use of a given line in a spectrum requires the 
            evaluation of four calls to erf().
        """
    def find_best_1d(self, spectrum: cpl.core.Vector, wl_search: cpl.core.Vector, nsamples: typing.SupportsInt | typing.SupportsIndex, hsize: typing.SupportsInt | typing.SupportsIndex, filler: collections.abc.Callable, guess: cpl.core.Polynomial | None = None) -> typing.Any:
        """
            Find the best 1D dispersion polynomial in a given search space
        
            Find the polynomial that maximizes the cross-correlation between an
            observed 1D-spectrum and a model spectrum based on the polynomial
            dispersion relation.
            
            Parameters
            ----------
            spectrum : cpl.core.Vector
                The vector with the observed 1D-spectrum
            wl_search : cpl.core.Vector
                Search range around the anchor points
            nsamples : int 
                Number of samples around the anchor points
            hsize : int
                Maximum (pixel) displacement of the polynomial guess
            filler : function(cpl.core.Vector, cpl.core.Polynomial)
                The function used to make the spectrum. Currently only supports fill functions
                in cpl.drs.wlcalib, including:
        
                - cpl.drs.wlcalib.SlitModel.fill_line_spectrum
                - cpl.drs.wlcalib.SlitModel.fill_line_spectrum_fast
                - cpl.drs.wlcalib.SlitModel.fill_logline_spectrum
                - cpl.drs.wlcalib.SlitModel.fill_logline_spectrum_fast
            guess : cpl.core.Polynomial, optional
                1D-polynomial with the guess. If not given the guess will simply be a 1D-Polynomial 
                with no coefficients
        
            Return
            -------
            NamedTuple(cpl.core.Polynomial, float, cpl.core.Vector)
                NamedTuple in the format (result, xcmax, xcoors) where:
        
                - result: the resulting best 1D dispersion polynomial
                - xcmax: the maximum cross-correlation
                - xcoors: the correlation values    
        
            Raises
            ------
            cpl.core.InvalidTypeError
                if an input polynomial is not 1D
            cpl.core.IllegalInputError
                if wl_search size is less than 2, nsamples is less than 1, hsize is negative, or 
                wl_search contains a zero search bound.
            cpl.core.DataNotFoundError
                if no model spectra can be created with the calling SlitModel and passed filler
        """
    @property
    def catalog(self) -> cpl.core.Bivector:
        """
            Set the catalog of lines to be used by the spectrum filler
        
            The values in the X-vector must be increasing. The catalog values will be copied into 
            the slitmodel and thus modification of the passed Bivector will not impact the internal
            Slitmodel catalog, and vice versa. 
        """
    @catalog.setter
    def catalog(self, arg1: cpl.core.Bivector) -> None:
        ...
    @property
    def threshold(self) -> float:
        """
            The FWHM of the transfer function to be used by the spectrum filler.
        
            The threshold should be high enough to ensure a good line profile, but 
            not too high to make the spectrum generation too costly. 5 is the CPL recommended.
        """
    @threshold.setter
    def threshold(self, arg1: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def wfwhm(self) -> float:
        """
        Set the FWHM of the transfer function to be used by the spectrum filler.
        """
    @wfwhm.setter
    def wfwhm(self, arg1: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def wslit(self) -> float:
        """
            Slit width to be used by the spectrum filler.
        """
    @wslit.setter
    def wslit(self, arg1: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
