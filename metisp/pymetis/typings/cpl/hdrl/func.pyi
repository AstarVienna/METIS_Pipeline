"""
HDRL Functionalities submodule

  This module provides the Python API for HDRL algorithms including:
  effective airmass, bad-pixel detection, barycentric correction,
  flat fielding, fringing, overscan correction, image resampling, and more.
  
"""
from __future__ import annotations
import cpl.core
import cpl.drs
import cpl.hdrl.core
import numpy
import typing
__all__: list[str] = ['Airmass', 'AirmassApprox', 'BPM', 'BPM2D', 'BPM3D', 'BPMFit', 'Barycorr', 'Catalogue', 'CatalogueResult', 'Collapse', 'Dar', 'DarResult', 'Efficiency', 'EfficiencyParameter', 'EfficiencyResponseParameter', 'Flat', 'FpnResult', 'Fringe', 'FringeCorrectResult', 'LaCosmic', 'Maglim', 'Overscan', 'Resample', 'ResampleMethod', 'ResampleOutgrid', 'ResampleResult', 'Response', 'ResponseCalcParameter', 'ResponseFitParameter', 'ResponseResult', 'ResponseTelluricParameter', 'ResponseVelocityParameter', 'Strehl', 'StrehlResult', 'Window', 'fpn_compute']
class Airmass:
    def __init__(self, ra: tuple, dec: tuple, lst: tuple, exptime: tuple, latitude: tuple, type: AirmassApprox) -> None:
        """
                         Create an Airmass computation object.
        
                         Parameters
                         ----------
                         ra : tuple or None
                             Right Ascension in degrees with error (data, error) or None for (0, 0).
                         dec : tuple or None
                             Declination in degrees with error (data, error) or None for (0, 0).
                         lst : tuple or None
                             Local Sidereal Time in seconds with error (data, error) or None for (0, 0).
                         exptime : tuple or None
                             Exposure time in seconds with error (data, error) or None for (0, 0).
                         latitude : tuple or None
                             Observatory latitude in degrees with error (data, error) or None for (0, 0).
                         type : hdrl.func.AirmassApprox
                             Airmass approximation method (Hardie, YoungIrvine, or Young).
        """
    def compute(self) -> tuple:
        """
                         Compute the effective air mass.
        
                         This function calculates the effective air mass using the specified
                         approximation method. The air mass is computed from the astronomical
                         coordinates (RA, Dec), local sidereal time, exposure time, and
                         observatory latitude.
        
                         Returns
                         -------
                         tuple
                             A tuple containing (airmass_data, airmass_error) where:
                             - airmass_data: The computed air mass value
                             - airmass_error: The error in the air mass calculation
        
                         Notes
                         -----
                         The function uses different approximation methods:
                         - Hardie (1962): Most commonly used method
                         - Young & Irvine (1967): Alternative approximation
                         - Young (1994): More recent approximation
        
                         The calculation takes into account the Earth's atmospheric refraction
                         and the geometric path length through the atmosphere.
        
                         Raises
                         ------
                         hdrlcore.IllegalInputError
                             If any input parameter is invalid (e.g., RA outside [0, 360],
                             Dec outside [-90, 90], latitude outside [-90, 90], etc.).
        """
    @property
    def dec(self) -> tuple:
        ...
    @property
    def exptime(self) -> tuple:
        ...
    @property
    def latitude(self) -> tuple:
        ...
    @property
    def lst(self) -> tuple:
        ...
    @property
    def ra(self) -> tuple:
        ...
    @property
    def type(self) -> AirmassApprox:
        ...
class AirmassApprox:
    """
    Members:
    
      Hardie
    
      YoungIrvine
    
      Young
    """
    Hardie: typing.ClassVar[AirmassApprox]  # value = <AirmassApprox.Hardie: 1>
    Young: typing.ClassVar[AirmassApprox]  # value = <AirmassApprox.Young: 3>
    YoungIrvine: typing.ClassVar[AirmassApprox]  # value = <AirmassApprox.YoungIrvine: 2>
    __members__: typing.ClassVar[dict[str, AirmassApprox]]  # value = {'Hardie': <AirmassApprox.Hardie: 1>, 'YoungIrvine': <AirmassApprox.YoungIrvine: 2>, 'Young': <AirmassApprox.Young: 3>}
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
class BPM:
    """
    
          The hdrl.func.BPM class provides an interface to various Bad Pixel Mask algorithms.
          This module contains static functions to detect bad pixels on single images, on a
          stack of identical images, and on a sequence of images.
          
    """
    @staticmethod
    def filter(input_mask: cpl.core.Mask, kernel_nx: typing.SupportsInt | typing.SupportsIndex, kernel_ny: typing.SupportsInt | typing.SupportsIndex, filter: cpl.core.Filter) -> cpl.core.Mask:
        """
                Sets pixels to bad if the pixel is surrounded by other bad pixels.
                Allows the growing and shrinking of bad pixel masks. 
         
                The algorithm assumes that all pixels outside the mask are good, i.e. it
                enlarges the mask by the kernel size and marks this border as good. It
                applies on the enlarged mask during the operation and extracts the original-size
                mask at the very end.
                
                Parameters
                ----------
                    input_mask : cpl.core.Mask
                        Input mask
                    kernel_nx : int
                        Size in x-direction of the filtering kernel
                    kernel_ny : int
                        Size in y-direction of the filtering kernel
                    filter : cpl.core.Filter mode
                        Filter modes as defined in PyCPL.
                        Supported modes: cpl.core.Filter.EROSION, cpl.core.Filter.DILATION, cpl.core.Filter.OPENING, cpl.core.Filter.CLOSING, cpl.core.Filter.LINEAR
        
                Returns
                -------
                cpl.core.Mask
                    mask of a defined size.
        
                See Also
                ---------
                hdrl.func.BPM.filter_list : Wrapper around hdrl.func.BPM.filter() to filter list of images.
        """
    @staticmethod
    def filter_list(inlist: cpl.core.ImageList, kernel_nx: typing.SupportsInt | typing.SupportsIndex, kernel_ny: typing.SupportsInt | typing.SupportsIndex, filter: cpl.core.Filter) -> cpl.core.ImageList:
        """
                Wrapper around hdrl.func.BPM.filter() to filter list of images
                
                Parameters
                ----------
                    inlist : cpl.core.ImageList
                        Input image list
                    kernel_nx : int
                        Size in x-direction of the filtering kernel
                    kernel_ny : int
                        Size in y-direction of the filtering kernel
                    filter : cpl.core.Filter mode
                        Filter modes as defined in PyCPL.
                        Supported modes: cpl.core.Filter.EROSION, cpl.core.Filter.DILATION, cpl.core.Filter.OPENING, cpl.core.Filter.CLOSING, cpl.core.Filter.LINEAR
        
                Returns
                -------
                cpl.core.ImageList
                    the filtered image list.
                
                See Also
                --------
                hdrl.func.BPM.filter : Sets pixels to bad if the pixel is surrounded by other bad pixels.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
class BPM2D:
    """
    
          The hdrl.func.BPM2D class provides an interface to the bad pixel mask 2D algorithm.
          Bad pixels on single images, on a stack of identical images, and
          on a sequence of images can be detected.
    
          The algorithm first smoothes the image by applying different methods.
          Then it subtracts the smoothed image and derives bad pixels by thresholding
          the residual image, i.e. all pixels exceeding the threshold are considered bad.
    
          In order to create instances of the hdrl.func.BPM2D class, two methods are
          available depending on the smoothing algorithm used, namely hdrl.func.BPM2D.Method.Filter
          and hdrl.func.BPM2D.Method.Legendre.  
          
    """
    class Method:
        """
        The method to be used when creating an instance of the hdrl.func.BPM2D class.
        
        Members:
        
          Legendre : 
                This algorithm will use Legendre smoothing techniques to detect bad pixels. Fitting a Legendre
                polynomial to the image of order `order_x` in x and `order_y` in y direction. This method allows you to
                define `steps_x` :math:`\\times` `steps_y` sampling points (the latter are computed as the median within a box of 
                `filter_size_x` and `filter_size_y`) where the polynomial is fitted. This substantially decreases the 
                fitting time for the Legendre polynomial.
              
        
          Filter : 
                This algorithm will use Filter smoothing techniques to detect bad pixels. Applying a filter
                like a median filter to the image. The filtering can be done by all modes currently supported
                by PyCPL and is controlled by the cpl.core.Filter type `filter`, the cpl.core.Border type `border`, and by the kernel size
                in x and y, i.e. `smooth_x`, and `smooth_y`.
              
        """
        Filter: typing.ClassVar[BPM2D.Method]  # value = <Method.Filter: 1>
        Legendre: typing.ClassVar[BPM2D.Method]  # value = <Method.Legendre: 0>
        __members__: typing.ClassVar[dict[str, BPM2D.Method]]  # value = {'Legendre': <Method.Legendre: 0>, 'Filter': <Method.Filter: 1>}
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
    @staticmethod
    def Filter(kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, maxiter: typing.SupportsInt | typing.SupportsIndex, filter: cpl.core.Filter, border: cpl.core.Border, smooth_x: typing.SupportsInt | typing.SupportsIndex, smooth_y: typing.SupportsInt | typing.SupportsIndex) -> BPM2D:
        """
                Creates an instance of hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Filter
        
               Parameters
               ----------
                   kappa_low : float
                      Low kappa factor for thresholding algorithm
                   kappa_high : float
                      High kappa factor for thresholding algorithm
                   maxiter : int
                      Maximum number of iterations
                   filter : cpl.core.Filter mode
                      Filter mode as defined in PyCPL.
                      Supported modes: cpl.core.Filter.EROSION, cpl.core.Filter.DILATION, cpl.core.Filter.OPENING, cpl.core.Filter.CLOSING, cpl.core.Filter.LINEAR
                   border : cpl.core.Border mode
                      Border mode as defined in PyCPL.
                      Supported modes: cpl.core.Border.FILTER. cpl.core.Border.ZERO, cpl.core.Border.CROP, cpl.core.Border.NOP, cpl.core.Border.COPY
                   smooth_x : int
                      Smoothing kernel size in the x-direction 
                   smooth_y : int
                      Smoothing kernel size in the y-direction
               
               Returns
               -------
               The hdrl.func.BPM2D
                   An instance of the hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Filter.
           
               Example
               -------
               .. code-block:: python
           
                   fs = hdrl.func.BPM2D.Filter(4.0, 5.0, 6, cpl.core.Filter.MEDIAN, cpl.core.Border.NOP, 7, 9)
           
               Notes
               -----
               Filter smooth algorithm: 
               This instance applies a filter like e.g. a median filter to the image. The filtering can be
               done by all modes currently supported by CPL and is controlled by the filter type, the border type
               and the kernel size in x and y.
               
               See Also
               --------
               hdrl.func.BPM2D.Legendre : Creates an instance of hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Legendre.
               hdrl.func.BPM2D.compute : Detect bad pixels on a single image with an iterative process.
        """
    @staticmethod
    def Legendre(kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, maxiter: typing.SupportsInt | typing.SupportsIndex, steps_x: typing.SupportsInt | typing.SupportsIndex, steps_y: typing.SupportsInt | typing.SupportsIndex, filter_size_x: typing.SupportsInt | typing.SupportsIndex, filter_size_y: typing.SupportsInt | typing.SupportsIndex, order_x: typing.SupportsInt | typing.SupportsIndex, order_y: typing.SupportsInt | typing.SupportsIndex) -> BPM2D:
        """
              Creates an instance of hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Legendre.
        
              Parameters
              ----------
              kappa_low : float
                  Low kappa factor for the thresholding algorithm
              kappa_high : float
                  High kappa factor for the thresholding algorithm
              maxiter : int
                  Maximum number of iterations
              steps_x : int
                  Number of sampling points in the x-direction
              steps_y : int
                  Number of sampling points in the y-direction
              filter_size_x : int
                  Size of the median box in the x-direction
              filter_size_y : int
                  Size of the median box in the y-direction
              order_x : int
                  Order of polynomial in the x-direction
              order_y : int
                  Order of polynomial in the y-direction
          
              Returns
              -------
              The hdrl.func.BPM2D
                  An instance of the hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Legendre.
          
              Example
              -------
              .. code-block:: python
          
                  ls = hdrl.func.BPM2D.Legendre(4, 5, 6, 20, 21, 11, 12, 2, 10)
          
              Notes
              ----
              This instance of a hdrl.func.BPM2D class fits a Legendre polynomial to the image of order order_x,
              in x and order_y in y direction. This method allows you to define steps_x and steps_y
              sampling points (the latter are computed as the median within a box of filter_size_x
              and filter_size_y) where the polynomial is fitted. This substantially decreases the
              fitting time for the Legendre polynomial. 
              
              See Also
              --------
              hdrl.func.BPM2D.compute : Detect bad pixels on a single image with an iterative process.
              hdrl.func.BPM2D.Filter : Creates an instance of hdrl.func.BPM2D for the method hdrl.func.BPM2D.Method.Filter.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, img_in: cpl.hdrl.core.Image) -> cpl.core.Mask:
        """
                Detects bad pixels on a single image with an iterative process
        
                Parameters
                ----------
                img_in : hdrl.core.Image
                    Input image
          
                Returns
                -------
                cpl.core.Mask
                    Bad pixel mask with the newly found bad pixels.
        
                Notes
                -----
                The algorithm first smoothes the image by applying the methods
                described under hdrl.func.BPM2D.Filter or hdrl.func.BPM2D.Legendre. 
                Then it subtracts the smoothed image and derives bad pixels by 
                thresholding the residual image, i.e. all pixels exceeding
                the threshold are considered bad. To compute the upper and lower
                thresholds, it measures a robust rms (a properly scaled Median Absolute
                Deviation), which is then scaled by the parameters  kappa_low and 
                kappa_high. Furthermore, the algorithm is applied iteratively
                controlled by  maxiter. During each iteration, the newly found bad
                pixels are ignored. Please note that the thresholding values are
                applied as median (residual-image) :math:`\\pm` thresholds. This makes the
                algorithm more robust in the case that the methods are
                not able to completely remove the background level, e.g due to an
                exceeding number of bad pixels in the first iteration.
                
                See Also
                --------
                hdrl.func.BPM2D.Filter : Creates an instance of hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Filter.
                hdrl.func.BPM2D.Legendre : Creates an instance of hdrl.func.BPM2D class for the method hdrl.func.BPM2D.Method.Legendre.
        """
    @property
    def border(self) -> cpl.core.Border:
        """
        cpl.core.Border : border mode
        """
    @property
    def filter(self) -> cpl.core.Filter:
        """
        cpl.core.Filter : filter mode
        """
    @property
    def filter_size_x(self) -> int:
        """
        int : Size of the median box in the x-direction
        """
    @property
    def filter_size_y(self) -> int:
        """
        int : Size of the median box in the y-direction
        """
    @property
    def kappa_high(self) -> float:
        """
        float : High kappa factor for thresholding algorithm
        """
    @property
    def kappa_low(self) -> float:
        """
        float : Low kappa factor for thresholding algorithm
        """
    @property
    def maxiter(self) -> int:
        """
        int : Maximum number of iterations
        """
    @property
    def method(self) -> hdrl_bpm_2d_method:
        """
        hdrl.func.BPM2D.Method : Selected algorithm method
        """
    @property
    def order_x(self) -> int:
        """
        int : Order of polynomial in the x-direction
        """
    @property
    def order_y(self) -> int:
        """
        int : Order of polynomial in the y-direction
        """
    @property
    def smooth_x(self) -> int:
        """
        int : Smoothing kernel size in the x-direction
        """
    @property
    def smooth_y(self) -> int:
        """
        int : Smoothing kernel size in the y-direction
        """
    @property
    def steps_x(self) -> int:
        """
        int : Number of sampling points in the x-direction
        """
    @property
    def steps_y(self) -> int:
        """
        int : Number of sampling points in the y-direction
        """
class BPM3D:
    """
    
          The hdrl.func.BPM3D class provides an interface to the bad pixel mask 3D algorithm.
          This algorithm detects bad pixels on a stack of identical images in an imagelist.
          Once the class is instantiated with the desired parameters, the compute function 
          may be called to run the algorithm.
    
    
          Parameters
          ----------
          kappa_low : float
              Low kappa factor for thresholding algorithm
          kappa_high : float
              High kappa factor for thresholding algorithm
          method : hdrl.func.BPM3D.Method
              Selected algorithm method
      
          Returns
          -------
          hdrl.func.BPM3D
    
          Notes
          -----
          There are three algorithm methods to be selected from: 
    
          - hdrl.func.BPM3D.Method.Absolute: It uses `kappa_low` and `kappa_high` as absolute threshold.
          - hdrl.func.BPM3D.Method.Relative: It scales the measured rms on the residual-image with
            `kappa_low` and `kappa_high` and uses it as threshold. For the rms a properly scaled Median
            Absolute Deviation (MAD) is used.
          - hdrl.func.BPM3D.Method.Error: It scales the propagated error of each individual
            pixel with `kappa_low` and `kappa_high` and uses it as threshold.
          
          Example
          -------
          .. code-block:: python
      
            bpm_3d = hdrl.func.BPM3D(4, 5, hdrl.func.BPM3D.Method.Absolute)
            bpm_3d = hdrl.func.BPM3D(4, 5, hdrl.func.BPM3D.Method.Relative)
            bpm_3d = hdrl.func.BPM3D(4, 5, hdrl.func.BPM3D.Method.Error)
      
          See Also
          --------
          hdrl.func.BPM3D.compute : Detect bad pixels on a stack of identical images.
          
          
    """
    class Method:
        """
        The method to be used when creating an instance of the hdrl.func.BPM3D class.
        
        Members:
        
          Absolute : 
                Uses kappa_low and kappa_high as absolute thresholds.
              
        
          Relative : 
                Scales the measured RMS on the residual image with
                kappa_low and kappa_high and uses it as thresholds. For the rms a properly
                scaled Median Absolute Deviation (MAD) is used.
              
        
          Error : 
                Scales the propagated error of each individual
                pixel with kappa_low and kappa_high and uses it as a threshold.
              
        """
        Absolute: typing.ClassVar[BPM3D.Method]  # value = <Method.Absolute: 0>
        Error: typing.ClassVar[BPM3D.Method]  # value = <Method.Error: 2>
        Relative: typing.ClassVar[BPM3D.Method]  # value = <Method.Relative: 1>
        __members__: typing.ClassVar[dict[str, BPM3D.Method]]  # value = {'Absolute': <Method.Absolute: 0>, 'Relative': <Method.Relative: 1>, 'Error': <Method.Error: 2>}
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
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self, kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, method: hdrl_bpm_3d_method) -> None:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, imglist_in: cpl.hdrl.core.ImageList) -> cpl.core.ImageList:
        """
                Detects bad pixels on a stack of identical images.
                
                Parameters
                ----------
                imglist : hdrl.core.ImageList
                    input imagelist.
        
                Returns
                -------
                cpl.core.ImageList
                    imagelist containing the newly found bad pixels for each input image with the same pixel
                    coding as for a cpl.core.Mask bad pixel mask, i.e. 0 for good pixels and 1 for bad pixels.
                    Please note that already known bad pixels given to the routine will not be included in
                    the output mask.
        
                Notes
                -----
                The algorithm first collapses the stack of images by using the median in order
                to generate a master image. Then it subtracts the master image from each individual
                image and derives the bad pixels on the residual images by thresholding, i.e. all
                pixels exceeding the threshold are considered bad. 
                
                Please note that the algorithm assumes that the mean level of the different images
                is the same. If this is not the case, then the master image as described above 
                will be biased.
        
                See Also
                --------
                hdrl.func.BPM3D : Creates an instance of the hdrl.func.BPM3D class for the imagelist method.
        """
    @property
    def kappa_high(self) -> float:
        """
        float : High kappa factor for thresholding algorithm
        """
    @property
    def kappa_low(self) -> float:
        """
        float : Low kappa factor for thresholding algorithm
        """
    @property
    def method(self) -> hdrl_bpm_3d_method:
        """
        hdrl.func.BPM3D.Method : Selected algorithm method
        """
class BPMFit:
    """
    
          The hdrl.func.BPMFit module provides an interface to various bad pixel mask fit algorithms
          to detect bad pixels on a sequence of images e.g. domeflats.
    
          The algorithm fits a polynomial to each pixel sequence and determines bad
          pixels based on this fit and various thresholding methods.
    
          Three thresholding methods are available to convert the information from the fit into a bad pixel map:
    
            - PVal: Pixels with p-values below the mentioned threshold are considered as bad pixels.
            - RelChi: Pixels with a chi value below the mentioned threshold are considered as bad pixels.
            - RelCoef: Pixels with a fit coefficient below the mentioned threshold are considered as bad pixels.
    
            In order to create an instance of the hdrl.func.BPMFit class, three constructors
            corresponding to the above methods are available. 
          
    """
    @staticmethod
    def PVal(degree: typing.SupportsInt | typing.SupportsIndex, pval: typing.SupportsFloat | typing.SupportsIndex) -> BPMFit:
        """
                Creates an instance of hdrl.func.BPMFit class for the p-value method.
        
                Parameters
                ----------
                    degree : int
                        The degree of the fit
                    pval : float
                        The p-value bpm cutoff
        
                Notes
                -----
                Pixels with low p-value. When the errors of the pixels are correct the p-value can
                be interpreted as the probability with which the pixel response fits the chosen model.
        
                See Also
                --------
                hdrl.func.BPMFit.compute : Derives bad pixels on a sequence of images by fitting a polynomial along each pixel sequence of the images. 
                hdrl.func.BPMFit.RelChi : Creates an instance of hdrl.func.BPMFit class with relative chi bpm threshold.
                hdrl.func.BPMFit.RelCoef : Creates an instance of hdrl.func.BPMFit class with relative coefficient bpm threshold.
        """
    @staticmethod
    def RelChi(degree: typing.SupportsInt | typing.SupportsIndex, low: typing.SupportsFloat | typing.SupportsIndex, high: typing.SupportsFloat | typing.SupportsIndex) -> BPMFit:
        """
                Creates an instance of hdrl.func.BPMFit class with a relative chi bpm threshold.
        
                Parameters
                ----------
                    degree : int
                        The degree of the fit
                    low : float
                        Relative chi distribution bpm lower threshold
                    high : float
                        Relative chi distribution bpm upper threshold
                
                Returns
                -------
                hdrl.func.BPMFit
        
                Notes
                -----
                Relative cutoff on the chi distribution of all fits. Pixels with chi values which
                exceed mean :math:`\\pm` cutoff :math:`\\times` standard deviation are considered bad.
        
                See Also
                --------
                hdrl.func.BPMFit.compute : Derives bad pixels on a sequence of images by fitting a polynomial along each pixel sequence of the images. 
                hdrl.func.BPMFit.PVal : Creates an instance of hdrl.func.BPMFit class with p-value bpm threshold.
                hdrl.func.BPMFit.RelCoef : Creates an instance of hdrl.func.BPMFit class with relative coefficient bpm threshold.
        """
    @staticmethod
    def RelCoef(degree: typing.SupportsInt | typing.SupportsIndex, low: typing.SupportsFloat | typing.SupportsIndex, high: typing.SupportsFloat | typing.SupportsIndex) -> BPMFit:
        """
                Creates an instance of hdrl.func.BPMFit class with relative coefficient bpm threshold.
        
                Parameters
                ----------
                    degree : int
                        The degree of the fit
                    low : float
                        Relative fit coefficient distribution bpm lower threshold
                    high : float
                        Relative fit coefficient distribution bpm upper threshold
                
                Returns
                -------
                hdrl.func.BPMFit
        
                Notes
                -----
                Relative cutoff on the distribution of the fit coefficients. Pixels with fit coefficients which
                exceed mean :math:`\\pm` cutoff :math:`\\times` standard deviation are considered bad.
        
                See Also
                --------
                hdrl.func.BPMFit.compute : Derives bad pixels on a sequence of images by fitting a polynomial along each pixel sequence of the images. 
                hdrl.func.BPMFit.PVal : Creates an instance of hdrl.func.BPMFit class with p-value bpm threshold.
                hdrl.func.BPMFit.RelChi : Creates an instance of hdrl.func.BPMFit class with relative chi bpm threshold.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, imglist_in: cpl.hdrl.core.ImageList, sample_position: cpl.core.Vector) -> cpl.core.Image:
        """
                Derives bad pixels on a sequence of images by fitting a polynomial 
                along each pixel sequence of the images.
                
                Parameters
                ----------
                    imglist_in : hdrl.core.ImageList
                        imagelist to fit
                    sample_position : cpl.core.Vector
                        vector of sampling position of the images in data, e.g. exposure time
                  
                Returns
                -------
                cpl.core.Image
                    image with cpl.core.Type.INT pixels containing the 
                    newly found bad pixels for each input image
        
                Notes
                -----
                When using hdrl.func.BPMFit.RelCoef the value of the returned image encodes 
                the coefficient that was outside the relative threshold as a power of two. 
                e.g. if coefficient 0 and 3 of the fit were not within the threshold for a pixel, 
                it will have the value :math:`2^0 + 2^3 = 9`. The other hdrl.func.BPMFit algorithms 
                return an image with non-zero values marking pixels outside the selection thresholds.
        
                Please note that already known bad pixels given to the routine will not be included in the output mask.
        
                See Also
                --------
                hdrl.func.BPMFit.PVal : Creates an instance of hdrl.func.BPMFit class with p-value bpm threshold.
                hdrl.func.BPMFit.RelChi : Creates an instance of hdrl.func.BPMFit class with relative chi bpm threshold.
                hdrl.func.BPMFit.RelCoef : Creates an instance of hdrl.func.BPMFit class with relative coefficient bpm threshold.
        """
    @property
    def chi_high(self) -> float:
        """
        float : Relative chi distribution bpm upper threshold
        """
    @property
    def chi_low(self) -> float:
        """
        float : Relative chi distribution bpm lower threshold
        """
    @property
    def coef_high(self) -> float:
        """
        float : Relative fit coefficient distribution bpm upper threshold
        """
    @property
    def coef_low(self) -> float:
        """
        float : Relative fit coefficient distribution bpm lower threshold
        """
    @property
    def degree(self) -> int:
        """
        int : The degree of the fit
        """
    @property
    def pval(self) -> float:
        """
        float: The p-value bpm cutoff
        """
class Barycorr:
    @staticmethod
    def compute(target: tuple[typing.SupportsFloat | typing.SupportsIndex, typing.SupportsFloat | typing.SupportsIndex], observer: tuple[typing.SupportsFloat | typing.SupportsIndex, typing.SupportsFloat | typing.SupportsIndex, typing.SupportsFloat | typing.SupportsIndex], eop_table: cpl.core.Table, mjd_obs: typing.SupportsFloat | typing.SupportsIndex, time_to_mid_exposure: typing.SupportsFloat | typing.SupportsIndex, pressure: typing.SupportsFloat | typing.SupportsIndex = 0.0, temperature: typing.SupportsFloat | typing.SupportsIndex = 0.0, humidity: typing.SupportsFloat | typing.SupportsIndex = 0.0, wavelength: typing.SupportsFloat | typing.SupportsIndex = 0.0) -> float:
        """
                    Compute the barycentric correction for an observation, using the ERFA function
                    eraApco13().
        
                    Parameters
                    ----------
                    target : tuple
                        A tuple (ra, dec) in degrees.
                    observer : tuple
                        A tuple (lat, lon, height) where latitude and longitude are in degrees,
                        and height is in meters.
                    eop_table : cpl.core.Table
                        The Earth Orientation Parameter (EOP) table.
                    mjd_obs : float
                        Modified Julian Date of the observation.
                    time_to_mid_exposure : float
                        Time to mid exposure in seconds.
                    pressure : float, optional
                        Atmospheric pressure in hPa (default: 0.0).
                    temperature : float, optional
                        Ambient temperature in degrees Celsius (default: 0.0).
                    humidity : float, optional
                        Relative humidity (0 to 1, default: 0.0).
                    wavelength : float, optional
                        Observing wavelength in micrometers (default: 0.0).
        
                    Returns
                    -------
                    float
                        Computed barycentric correction in m/s.
        """
    def __init__(self) -> None:
        """
                   The hdrl.func.Barycorr class provides an interface to 
                   calculation of the barycentric correction.
        """
class Catalogue:
    """
    
          A hdrl.func.Catalogue class provides an interface to object detection and 
          catalogue generation from astronomical images. 
          
    """
    def __init__(self, obj_min_pixels: typing.SupportsInt | typing.SupportsIndex, obj_threshold: typing.SupportsFloat | typing.SupportsIndex, obj_deblending: bool, obj_core_radius: typing.SupportsFloat | typing.SupportsIndex, bkg_estimate: bool, bkg_mesh_size: typing.SupportsInt | typing.SupportsIndex, bkg_smooth_fwhm: typing.SupportsFloat | typing.SupportsIndex, det_eff_gain: typing.SupportsFloat | typing.SupportsIndex, det_saturation: typing.SupportsFloat | typing.SupportsIndex, resulttype: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Create a Catalogue computation object for object detection and catalogue generation.
              
              This class provides functionality to detect objects in astronomical images,
              perform background estimation, and generate catalogues with photometric and
              astrometric measurements.
              
              Parameters
              ----------
              obj_min_pixels : int
                  Minimum pixel area for each detected object. Objects smaller than this
                  will be rejected from the catalogue.
              obj_threshold : float
                  Detection threshold in sigma above sky. Objects below this threshold
                  will not be detected.
              obj_deblending : bool
                  Use deblending algorithm to separate overlapping objects.
              obj_core_radius : float
                  Value of Rcore in pixels for object detection.
              bkg_estimate : bool
                  Estimate background from input image. If False, it is assumed the input
                  is already background corrected with median 0.
              bkg_mesh_size : int
                  Background smoothing box size in pixels.
              bkg_smooth_fwhm : float
                  The FWHM of the Gaussian kernel used in convolution for object detection.
              det_eff_gain : float
                  Detector gain value to rescale and convert intensity to electrons.
              det_saturation : float
                  Detector saturation value in ADU.
              resulttype : int
                  Requested output type using bitwise flags:
                  - 1: Background image only
                  - 2: Segmentation map only  
                  - 4: Complete catalogue only
                  - 7: All outputs (background, segmentation map, and catalogue)
                  
              Example
              -------
              .. code-block:: python
                  
                  # Create catalogue object for basic object detection
                  cat = hdrl.func.Catalogue(
                      obj_min_pixels=10,
                      obj_threshold=3.0,
                      obj_deblending=True,
                      obj_core_radius=2.0,
                      bkg_estimate=True,
                      bkg_mesh_size=64,
                      bkg_smooth_fwhm=2.0,
                      det_eff_gain=1.0,
                      det_saturation=65535.0,
                      resulttype=7  # All outputs
                  )
                  
              Notes
              -----
              The algorithm performs local sky background estimation and removal,
              detects objects and blends, assigns image pixels to each object,
              and performs astrometry, photometry and shape analysis on the
              detected objects.
        """
    def compute(self, image: cpl.core.Image, confidence_map: cpl.core.Image = None, wcs: cpl.drs.WCS = None) -> CatalogueResult:
        """
              Compute object catalogue from an astronomical image.
              
              This function builds an object catalogue from the input image. The algorithm
              performs local sky background estimation and removal, detects objects and blends,
              assigns image pixels to each object, and performs astrometry, photometry and
              shape analysis on the detected objects.
              
              Parameters
              ----------
              image : cpl.core.Image
                  Input image for catalogue computation. The image should be in
                  units of ADU (Analog-to-Digital Units).
              confidence_map : cpl.core.Image, optional
                  Optional confidence map providing uncertainty information for each pixel.
                  If None, no confidence map is used. Must contain only positive numbers
                  if provided.
              wcs : cpl.core.WCS, optional
                  Optional WCS information for astrometric measurements. If None,
                  no WCS information is used and astrometry will not be performed.
              
              Returns
              -------
              hdrl.func.CatalogueResult
                  A result object containing:
                  - catalogue: cpl.core.Table with the object catalogue including positions, fluxes, and shape parameters
                  - segmentation_map: cpl.core.Image with the segmentation map showing object assignments
                  - background: cpl.core.Image with the estimated background
                  - qclist: cpl.core.PropertyList with quality control information
              
              Example
              -------
              .. code-block:: python
                  
                  # Load image and WCS
                  image = cpl.core.Image.load("science_image.fits")
                  wcs = cpl.drs.WCS(cpl.core.PropertyList.load("science_image.fits"))
                  
                  # Create catalogue object
                  cat = hdrl.func.Catalogue(
                      obj_min_pixels=10,
                      obj_threshold=3.0,
                      obj_deblending=True,
                      obj_core_radius=2.0,
                      bkg_estimate=True,
                      bkg_mesh_size=64,
                      bkg_smooth_fwhm=2.0,
                      det_eff_gain=1.0,
                      det_saturation=65535.0,
                      resulttype=7
                  )
                  
                  # Compute catalogue
                  result = cat.compute(image, wcs=wcs)
                  
                  # Access results
                  catalogue = result.catalogue
                  segmap = result.segmentation_map
                  background = result.background
                  qc_info = result.qclist
                  
              Notes
              -----
              The confidence_map must contain only positive numbers if provided.
              
              The function automatically handles image type conversion to double precision
              if needed.
              
              Raises
              ------
              hdrlcore.NullInputError
                  If image is None.
              hdrlcore.IllegalInputError
                  If parameter validation fails (e.g., invalid parameter values).
              hdrlcore.IncompatibleInputError
                  If confidence_map contains negative values or other incompatible inputs.
        """
    @property
    def bkg_estimate(self) -> bool:
        """
        bool : Estimate background from input
        """
    @property
    def bkg_mesh_size(self) -> int:
        """
        int : Background smoothing box size
        """
    @property
    def bkg_smooth_fwhm(self) -> float:
        """
        float : FWHM of Gaussian kernel for object detection
        """
    @property
    def det_eff_gain(self) -> float:
        """
        float : Detector gain value
        """
    @property
    def det_saturation(self) -> float:
        """
        float : Detector saturation value
        """
    @property
    def obj_core_radius(self) -> float:
        """
        float : Value of Rcore in pixels
        """
    @property
    def obj_deblending(self) -> bool:
        """
        bool : Use deblending algorithm
        """
    @property
    def obj_min_pixels(self) -> int:
        """
        int : Minimum pixel area for each detected object
        """
    @property
    def obj_threshold(self) -> float:
        """
        float : Detection threshold in sigma above sky
        """
    @property
    def resulttype(self) -> int:
        """
        int : Requested output type
        """
class CatalogueResult:
    """
    
          A hdrl.func.CatalogueResult class is a container for the results of hdrl.func.Catalogue.compute().
          The results consist of a catalogue table, segmentation map, background image, and quality control information.
          
          These can be accessed via the catalogue, segmentation_map, background, and qclist attributes of the object.
          
          Example
          -------
          .. code-block:: python
              
              result = hdrl.func.Catalogue.compute(image, wcs=wcs)
              catalogue = result.catalogue
              segmap = result.segmentation_map
              background = result.background
              qc_info = result.qclist
          
    """
    def __repr__(self) -> str:
        ...
    @property
    def background(self) -> cpl.core.Image:
        """
        cpl.core.Image : Estimated background image
        """
    @property
    def catalogue(self) -> cpl.core.Table:
        """
        cpl.core.Table : Object catalogue with positions, fluxes, and shape parameters
        """
    @property
    def qclist(self) -> cpl.core.PropertyList:
        """
        cpl.core.PropertyList : Quality control information
        """
    @property
    def segmentation_map(self) -> cpl.core.Image:
        """
        cpl.core.Image : Segmentation map showing object assignments
        """
class Collapse:
    """
    
          A hdrl.func.Collapse is a helper class consisting of static functions that conduct Collapse operations like sum,
          mean, or standard deviation with an hdrl.core.Imagelist. hdrl.func.Collapse class cannot be instantiated on its own,
          but only via one of the following constructors:
          - hdrl.func.Collapse.Mean: Mean Collapse operation on HDRL ImageList.
          - hdrl.func.Collapse.Median: Median Collapse operation on HDRL ImageList.
          - hdrl.func.Collapse.SigClip: Sigma Clipped Collapse operation on HDRL ImageList.
          - hdrl.func.Collapse.MinMax: Min-Max Clipped  Collapse operation on HDRL ImageList.
          - hdrl.func.Collapse.Mode: Mode Collapse operation on HDRL ImageList.
    
          Once the constructors are set up with the desired parameters, you can call  hdrl.core.Collapse.compute() or pass it to
          a function that accepts the Collapse instance (e.g. hdrl.func.Flat.compute).
    
          Notes
          ----------
          Sigma Clipping or Median Collapse methods are recommended for robust application against outliers (e.g. cosmic ray
          hits). However, Median has a low statistical efficiency, so will display higher uncertainty than images collapsed using
          sigma clipping with few outliers.
          
    """
    class Method:
        """
        Method to use for the mode computation.
        
        Members:
        
          Median : 
                This method is the most robust method and can/should be used for very asymmetric point distributions.
                
        
          Fit : 
                This method gives accurate results for symmetric distributions but is also more likely to fail.
                
        
          Weighted : 
                This method can/should be used with asymmetric distributions.
                
        """
        Fit: typing.ClassVar[Collapse.Method]  # value = <Method.Fit: 2>
        Median: typing.ClassVar[Collapse.Method]  # value = <Method.Median: 0>
        Weighted: typing.ClassVar[Collapse.Method]  # value = <Method.Weighted: 1>
        __members__: typing.ClassVar[dict[str, Collapse.Method]]  # value = {'Median': <Method.Median: 0>, 'Fit': <Method.Fit: 2>, 'Weighted': <Method.Weighted: 1>}
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
    @staticmethod
    def Mean() -> Collapse:
        """
                Creates an instance of hdrl.func.Collapse class with Mean parameters.
        
                Mean and associated error are computed with standard formulae.
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                ----------
                hdrl.core.ImageList.collapse : Perform Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_mean : Perform Mean Collapse operation on HDRL ImageList.
        """
    @staticmethod
    def Median() -> Collapse:
        """
                The median collapse of an ImageList to a single image.
        
                Median and associated error are computed with standard formulae.
        
                Returns
                --------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image,
                        one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                ----------
                hdrl.core.ImageList.collapse_median : Perform Median Collapse operation on HDRL ImageList.
                hdrl.func.Collapse.compute : Perform Collapse function on an HDRL ImageList to create one HDRL Image.
        """
    @staticmethod
    def MinMax(nlow: typing.SupportsFloat | typing.SupportsIndex, nhigh: typing.SupportsFloat | typing.SupportsIndex) -> Collapse:
        """
                Creates an instance of hdrl.func.Collapse class with Min-Max clipped parameters.
        
                Minmax-clipped mean and associated error, computed similarly as for mean but without taking the clipped values
                into account
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has four components- one hdrl.core.Image (out), containing the collapsed image,
                        one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image, one cpl.core.Image (reject_low) containing low rejection threshold,
                        one cpl.core.Image (reject_high) containing high rejection threshold.
        
                See Also
                ----------
                hdrl.core.ImageList.collapse : Perform Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_minmax : Perform MinMax Collapse operation on HDRL ImageList.
        """
    @staticmethod
    def Mode(histo_min: typing.SupportsFloat | typing.SupportsIndex, histo_max: typing.SupportsFloat | typing.SupportsIndex, bin_size: typing.SupportsFloat | typing.SupportsIndex, mode_method: Collapse.Method, error_niter: typing.SupportsInt | typing.SupportsIndex) -> Collapse:
        """
                Creates an instance of hdrl.func.Collapse class with Mode parameters.
        
                Parameters
                ----------
                histo_min : float
                    minimum value of low pixels to use
                histo_max : float
                    maximum value of high pixels to be use
                bin_size : float
                    size of the histogram bin
                mode_method : hdrl.func.Collapse.Mode
                    mode_method to use for the mode computation
                error_niter : int
                    size of the histogram bin
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                ----------
                hdrl.core.ImageList.collapse : Perform Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_mode : Perform Mode Collapse operation on HDRL ImageList.
        """
    @staticmethod
    def Sigclip(kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, niter: typing.SupportsInt | typing.SupportsIndex) -> Collapse:
        """
                The sigma clipped collapse of an ImageList to a single image.
        
                Sigma-clipped mean and associated error, computed similarly as for mean but
                without taking the clipped values into account.
        
                Parameters
                ----------
                kappa_low : float
                    low sigma bound
                kappa_high : float
                    high sigma bound
                niter : int
                    number of clipping iterators
        
                Returns
                --------
                     namedtuple
                        The namedtuple has four components- one hdrl.core.Image (out), containing the collapsed image,
                        one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image, one cpl.core.Image (reject_low) containing low rejection threshold,
                        one cpl.core.Image (reject_high) containing high rejection threshold.
        
                See Also
                ----------
                hdrl.core.ImageList.collapse_sigclip : Perform Sigma Clipped Collapse operation on HDRL ImageList.
                hdrl.func.Collapse.compute : Perform Collapse function on an HDRL ImageList to create one HDRL Image.
        """
    @staticmethod
    def WeightedMean() -> Collapse:
        """
                Creates an instance of hdrl.func.Collapse class with Weighted Mean parameters.
        
                Weighted mean and associated error are computed with standard formulae
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                ---------
                hdrl.core.ImageList.collapse : Perform Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_weighted_mean : Perform Weighted Mean Collapse operation on HDRL ImageList.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, himlist: cpl.hdrl.core.ImageList) -> typing.Any:
        """
                Implement the Collapse operations for an HDRL Imagelist to a single image.
        
                Parameters
                ----------
                    himlist : hdrl.core.ImageList
                        Image to collapse using the operation defined in `self`.
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                Notes
                ----------
                These operations can also be accessed directly by the hdrl.core.ImageList objects.
        
                See Also
                --------
                hdrl.core.ImageList.collapse : Perform Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_mean : Perform Mean Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_weighted_mean : Perform Weighted Mean Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_sigclip : Perform Sigma Clipped Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_minmax : Perform Min-max clipped Collapse operation on HDRL ImageList.
                hdrl.core.ImageList.collapse_mode : Perform Mode Collapse operation on HDRL ImageList.
        """
    @property
    def bin_size(self) -> float:
        ...
    @property
    def error_niter(self) -> int:
        ...
    @property
    def histo_max(self) -> float:
        ...
    @property
    def histo_min(self) -> float:
        ...
    @property
    def kappa_high(self) -> float:
        ...
    @property
    def kappa_low(self) -> float:
        ...
    @property
    def method(self) -> Collapse.Method:
        ...
    @property
    def nhigh(self) -> float:
        ...
    @property
    def niter(self) -> int:
        ...
    @property
    def nlow(self) -> float:
        ...
class Dar:
    def __init__(self, airmass: tuple, parang: tuple, posang: tuple, temp: tuple, rhum: tuple, pres: tuple, wcs: cpl.drs.WCS) -> None:
        """
                         Create a DAR (Differential Atmospheric Refraction) computation object.
                         
                         Parameters
                         ----------
                         airmass : tuple or None
                             Air mass value with error (data, error) or None for (0, 0).
                         parang : tuple or None
                             Parallactic angle during exposure in degrees with error (data, error) or None for (0, 0).
                         posang : tuple or None
                             Position angle on the sky in degrees with error (data, error) or None for (0, 0).
                         temp : tuple or None
                             Temperature in Celsius with error (data, error) or None for (0, 0).
                         rhum : tuple or None
                             Relative humidity in percent with error (data, error) or None for (0, 0).
                         pres : tuple or None
                             Pressure in mbar with error (data, error) or None for (0, 0).
                         wcs : cpl.core.WCS or None
                             World Coordinate system (WCS) in degrees or None.
        """
    def compute(self, lambdaRef: tuple, lambdaIn: cpl.core.Vector) -> DarResult:
        """
                         Compute differential atmospheric refraction corrections.
                         
                         This function corrects the pixel coordinates of all pixels of a given pixel table
                         for differential atmospheric refraction (DAR). The algorithm computes the DAR offset
                         for the wavelength difference with respect to the reference wavelength, and stores
                         the shift in the coordinates, taking into account the instrument rotation angle on
                         the sky and the parallactic angle at the time of the observations.
                         
                         Parameters
                         ----------
                         lambdaRef : tuple or None
                             Reference wavelength in Angstroms with error (data, error) or None for (0, 0).
                         lambdaIn : cpl.core.Vector
                             One lambda for each plane (in Angstroms).
                         
                         Returns
                         -------
                         DarResult
                             A result object containing:
                             - xShift: cpl.core.Vector, correction for each plane in x-axis (pixels)
                             - yShift: cpl.core.Vector, correction for each plane in y-axis (pixels)
                             - xShiftErr: cpl.core.Vector, error in correction for each plane in x-axis (pixels)
                             - yShiftErr: cpl.core.Vector, error in correction for each plane in y-axis (pixels)
                         
                         Notes
                         -----
                         The resulting correction can be directly applied to the pixel table.
                         
                         The algorithm is based on Filippenko (1982, PASP, 94, 715) and uses the formula
                         from Owens which converts relative humidity to water vapor pressure.
                         
                         The calculation is performed by calling the top-level function hdrl_dar_compute()
                         and the parameters passed to this function can be created by calling the constructor.
                         
                         Raises
                         ------
                         hdrlcore.NullInputError
                             If any required parameter is None.
                         hdrlcore.IllegalInputError
                             If parameter validation fails (e.g., invalid parameter values).
                         hdrlcore.IncompatibleInputError
                             If inputs are incompatible.
        """
    @property
    def airmass(self) -> tuple:
        ...
    @property
    def parang(self) -> tuple:
        ...
    @property
    def posang(self) -> tuple:
        ...
    @property
    def pres(self) -> tuple:
        ...
    @property
    def rhum(self) -> tuple:
        ...
    @property
    def temp(self) -> tuple:
        ...
    @property
    def wcs(self) -> cpl.drs.WCS:
        ...
class DarResult:
    def __repr__(self) -> str:
        ...
    @property
    def xShift(self) -> cpl.core.Vector:
        ...
    @property
    def xShiftErr(self) -> cpl.core.Vector:
        ...
    @property
    def yShift(self) -> cpl.core.Vector:
        ...
    @property
    def yShiftErr(self) -> cpl.core.Vector:
        ...
class Efficiency:
    @staticmethod
    def compute(I_std_arg: cpl.hdrl.core.Spectrum1D, I_std_ref: cpl.hdrl.core.Spectrum1D, E_x: cpl.hdrl.core.Spectrum1D, pars: EfficiencyParameter) -> cpl.hdrl.core.Spectrum1D:
        """
                  Compute HDRL efficiency.
        
                  Parameters
                  ----------
                  I_std_arg : hdrl.core.Spectrum1D
                      Std star observed spectrum, wavelength in [nm].
                  I_std_ref : hdrl.core.Spectrum1D
                      Std start model spectrum, wavelength in [nm].
                  E_x : hdrl.core.Spectrum1D
                      Atm. extinction model spectrum, wavelength in [nm].
                  pars : hdrl.func.EfficiencyParameter
                      Parameters.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      Efficiency.
        """
    @staticmethod
    def compute_response_core(I_std_arg: cpl.hdrl.core.Spectrum1D, I_std_ref: cpl.hdrl.core.Spectrum1D, E_x: cpl.hdrl.core.Spectrum1D, pars: EfficiencyResponseParameter) -> cpl.hdrl.core.Spectrum1D:
        """
                  Compute HDRL response core.
        
                  Parameters
                  ----------
                  I_std_arg : hdrl.core.Spectrum1D
                      Std star observed spectrum, wavelength in [nm].
                  I_std_ref : hdrl.core.Spectrum1D
                      Std start model spectrum, wavelength in [nm].
                  E_x : hdrl.core.Spectrum1D
                      Atm. extinction model spectrum, wavelength in [nm].
                  pars : hdrl.func.EfficiencyParameter
                      Parameters.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      Response.
        """
    @staticmethod
    def create_parameter(Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex, Atel: typing.SupportsFloat | typing.SupportsIndex) -> EfficiencyParameter:
        """
                  Create an HDRL efficiency parameter.
        
                  Parameters
                  ----------
                  Ap : float
                      Parameter to indicate if the efficiency is computed at
                      airmass = 0, or at a given non zero value.
                  Am : float
                      Airmass at which the std star was observed.
                  G : float
                      Gain [ADU/e].
                  Tex : float
                      Exposure time [s].
                  Atel : float
                      Collecting area of the telescope [cm2].
        
                  Returns
                  -------
                  hdrl.func.EfficiencyParameter
                      A newly alocated parameter.
        """
    @staticmethod
    def create_response_parameter(Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex) -> EfficiencyResponseParameter:
        """
                  Create an HDRL response parameter.
        
                  Parameters
                  ----------
                  Ap : float
                      Parameter to indicate if the efficiency is computed at
                      airmass = 0, or at a given non zero value.
                  Am : float
                      Airmass at which the std star was observed.
                  G : float
                      Gain [ADU/e].
                  Tex : float
                      Exposure time [s].
        
                  Returns
                  -------
                  hdrl.func.EfficiencyResponseParameter
                      A newly alocated parameter.
        """
class EfficiencyParameter:
    def __init__(self, Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex, Atel: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                   Constructor for Efficiency parameter.
        
                   Parameters
                   ----------
                   Ap : float
                       Parameter to indicate if the efficiency is computed at
                       airmass = 0, or at a given non zero value.
                   Am : float
                       Airmass at which the std star was observed.
                   G : float
                       Gain [ADU/e].
                   Tex : float
                       Exposure time [s].
                   Atel : float
                       Collecting area of the telescope [cm2].
        """
class EfficiencyResponseParameter:
    def __init__(self, Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                   Constructor for Efficiency Response parameter.
        
                   Parameters
                   ----------
                   Ap : float
                       Parameter to indicate if the efficiency is computed at
                       airmass = 0, or at a given non zero value.
                   Am : float
                       Airmass at which the std star was observed.
                   G : float
                       Gain [ADU/e].
                   Tex : float
                       Exposure time [s].
        """
class Flat:
    """
    
          The hdrl.func.Flat class provides an interface to combining single flatfields 
          into a master flatfield with increased signal-to-noise ratio. 
    
          Once the class has been instantiated with the desired parameters, the master flat 
          can be created using the member function hdrl.func.Flat.compute().
    
          Parameters
          ----------
          filter_size_x : int
            The dimension of the smoothing kernel (median filter) in the x-direction in pixel units.
          filter_size_y : int
            The dimension of the smoothing kernel (median filter) in the y-direction in pixel units.
          method : hdrl.func.Flat.Mode
            The flat combination algorithm mode, either
            hdrl.func.Flat.FreqLow or hdrl.func.Flat.FreqHigh.
          
          Returns
          -------
          hdrl.func.Flat
    
          Notes
          -----
          See hdrl.func.Flat.Mode for descriptions of each algorithm.
    
          Examples
          --------
          .. code-block:: python
    
            # Example 1: with a stat_mask 
            
            # choose a collapse method
            collapse = hdrl.func.Collapse.Mean()
            # dx and dy are image dimensions
            stat_mask = cpl.core.Mask(dx,dy)
            # set the mask for a window defined by (r1_llx, r1_lly, r1_urx, r1_ury)
            for j in range(r1_lly, r1_ury):
              for i in range(r1_llx, r1_urx):
                  stat_mask[j - 1][i - 1] = True
            # create the hdrl.func.Flat instance
            flat = hdrl.func.Flat(filter_size_x, filter_size_y, hdrl.func.Flat.Mode.FreqLow)
            # compute the master flat with imglist being an hdrl.core.ImageList 
            # holding the images to combine into the master flat
            results = flat.compute(imglist, collapse, stat_mask)
            mflat = results.master
            cmap = results.contrib_map
    
            # Example 2: without a stat_mask
    
            collapse = hdrl.func.Collapse.Median()
            stat_mask = None
            # create the hdrl.func.Flat instance
            flat = hdrl.func.Flat(filter_size_x, filter_size_y, hdrl.func.Flat.Mode.FreqLow)
            # compute the master flat with imglist being an hdrl.core.ImageList 
            # holding the images to combine into the master flat
            results = flat.compute(imglist, collapse, stat_mask)
    
          
    """
    class Mode:
        """
        The flat combination algorithm mode.
        
        Members:
        
          FreqLow : 
              This algorithm derives the low frequency part of the master flatfield – often also denoted as the shape of the flatfield. 
        
              The algorithm multiplicatively normalizes the input images by the median (considered to be 
              noiseless) of the image to unity. An optional static mask `stat_mask` can be provided to the
              algorithm in order to define the pixels that should be taken into account when computing the 
              normalisation factor. This allows the user to normalize the flatfield e.g. only by the illuminated
              section. In the next step, all normalized images are collapsed into a single master flatfield.
              The collapsing can be done with all methods currently implemented in HDRL (see hdrl.func.Collapse
              or Sect. 3.2.2 of the HDRL manual for an overview). Finally, the master flatfield is smoothed by 
              a median filter controlled by `filter_size_x` and `filter_size_y`. The associated error of the 
              final master frame is the error derived via error propagation of the previous steps, i.e. the 
              smoothing itself is considered noiseless. Please note that, if the smoothing kernel is set to unity, 
              i.e. filter_size_x = 1 and filter_size_y = 1, no final smoothing will take place but the resulting 
              masterframe is simply the collapsed normalized flatfield.
              
        
          FreqHigh : 
              This algorithm derives the high frequency part of the master flatfield – often also denoted as the 
              pixel-to-pixel variation of the flatfield.
        
              The algorithm first divides each input image by the smooth image obtained with a median filter 
              (the latter is controlled by the parameters `filter_size_x` and `filter_size_y`). Concerning the 
              error propagation, the smoothed image is considered to be noiseless, i.e. the relative error 
              associated to the normalised images is the same as the one of the input images. Then all residual 
              images are collapsed into a single master flatfield. The collapsing can be done with all methods 
              currently implemented in HDRL (see hdrl.func.Collapse or Sect. 3.2.2 of the HDRL manual for an 
              overview).
              
              To distinguish between illuminated and not illuminated regions/pixels (i.e. orders in an echelle flat image), the user may provide an optional static mask `stat_mask` to the algorithm. In this case the smoothing procedure is done twice, once for the illuminated region and once for the blanked region. This ensures that the information of one region does not influence the other regions during the smoothing process.
              
        """
        FreqHigh: typing.ClassVar[Flat.Mode]  # value = <Mode.FreqHigh: 1>
        FreqLow: typing.ClassVar[Flat.Mode]  # value = <Mode.FreqLow: 0>
        __members__: typing.ClassVar[dict[str, Flat.Mode]]  # value = {'FreqLow': <Mode.FreqLow: 0>, 'FreqHigh': <Mode.FreqHigh: 1>}
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
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self, filter_size_x: typing.SupportsInt | typing.SupportsIndex, filter_size_y: typing.SupportsInt | typing.SupportsIndex, method: Flat.Mode) -> None:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, hdrl_data: cpl.hdrl.core.ImageList, collapse: Collapse, stat_mask: cpl.core.Mask) -> typing.Any:
        """
               Compute the master flat image and a contribution map image. 
        
               Parameters
               ----------
        
               hdrl_data : hdrl.core.ImageList
                 The imagelist of images to combine into the master flat.
                
               collapse : hdrl.func.Collapse
                 Specifies the collapsing algorithm to apply to `hdrl_data`.
        
               stat_mask : cpl.core.Mask
                 A static mask to distinguish between illuminated and dark regions.
                 If no static mask is needed, this can be set to None. 
        
               Returns
               -------
               namedtuple
                 The namedtuple FlatResult contains two output products: a master flat image (hdrl.core.Image) and 
                 a contribution map image (cpl.core.Image).
                 The respective products may be accessed via the attributes master and contrib_map.
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        
               Notes
               -----
               See hdrl.func.Flat.Mode for descriptions of each algorithm.
        """
    @property
    def method(self) -> Flat.Mode:
        """
        hdrl.func.Flat.Mode : flat combination algorithm mode.
        """
    @property
    def sizex(self) -> int:
        """
        int : dimension of the smoothing kernel (median filter) in the x-direction in pixel units.
        """
    @property
    def sizey(self) -> int:
        """
        int : dimension of the smoothing kernel (median filter) in the y-direction in pixel units.
        """
class FpnResult:
    def __repr__(self) -> str:
        ...
    @property
    def power_spectrum(self) -> cpl.core.Image:
        ...
    @property
    def std(self) -> float:
        ...
    @property
    def std_mad(self) -> float:
        ...
class Fringe:
    """
    
          The hdrl.func.Fringe class provides an interface to fringe pattern detection 
          and correction in astronomical images. 
    
          The class provides two main methods:
          - compute(): Creates a master fringe pattern from a list of fringe images
          - correct(): Applies fringe correction to images using a master fringe pattern
    
          Parameters
          ----------
          None
            The Fringe class does not require any parameters for initialization.
          
          Returns
          -------
          hdrl.func.Fringe
    
          Notes
          -----
          The fringe functions work with HDRL image lists and can handle optional
          object masks and static masks for improved fringe detection and correction.
    
          Examples
          --------
          .. code-block:: python
    
            # Example 1: Compute master fringe pattern
            
            # Create fringe object
            fringe = hdrl.func.Fringe()
            
            # Create collapse method for combining images
            collapse = hdrl.func.Collapse.Mean()
            
            # Optional: Create static mask for fringe regions
            stat_mask = cpl.core.Mask(dx, dy)
            # Set mask for fringe-affected regions
            
            # Compute master fringe pattern
            # ilist_fringe is an hdrl.core.ImageList with fringe images
            # ilist_obj is optional cpl.core.ImageList with object masks (can be None)
            fringe.compute(ilist_fringe, ilist_obj, stat_mask, collapse)
            master_fringe = fringe.master
            contrib_map = fringe.contrib_map
            qctable = fringe.qctable
    
            # Example 2: Apply fringe correction
            
            # Apply correction to images using master fringe
            # ilist_fringe is an hdrl.core.ImageList with images to correct
            # masterfringe is the master fringe pattern from compute()
            result = fringe.correct(ilist_fringe, ilist_obj, stat_mask, masterfringe)
            qctable = result.qctable
    
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self) -> None:
        """
                  Create a Fringe computation object.
                  
                  The Fringe class provides methods for detecting and correcting
                  fringe patterns in astronomical images.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, ilist_fringe: cpl.hdrl.core.ImageList, ilist_obj: cpl.core.ImageList = None, stat_mask: cpl.core.Mask = None, collapse_params: Collapse = ...) -> None:
        """
                       Compute a master fringe pattern from a list of fringe images.
                       
                       This method combines multiple fringe images to create a master fringe
                       pattern with improved signal-to-noise ratio. The method uses the
                       specified collapse algorithm to combine the images.
                       
                       Parameters
                       ----------
                       ilist_fringe : hdrl.core.ImageList
                           List of fringe images to combine into master fringe pattern.
                       ilist_obj : cpl.core.ImageList or None, optional
                           Optional list of object masks. If provided, must have same size
                           and dimensions as ilist_fringe. Default is None.
                       stat_mask : cpl.core.Mask or None, optional
                           Optional static mask for fringe regions. If provided, must have
                           same dimensions as fringe images. Default is None.
                       collapse_params : hdrl.func.Collapse
                           Collapse method for combining fringe images.
                       
                       Returns
                       -------
                       None
                       
                       Raises
                       ------
                       hdrl.core.NullInputError
                           If ilist_fringe is None or empty, or if collapse_params is None.
                       hdrl.core.IncompatibleInputError
                           If ilist_obj dimensions don't match ilist_fringe, or if
                           stat_mask dimensions don't match fringe images.
        """
    def correct(self, ilist_fringe: cpl.hdrl.core.ImageList, ilist_obj: cpl.core.ImageList = None, stat_mask: cpl.core.Mask = None, masterfringe: cpl.hdrl.core.Image = None) -> FringeCorrectResult:
        """
                       Apply fringe correction to images using a master fringe pattern.
                       
                       This method corrects fringe patterns in images by subtracting
                       a scaled version of the master fringe pattern. The scaling is
                       determined by fitting the fringe amplitude for each image.
                       
                       Parameters
                       ----------
                       ilist_fringe : hdrl.core.ImageList
                           List of images to correct for fringe patterns.
                       ilist_obj : cpl.core.ImageList or None, optional
                           Optional list of object masks. If provided, must have same size
                           and dimensions as ilist_fringe. Default is None.
                       stat_mask : cpl.core.Mask or None, optional
                           Optional static mask for fringe regions. If provided, must have
                           same dimensions as fringe images. Default is None.
                       masterfringe : hdrl.core.Image
                           Master fringe pattern to use for correction. If None, uses
                           the master computed by compute().
                       
                       Returns
                       -------
                       FringeCorrectResult
                           Result object containing a cpl.core.Table, the quality control table with
                           background levels and fringe amplitudes for each image
                       
                       Raises
                       ------
                       hdrl.core.NullInputError
                           If ilist_fringe is None or empty, or if masterfringe is None.
                       hdrl.core.IncompatibleInputError
                           If ilist_obj dimensions don't match ilist_fringe, if
                           stat_mask dimensions don't match fringe images, or if
                           masterfringe dimensions don't match fringe images.
                       
                       Notes
                       -----
                       The correction is applied in-place to the images in ilist_fringe.
                       The method modifies the input images directly.
        """
    @property
    def contrib_map(self) -> cpl.core.Image:
        """
        Contribution map from the last compute() call.
        """
    @property
    def master(self) -> cpl.hdrl.core.Image:
        """
        Master fringe pattern from the last compute() call.
        """
    @property
    def qctable(self) -> cpl.core.Table:
        """
        QC table from the last compute() call.
        """
class FringeCorrectResult:
    """
    
          A hdrl.func.FringeCorrectResult class is a container for the results of hdrl.func.Fringe.correct().
          The results consist of a quality control table with background levels and fringe amplitudes.
          
          These can be accessed via the qctable attribute of the object.
          
          Example
          -------
          .. code-block:: python
              
              result = fringe.correct(ilist_fringe, ilist_obj, stat_mask, masterfringe)
              qctable = result.qctable
          
    """
    def __repr__(self) -> str:
        ...
    @property
    def qctable(self) -> cpl.core.Table:
        """
        cpl.core.Table : Quality control table with background levels and fringe amplitudes
        """
class LaCosmic:
    """
    
            The hdrl.func.LaCosmic class provides an interface to the LA-Cosmic algorithm 
            described in van Dokkum et al. 2001, PASP, 113, 1420 to detect bad-pixels
            and cosmic-rays hits on a single image.
            
            After creating an instance of the class using hdrl.func.LaCosmic() with the desired
            parameters, the detection is executed using the member function edgedetect().
    
            Parameters
            ----------
            sigma_lim : float
                Limiting sigma for detection on the sampling image.
            f_lim : float
                Limiting f factor for detection on the modified Laplacian image.
            max_iter : int
                Maximum number of iterations.
    
            Returns
            -------
            hdrl.func.LaCosmic
    
            See Also
            --------
            hdrl.func.LaCosmic.edgedetect : Detect bad-pixels or cosmic-rays on a single image.
    
            Notes
            -----
            For the algorithm see the paper of `van Dokkum et al. 2001, PASP, 113, 1420 <https://ui.adsabs.harvard.edu/abs/2001PASP..113.1420V/abstract>`_.
    
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self, sigma_lim: typing.SupportsFloat | typing.SupportsIndex, f_lim: typing.SupportsFloat | typing.SupportsIndex, max_iter: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def edgedetect(self, img_in: cpl.hdrl.core.Image) -> cpl.core.Mask:
        """
                Detect bad-pixels or cosmic-rays on a single image.
                
                This routine determines bad-pixels on a single image via edge
                detection following the LA-Cosmic algorithm described in van Dokkum
                et al. 2001, PASP, 113, 1420. It was originally developed to detect cosmic
                ray hits, but it can also be used in the more general context to detect
                bad pixels. The HDRL implementation does not use the error model as
                described in the paper, but instead uses the error image passed to the
                function. Moreover, several iterations are performed until no new 
                bad pixels are found or the number of iterations reaches max_iter.
                In each iteration the detected cosmic ray hits are replaced by the median 
                of the surrounding 5x5 pixels taking into account the pixel quality
                information. The input parameters `sigma_lim` and `f_lim` refer to
                :math:`\\sigma_{lim}` and :math:`f_{lim}` as described in the paper
                mentioned above.
        
                Parameters
                ----------
                ima_in : hdrl.core.Image
                  The input image
        
                Returns
                -------
                cpl.core.Mask 
                    A mask with all detected bad pixels or cosmics-rays marked as bad or None on error
        
                See Also
                --------
                hdrl.func.LaCosmic : Creates an LaCosmic instance.
        
                Notes
                -----
                Be aware that the implementation only detects positive bad
                pixels or cosmic ray hits, i.e. no "holes" in the image are detected,
                but in such a case the pixels surrounding the hole are marked as
                bad. Holes in the image can be introduced if e.g. one subtracts a not
                cosmic-ray-cleaned image from another image.
        """
    @property
    def f_lim(self) -> float:
        """
        float : Limiting f factor for detection on the modified Laplacian image.
        """
    @property
    def max_iter(self) -> int:
        """
        int : Maximum number of iterations.
        """
    @property
    def sigma_lim(self) -> float:
        """
        float : Limiting sigma for detection on the sampling image.
        """
class Maglim:
    """
    
            A hdrl.func.Maglim is a helper class for computing limiting magnitudes.
            
            Parameters
            ----------
            zeropoint : float
                Zeropoint for magnitude calculation.
            fwhm : float
                Full width at half maximum for the PSF.
            kernel_size_x : int
                Kernel size in x direction.
            kernel_size_y : int
                Kernel size in y direction.
            extend_method : hdrl.func.Maglim.ImageExtendMethod
                Image extension method (Nearest or Mirror).
            mode_param : hdrl.func.Collapse or hdrl.core.Parameter
                Mode parameter for the computation.
        
    """
    class ImageExtendMethod:
        """
        Image extension method for maglim computation.
        
        Members:
        
          Nearest : Extend using nearest value.
        
          Mirror : Extend using mirror value.
        """
        Mirror: typing.ClassVar[Maglim.ImageExtendMethod]  # value = <ImageExtendMethod.Mirror: 1>
        Nearest: typing.ClassVar[Maglim.ImageExtendMethod]  # value = <ImageExtendMethod.Nearest: 0>
        __members__: typing.ClassVar[dict[str, Maglim.ImageExtendMethod]]  # value = {'Nearest': <ImageExtendMethod.Nearest: 0>, 'Mirror': <ImageExtendMethod.Mirror: 1>}
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
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self, zeropoint: typing.SupportsFloat | typing.SupportsIndex, fwhm: typing.SupportsFloat | typing.SupportsIndex, kernel_size_x: typing.SupportsInt | typing.SupportsIndex, kernel_size_y: typing.SupportsInt | typing.SupportsIndex, extend_method: Maglim.ImageExtendMethod, mode_param: typing.Any) -> None:
        """
                         Create a Maglim computation object.
                         
                         Parameters
                         ----------
                         zeropoint : float
                             Zeropoint for magnitude calculation.
                         fwhm : float
                             Full width at half maximum for the PSF.
                         kernel_size_x : int
                             Kernel size in x direction.
                         kernel_size_y : int
                             Kernel size in y direction.
                         extend_method : hdrl.func.Maglim.ImageExtendMethod
                             Image extension method (Nearest or Mirror).
                         mode_param : hdrl.func.Collapse or hdrl.core.Parameter or None
                             Mode parameter for the computation.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, image: cpl.core.Image) -> float:
        """
                         Compute the limiting magnitude for the given image.
                         
                         Parameters
                         ----------
                         image : hdrl.core.Image
                             The input image.
                         
                         Returns
                         -------
                         float
                             The computed limiting magnitude.
        """
    @property
    def extend_method(self) -> Maglim.ImageExtendMethod:
        """
        Image extension method.
        """
    @property
    def fwhm(self) -> float:
        """
        Full width at half maximum for the PSF.
        """
    @property
    def kernel_size_x(self) -> int:
        """
        Kernel size in x direction.
        """
    @property
    def kernel_size_y(self) -> int:
        """
        Kernel size in y direction.
        """
    @property
    def mode_param(self) -> ...:
        """
        Mode parameter for the computation.
        """
    @property
    def zeropoint(self) -> float:
        """
        Zeropoint for magnitude calculation.
        """
class Overscan:
    """
    
          The hdrl.func.Overscan class provides an interface to overscan calculations.
          This module contains functionality to compute and correct the overscan level
          of CCD image.
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self, direction: str, ccd_ron: typing.SupportsFloat | typing.SupportsIndex, box_hsize: typing.SupportsInt | typing.SupportsIndex, collapse: Collapse, region: tuple) -> None:
        """
                  Create an Overscan computation object.
        
                  Parameters
                  ----------
                      direction : str
                          HDRL_X_AXIS ("x") or HDRL_Y_AXIS ("y")
                      ccd_ron : float
                          The CCD read out noise.
                      box_hsize : int
                          The running box half size.
                      collapse : hdrl.func.Collapse method
                          Collapse methods as defined in PyHDRL.
                          Supported methods: hdrl.func.Collapse.Mean, hdrl.func.Collapse.Median, hdrl.func.Collapse.SigClip, hdrl.func.Collapse.MinMax, hdrl.func.Collapse.Mode
                      region : tuple
                          The overscan computation region.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def compute(self, image: cpl.hdrl.core.Image) -> None:
        """
                         Compute the overscan correction model from an HDRL Image.
        
                         Parameters
                         ----------
                             image : hdrl.core.Image
                                 The image to compute the overscan model from.
        """
    def correct(self, input_image: cpl.hdrl.core.Image, region: tuple | None = None) -> typing.Any:
        """
                          Apply overscan correction to an HDRL Image.
        
                          Parameters
                          ----------
                              input_image : hdrl.core.Image
                                  Image to correct.
                              region : tuple[int, int, int, int], optional
                                  Region to apply correction.
        
                          Returns
                          -------
                          OverscanCorrectResult
                             Tuple with:
                                - corrected: corrected image (hdrl.core.Image)
                                - badmask: corresponding bad pixel mask (hdrl.core.pycpl_image)
        """
    @property
    def box_hsize(self) -> int:
        ...
    @property
    def ccd_ron(self) -> float:
        ...
    @property
    def chi2(self) -> ...:
        """
        Chi^2 image from the last compute() call.
        """
    @property
    def contribution(self) -> ...:
        """
        Contribution image from the last compute() call.
        """
    @property
    def correction(self) -> cpl.hdrl.core.Image:
        """
        Correction image from the last compute() call.
        """
    @property
    def direction(self) -> str:
        ...
    @property
    def minmax_reject_high(self) -> ...:
        """
        Min/max high rejection map from the last compute() call.
        """
    @property
    def minmax_reject_low(self) -> ...:
        """
        Min/max low rejection map from the last compute() call.
        """
    @property
    def overscan_region(self) -> tuple:
        """
        Returns the overscan region as a Window object.
        """
    @property
    def red_chi2(self) -> ...:
        """
        Reduced chi^2 image from the last compute() call.
        """
    @property
    def sigclip_reject_high(self) -> ...:
        """
        Sigma-clip high rejection map from the last compute() call.
        """
    @property
    def sigclip_reject_low(self) -> ...:
        """
        Sigma-clip low rejection map from the last compute() call.
        """
class Resample:
    """
    
          A hdrl.func.Resample class provides an interface to static functions 
          required to resample images and cubes. 
          
    """
    @staticmethod
    def compute(restable: cpl.core.Table, method: ResampleMethod, outputgrid: ResampleOutgrid, wcs: cpl.drs.WCS) -> ResampleResult:
        """
              This routine does not work directly on an image or cube but on a table (`restable`). 
              
              For 2D images, the table is created by the function hdrl.func.Resample.image_to_table(), 
              whereas for a 3D data cube the function hdrl.func.Resample.imagelist_to_table() is used. 
        
              In the case that many images or cubes have to be combined into a single mosaic, the two functions
              can be called multiple times and the returned tables should be merged into a single table using
              cpl.core.Table.insert().
        
              Parameters
              ----------
              restable : cpl.core.Table
                A PyCPL table with information on the data to be resampled. 
                
                It can be created either via hdrl.func.Resample.image_to_table() for 
                a 2D image or via hdrl.func.Resample.imagelist_to_table() for a 3D data cube.
                These functions require either an hdrl.core.Image (2D image) or hdrl.core.ImageList (3D data cube), 
                plus a valid cpl.drs.WCS object that encodes the world coordinate system of the given image or cube. 
        
                Alternatively, in case the above mentioned functions can not be used to create the table, 
                e.g. the pixel to sky mapping is very complex and can not be encoded by the cpl.drs.WCS object, 
                the pipeline developer has to create and fill the table. A template of this table can be
                generated using the function hdrl.func.Resample.restable_template().
        
              method : hdrl.func.ResampleMethod
                The interpolation `method` to apply as specified via an instance of the
                hdrl.func.ResampleMethod class, which can be instantiated via one of 
                the following constructors: 
                - hdrl.func.ResampleMethod.Nearest: Nearest neighbour resampling.
                - hdrl.func.ResampleMethod.Linear: Weighted resampling using an inverse distance weighting function.
                - hdrl.func.ResampleMethod.Quadratic: Weighted resampling using a quadratic inverse distance weighting function.
                - hdrl.func.ResampleMethod.Renka: Weighted resampling using a Renka weighting function.
                - hdrl.func.ResampleMethod.Drizzle: Weighted resampling using a drizzle-like weighting scheme.
                - hdrl.func.ResampleMethod.Lanczos: Weighted resampling using a Lanczos-like restricted sinc as weighting function.
        
              outputgrid : hdrl.func.ResampleOutgrid
                Defines basic properties of the resampled image or cube. Depending on the input
                data (image or cube), `outputgrid` should be an instance of the 
                hdrl.func.ResampleOutgrid class, which can be instantiated via one of
                the following constructors:
                - hdrl.func.ResampleOutgrid.User2D : User specified function for 2D images.
                - hdrl.func.ResampleOutgrid.User3D : User specified function for 3D data cubes.
                - hdrl.func.ResampleOutgrid.Auto2D : Convenience function for 2D images.
                - hdrl.func.ResampleOutgrid.Auto3D : Convenience function for 3D data cubes.
                In the case of the Auto2D and Auto3D constructors, only the step sizes in 
                right ascension, declination and wavelength of the output image or cube are required. 
                All the rest are automatically derived from the data by hdrl.func.Resample.compute().
              
              wcs : cpl.drs.WCS
                The World Coordinate System representative of the images to be resampled. 
                The resampling functions use the `wcs` input (CD matrix) mostly to determine the 
                scales between the input and output grid. 
                Please note, that in case the user would like to combine images or cubes with 
                substantially different pixel sizes into a single output image or cube, the 
                single tables have to be properly scaled to the same scales before merging them into the final table.
        
              Returns
              -------
              res : hdrl.func.ResampleResult
                An object containing the results consisting of a cpl.core.PropertyList, representing the image or cube header, 
                and an hdrl.core.ImageList. These can be accessed via the hdr and imlist attributes of the object.
        
              Example
              -------
              .. code-block:: python
        
                # plist is a cpl.core.PropertyList containing the WCS keywords from the header
                wcs = cpl.drs.WCS(plist)
                # himg is a hdrl.core.Image to be resampled
                table = hdrl.func.Resample.image_to_table(himg, wcs)
                rmethod = hdrl.func.ResampleMethod.Lanczos(1,False,2)
                outgrid = hdrl.func.ResampleOutgrid.Auto2D(0.01,0.01)
                result = hdrl.func.Resample.compute(table,rmethod,outgrid,wcs)
                first_img = result.imlist[0].image
                first_err = result.imlist[0].error
                hdr = result.hdr
        """
    @staticmethod
    def image_to_table(hima: cpl.hdrl.core.Image, wcs: cpl.drs.WCS) -> cpl.core.Table:
        """
              Creates the `restable` input table needed by hdrl.func.Resample.compute() for 2D images.
        
              Parameters
              ----------
              himg : hdrl.core.Image
                The image to be resampled. 
              wcs : cpl.drs.WCS
                The World Coordinate System representative of the images to be resampled. 
        
              Returns
              -------
              restable : cpl.core.Table
                The table that is used by hdrl.func.Resample.compute()
        
              See Also
              --------
              hdrl.func.Resample.imagelist_to_table : Creates the `restable` input table needed by hdrl.func.Resample.compute() for 3D data cubes.
              hdrl.func.Resample.restable_template : Creates a table template as per Sect. 4.14.3 of the HDRL manual. 
        """
    @staticmethod
    def imagelist_to_table(himlist: cpl.hdrl.core.ImageList, wcs: cpl.drs.WCS) -> cpl.core.Table:
        """
              Creates the `restable` input table needed by hdrl.func.Resample.compute() for 3D data cubes.
        
              Parameters
              ----------
              himlist : hdrl.core.ImageList
                The imagelist containing the images, i.e. the data cube, to be resampled. 
              wcs : cpl.drs.WCS
                The World Coordinate System representative of the images to be resampled. 
        
              Returns
              -------
              restable: cpl.core.Table
                The table that is used by hdrl.func.Resample.compute()
        
              See Also
              --------
              hdrl.func.Resample.image_to_table : Creates the `restable` input table needed by hdrl.func.Resample.compute() for 2D images.
              hdrl.func.Resample.restable_template : Creates a table template as per Sect. 4.14.3 of the HDRL manual. 
        """
    @staticmethod
    def restable_template(nrows: typing.SupportsInt | typing.SupportsIndex) -> cpl.core.Table:
        """
              Creates a table template as per Sect. 4.14.3 of the HDRL manual. 
              Useful for instances where the pixel to sky mapping is very complex 
              and can not be encoded in a cpl.drs.WCS object. This template can then 
              be filled by the pipeline developer as required. 
        
              Parameters
              ----------
              nrows : int 
                The number of rows to create.
        
              Returns
              -------
              restable : cpl.core.Table
                The table template with `nrows` rows. 
        
              See Also
              -------
              hdrl.func.Resample.image_to_table : Creates the `restable` input table needed by hdrl.func.Resample.compute() for 2D images.
              hdrl.func.Resample.imagelist_to_table : Creates the `restable` input table needed by hdrl.func.Resample.compute() for 3D data cubes.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
class ResampleMethod:
    """
    
          A hdrl.func.ResampleMethod class represents an interpolation algorithm 
          to be applied using hdrl.func.Resample.compute(). 
    
          The implemented interpolation algorithms are based on the MUSE pipeline and work for 2D images and 3D cubes. 
          The 2D and 3D interpolation is done in 2-dimensional and 3-dimensional spaces, respectively. 
          
          Currently there are six different interpolation methods implemented:
          - Nearest: Nearest neighbour resampling
          - Linear: Weighted resampling using an inverse distance weighting function
          - Quadratic: Weighted resampling using a quadratic inverse distance weighting function
          - Renka: Weighted resampling using a Renka weighting function
          - Drizzle: Weighted resampling using a drizzle-like weighting scheme
          - Lanczos: Weighted resampling using a Lanczos-like restricted sinc as weighting function
    
          Each method has its own separate constructor (e.g. hdrl.func.ResampleMethod.Nearest(), hdrl.func.ResampleMethod.Linear()).
          
    """
    @staticmethod
    def Drizzle(loop_distance: typing.SupportsInt | typing.SupportsIndex, use_errorweights: bool, pix_frac_x: typing.SupportsFloat | typing.SupportsIndex, pix_frac_y: typing.SupportsFloat | typing.SupportsIndex, pix_frac_lambda: typing.SupportsFloat | typing.SupportsIndex) -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Drizzle method.
              This method performs weighted resampling using a drizzle-like weighting scheme.
        
              Parameters
              ----------
              loop_distance : int
                Controls the number of surrounding pixels that are taken into account on the final grid.
              use_errorweights : bool
                Apply an additional weight of 1/variance.
              pix_frac_x : float
                Fraction of flux of the original pixel/voxel that drizzles into target pixel/voxel in the x-direction.
              pix_frac_y : float
                Fraction of flux of the original pixel/voxel that drizzles into target pixel/voxel in the y-direction.
              pix_frac_lambda : float
                Fraction of flux of the original pixel/voxel that drizzles into target pixel/voxel in the lambda-direction.
        
              Returns
              -------
              hdrl.func.ResampleMethod
                hdrl.func.ResampleMethod for the Drizzle method.
        
              Example
              -------
              .. code-block:: python
        
                loop_distance = 2
                use_errorweights = True
                pix_frac_drizzle_x = 0.8
                pix_frac_drizzle_y = 0.8
                pix_frac_drizzle_lambda = 1
                rmethod = hdrl.func.ResampleMethod.Drizzle(loop_distance, use_errorweights, pix_frac_drizzle_x, pix_frac_drizzle_y, pix_frac_drizzle_lambda)
              
              Notes
              -----
              The algorithm uses a drizzle-like distance weighting function for the interpolation. 
              The down-scaling factors `pix_frac_x`, `pix_frac_y`, and `pix_frac_lambda`, for x, y, and wavelength direction control the 
              percentage of flux of the original pixel/voxel that drizzles into the target pixel/voxel.
              The parameter `loop_distance` controls the number of surrounding pixels that are taken into account on the final grid, 
              e.g. a `loop_distance` of 1 uses 3 pixels :math:`(x - 1, x, x + 1)` in each dimension, i.e. 9 in total for a 2D image 
              and 27 in total for a 3D cube. 
              Moreover, if the parameter `use_errorweights` is set to True, an additional weight, defined as 1/variance, is taken into account.
              This additional weight is only applied if the variance of a pixel is greater than 0.
        
              See Also
              --------
              hdrl.func.ResampleMethod.Nearest : Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              hdrl.func.ResampleMethod.Linear : Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              hdrl.func.ResampleMethod.Quadratic : Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              hdrl.func.ResampleMethod.Renka : Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              hdrl.func.ResampleMethod.Lanczos : Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.  
        """
    @staticmethod
    def Lanczos(loop_distance: typing.SupportsInt | typing.SupportsIndex, use_errorweights: bool, kernel_size: typing.SupportsInt | typing.SupportsIndex) -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.
              This method performs weighted resampling using a Lanczos-like restricted sinc as weighting function.
        
              Parameters
              ----------
              loop_distance : int
                Controls the number of surrounding pixels that are taken into account on the final grid.
              use_errorweights : bool
                Apply an additional weight of 1/variance.
              kernel_size : int
                The kernel size in pixel units for the sinc distance weighting function.
        
              Returns
              -------
              hdrl.func.ResampleMethod
                hdrl.func.ResampleMethod for the Lanczos method.
        
              Example
              -------
              .. code-block:: python
        
                loop_distance = 2
                use_errorweights = True
                kernel_size = 2
                rmethod = hdrl.func.ResampleMethod.Lanczos(loop_distance, use_errorweights, kernel_size)
              
              Notes
              -----
              The algorithm uses a restricted sinc distance weighting function sinc(r)/sinc(r/kernel_size), 
              with the kernel size given by the parameter `kernel_size` for the interpolation.
              The parameter `loop_distance` controls the number of surrounding pixels that are taken into account on the final grid, 
              e.g. a `loop_distance` of 1 uses 3 pixels :math:`(x - 1, x, x + 1)` in each dimension, i.e. 9 in total for a 2D image 
              and 27 in total for a 3D cube. 
              Moreover, if the parameter `use_errorweights` is set to True, an additional weight, defined as 1/variance, is taken into account.
              This additional weight is only applied if the variance of a pixel is greater than 0.
              
              See Also
              --------
              hdrl.func.ResampleMethod.Nearest : Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              hdrl.func.ResampleMethod.Linear : Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              hdrl.func.ResampleMethod.Quadratic : Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              hdrl.func.ResampleMethod.Renka : Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              hdrl.func.ResampleMethod.Drizzle : Creates an instance of hdrl.func.ResampleMethod for the Drizzle method. 
        """
    @staticmethod
    def Linear(loop_distance: typing.SupportsInt | typing.SupportsIndex, use_errorweights: bool) -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              This method performs weighted resampling using an inverse distance weighting function.
        
              Parameters
              ----------
              loop_distance : int
                Controls the number of surrounding pixels that are taken into account on the final grid.
              use_errorweights : bool
                Apply an additional weight of 1/variance.
        
              Returns
              -------
              hdrl.func.ResampleMethod
               Instance of hdrl.func.ResampleMethod for the Linear method.
        
              Example
              -------
              .. code-block:: python
        
                loop_distance = 2
                use_errorweights = True
                rmethod = hdrl.func.ResampleMethod.Linear(loop_distance, use_errorweights)
              
              Notes
              -----
              The algorithm uses a linear inverse distance weighting function :math:`(1/r)` for the interpolation. 
              The parameter `loop_distance` controls the number of surrounding pixels that are taken into account on the final grid, 
              e.g. a `loop_distance` of 1 uses 3 pixels :math:`(x - 1, x, x + 1)` in each dimension, i.e. 9 in total for a 2D image 
              and 27 in total for a 3D cube. 
              Moreover, if the parameter `use_errorweights` is set to True, an additional weight, defined as 1/variance, is taken into account.
              This additional weight is only applied if the variance of a pixel is greater than 0.
        
              See Also
              --------
              hdrl.func.ResampleMethod.Nearest : Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              hdrl.func.ResampleMethod.Quadratic : Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              hdrl.func.ResampleMethod.Renka : Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              hdrl.func.ResampleMethod.Drizzle : Creates an instance of hdrl.func.ResampleMethod for the Drizzle method. 
              hdrl.func.ResampleMethod.Lanczos : Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.  
        """
    @staticmethod
    def Nearest() -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              This method performs nearest neighbour resampling. 
        
              Returns
              -------
              hdrl.func.ResampleMethod
                Instance of hdrl.func.ResampleMethod for Nearest method
        
              Example
              -------
              .. code-block:: python
        
                rmethod = hdrl.func.ResampleMethod.Nearest()
              
              Notes
              -----
              The algorithm does not use any weighting function, but simply uses the value of the nearest neighbour inside an output voxel [1]_ centre as the final output value. If there is no nearest neighbour inside the voxel (but e.g. only outside), the voxel is marked as bad. This speeds up the algorithm considerably. There are no control parameters for this method.
        
              .. [1] In 3D computer graphics, a voxel represents a value on a regular grid in three-dimensional space. See `http://https://en.wikipedia.org/wiki/Voxel <http://https://en.wikipedia.org/wiki/Voxel>`_ for more information.
        
              See Also
              --------
              hdrl.func.ResampleMethod.Linear : Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              hdrl.func.ResampleMethod.Quadratic : Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              hdrl.func.ResampleMethod.Renka : Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              hdrl.func.ResampleMethod.Drizzle : Creates an instance of hdrl.func.ResampleMethod for the Drizzle method. 
              hdrl.func.ResampleMethod.Lanczos : Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.  
        """
    @staticmethod
    def Quadratic(loop_distance: typing.SupportsInt | typing.SupportsIndex, use_errorweights: bool) -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              This method performs weighted resampling using a quadratic inverse distance weighting function. 
        
              Parameters
              ----------
              loop_distance : int
                Controls the number of surrounding pixels that are taken into account on the final grid.
              use_errorweights : bool
                Apply an additional weight of 1/variance.
        
              Returns
              -------
              hdrl.func.ResampleMethod
                Instance of hdrl.func.ResampleMethod for the Quadratic method.
        
              Example
              -------
              .. code-block:: python
        
                loop_distance = 2
                use_errorweights = True
                rmethod = hdrl.func.ResampleMethod.Quadratic(loop_distance, use_errorweights)
              
              Notes
              -----
              The algorithm uses a quadratic inverse distance weighting function :math:`(1/r^2)` for the interpolation. 
              The parameter `loop_distance` controls the number of surrounding pixels that are taken into account on the final grid, 
              e.g. a `loop_distance` of 1 uses 3 pixels :math:`(x - 1, x, x + 1)` in each dimension, i.e. 9 in total for a 2D image 
              and 27 in total for a 3D cube. 
              Moreover, if the parameter `use_errorweights` is set to True, an additional weight, defined as 1/variance, is taken into account.
              This additional weight is only applied if the variance of a pixel is greater than 0.
        
              See Also
              --------
              hdrl.func.ResampleMethod.Nearest : Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              hdrl.func.ResampleMethod.Linear : Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              hdrl.func.ResampleMethod.Renka : Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              hdrl.func.ResampleMethod.Drizzle : Creates an instance of hdrl.func.ResampleMethod for the Drizzle method. 
              hdrl.func.ResampleMethod.Lanczos : Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.  
        """
    @staticmethod
    def Renka(loop_distance: typing.SupportsInt | typing.SupportsIndex, use_errorweights: bool, critical_radius: typing.SupportsFloat | typing.SupportsIndex) -> ResampleMethod:
        """
              Creates an instance of hdrl.func.ResampleMethod for the Renka method.
              This method performs weighted resampling using a Renka weighting function.
        
              Parameters
              ----------
              loop_distance : int
                Controls the number of surrounding pixels that are taken into account on the final grid.
              use_errorweights : bool
                Apply an additional weight of 1/variance.
              critical_radius : float
                The distance beyond which the weights are set to 0.
        
              Returns
              -------
              hdrl.func.ResampleMethod
                Instance of hdrl.func.ResampleMethod for the Renka method.
        
              Example
              -------
              .. code-block:: python
        
                loop_distance = 2
                use_errorweights = True
                critical_radius = 3
                rmethod = hdrl.func.ResampleMethod.Renka(loop_distance, use_errorweights, critical_radius)
              
              Notes
              -----
              The algorithm uses a modified Shepard-like distance weighting function following Renka for the interpolation. 
              The parameter `critical_radius` defines the distance beyond which the weights are set to 0 and 
              the pixels are therefore not taken into account.
              The parameter `loop_distance` controls the number of surrounding pixels that are taken into account on the final grid, 
              e.g. a `loop_distance` of 1 uses 3 pixels :math:`(x - 1, x, x + 1)` in each dimension, i.e. 9 in total for a 2D image 
              and 27 in total for a 3D cube. 
              Moreover, if the parameter `use_errorweights` is set to True, an additional weight, defined as 1/variance, is taken into account.
              This additional weight is only applied if the variance of a pixel is greater than 0.
        
              See Also
              --------
              hdrl.func.ResampleMethod.Nearest : Creates an instance of hdrl.func.ResampleMethod for the Nearest method.
              hdrl.func.ResampleMethod.Linear : Creates an instance of hdrl.func.ResampleMethod for the Linear method.
              hdrl.func.ResampleMethod.Quadratic : Creates an instance of hdrl.func.ResampleMethod for the Quadratic method.
              hdrl.func.ResampleMethod.Drizzle : Creates an instance of hdrl.func.ResampleMethod for the Drizzle method. 
              hdrl.func.ResampleMethod.Lanczos : Creates an instance of hdrl.func.ResampleMethod for the Lanczos method.  
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
class ResampleOutgrid:
    """
    
          A hdrl.func.ResampleOutgrid class defines the basic properties of the resampled image or cube that are to
          be considered by hdrl.func.Resample.compute(). 
    
          It can be instantiated via one of the following constructors:
          - hdrl.func.ResampleOutgrid.User2D : User specified function for 2D images.
          - hdrl.func.ResampleOutgrid.User3D : User specified function for 3D data cubes.
          - hdrl.func.ResampleOutgrid.Auto2D : Convenience function for 2D images.
          - hdrl.func.ResampleOutgrid.Auto3D : Convenience function for 3D data cubes.
          In the case of the Auto2D and Auto3D constructors, only the step sizes in 
          right ascension, declination and wavelength of the output image or cube are required. 
          All the rest are automatically derived from the data by hdrl.func.Resample.compute().
          
    """
    @staticmethod
    def Auto2D(delta_ra: typing.SupportsFloat | typing.SupportsIndex, delta_dec: typing.SupportsFloat | typing.SupportsIndex) -> ResampleOutgrid:
        """
              Creates an instance of hdrl.func.ResampleOutgrid for 2D images.
        
              Parameters
              ----------
              delta_ra : float
                Output grid step in right ascension
              delta_dec : float
                Output grid step in declination
        
              Returns
              -------
              hdrl.func.ResampleOutgrid
                Instance of hdrl.func.ResampleOutgrid for 2D images.
        
              Example
              -------
              .. code-block:: python
        
                outputgrid = hdrl.func.ResampleOutgrid.Auto2D(0.1,0.2)
        
              See Also
              --------
              hdrl.func.ResampleOutgrid.User2D : Creates an instance of hdrl.func.ResampleOutgrid for 2D images. 
        """
    @staticmethod
    def Auto3D(delta_ra: typing.SupportsFloat | typing.SupportsIndex, delta_dec: typing.SupportsFloat | typing.SupportsIndex, delta_lambda: typing.SupportsFloat | typing.SupportsIndex) -> ResampleOutgrid:
        """
              Creates an instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        
              Parameters
              ----------
              delta_ra : float
                Output grid step in right ascension
              delta_dec : float
                Output grid step in declination
              delta_lambda: float
                Output grid step in wavelength
        
              Returns
              -------
              hdrl.func.ResampleOutgrid
                Instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        
              Example
              -------
              .. code-block:: python
        
                outputgrid = hdrl.func.ResampleOutgrid.Auto3D(0.1,0.2,0.001)
        
              See Also
              --------
              hdrl.func.ResampleOutgrid.User3D : Creates an instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        """
    @staticmethod
    def User2D(delta_ra: typing.SupportsFloat | typing.SupportsIndex, delta_dec: typing.SupportsFloat | typing.SupportsIndex, ra_min: typing.SupportsFloat | typing.SupportsIndex, ra_max: typing.SupportsFloat | typing.SupportsIndex, dec_min: typing.SupportsFloat | typing.SupportsIndex, dec_max: typing.SupportsFloat | typing.SupportsIndex, fieldmargin: typing.SupportsFloat | typing.SupportsIndex) -> ResampleOutgrid:
        """
              Creates an instance of hdrl.func.ResampleOutgrid for 2D images.
        
              Parameters
              ----------
              delta_ra : float
                Output grid step in right ascension
              delta_dec : float
                Output grid step in declination
              ra_min : float
                Minimum boundary of the image in right ascension
              ra_max : float
                Maximum boundary of the image in right ascension
              dec_min : float
                Minimum boundary of the image in declination
              dec_max : float
                Maximum boundary of the image in declination
              fieldmargin : float
                Percentage of how much margin to add to the output image in all spatial directions.
                A value of 0 adds no margin. 
        
              Returns
              -------
              hdrl.func.ResampleOutgrid
                instance of hdrl.func.ResampleOutgrid for 2D images.
        
              Example
              -------
              .. code-block:: python
        
                delta_ra = 0.1
                delta_dec = 0.2
                ra_min = 48.069416667
                ra_max = 48.0718125
                dec_min = -20.6229925
                dec_max = -20.620708611
                field_margin = 5
                outputgrid = hdrl.func.ResampleOutgrid.User2D(delta_ra, delta_dec, ra_min, ra_max, dec_min, dec_max, field_margin)
        
              See Also
              --------
              hdrl.func.ResampleOutgrid.Auto2D : Creates an instance of hdrl.func.ResampleOutgrid for 2D images. 
        """
    @staticmethod
    def User3D(delta_ra: typing.SupportsFloat | typing.SupportsIndex, delta_dec: typing.SupportsFloat | typing.SupportsIndex, delta_lambda: typing.SupportsFloat | typing.SupportsIndex, ra_min: typing.SupportsFloat | typing.SupportsIndex, ra_max: typing.SupportsFloat | typing.SupportsIndex, dec_min: typing.SupportsFloat | typing.SupportsIndex, dec_max: typing.SupportsFloat | typing.SupportsIndex, lambda_min: typing.SupportsFloat | typing.SupportsIndex, lambda_max: typing.SupportsFloat | typing.SupportsIndex, fieldmargin: typing.SupportsFloat | typing.SupportsIndex) -> ResampleOutgrid:
        """
              Creates an instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        
              Parameters
              ----------
              delta_ra : float
                Output grid step in right ascension
              delta_dec : float
                Output grid step in declination
              delta_lambda: float
                Output grid step in wavelength
              ra_min : float
                Minimum boundary of the image in right ascension
              ra_max : float
                Maximum boundary of the image in right ascension
              dec_min : float
                Minimum boundary of the image in declination
              dec_max : float
                Maximum boundary of the image in declination
              lambda_min : float
                Minimum boundary of the image in wavelength
              lambda_max : float
                Maximum boundary of the image in wavelength 
              fieldmargin : float
                Percentage of how much margin to add to the output image in all spatial directions.
                A value of 0 adds no margin. 
        
              Returns
              -------
              hdrl.func.ResampleOutgrid
                instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        
              Example
              -------
              .. code-block:: python
        
                delta_ra = 0.1
                delta_dec = 0.2
                delta_lambda = 0.001
                ra_min = 48.069416667
                ra_max = 48.0718125
                dec_min = -20.6229925
                dec_max = -20.620708611
                lambda_min = 1.9283e-06
                lambda_max = 2.47146e-06
                field_margin = 5
                outputgrid = hdrl.func.ResampleOutgrid.User3D(delta_ra, delta_dec, delta_lambda, ra_min, ra_max, dec_min, dec_max, lambda_min, lambda_max, field_margin)
        
              See Also
              --------
              hdrl.func.ResampleOutgrid.Auto3D : Creates an instance of hdrl.func.ResampleOutgrid for 3D data cubes.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
class ResampleResult:
    """
    
          A hdrl.func.ResampleResult class is a container for the results of hdrl.func.Resample.compute().
          The results consist of a cpl.core.PropertyList, representing the image or cube header, 
          and an hdrl.core.ImageList. 
    
          These can be accessed via the hdr and imlist attributes of the object.
    
          Example
          -------
          .. code-block:: python
    
            result = hdrl.func.Resample.compute(table,rmethod,outgrid,wcs)
            hdr = result.hdr
            himlist = result.imlist
            first_img = himlist[0].image
            first_err = himlist[0].error
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    @property
    def hdr(self) -> cpl.core.PropertyList:
        """
        cpl.core.PropertyList : image or cube header
        """
    @property
    def imlist(self) -> cpl.hdrl.core.ImageList:
        """
        hdrl.core.ImageList : imagelist containing the resampled images
        """
class Response:
    @staticmethod
    def calc_parameter_create(Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex) -> ResponseCalcParameter:
        """
                  Constructor for Response Calc parameter.
        
                  Parameters
                  ----------
                  Ap : float
                      Parameter to indicate if the efficiency is computed at
                      airmass = 0, or at a given non zero value.
                  Am : float
                      Airmass at which the std star was observed.
                  G : float
                      Gain [ADU/e].
                  Tex : float
                      Exposure time [s].
        
                  Returns
                  -------
                  hdrl.func.ResponseCalcParameter
        """
    @staticmethod
    def compute(obs_s: cpl.hdrl.core.Spectrum1D, ref_s: cpl.hdrl.core.Spectrum1D, E_x: cpl.hdrl.core.Spectrum1D, telluric_par: ResponseTelluricParameter, velocity_par: ResponseVelocityParameter, calc_par: ResponseCalcParameter, fit_par: ResponseFitParameter) -> ResponseResult:
        """
                  Computation of the response.
        
                  Parameters
                  ----------
                  obs_s : hdrl.core.Spectrum1D
                      Observed spectrum.
                  ref_s : hdrl.core.Spectrum1D
                      Reference std star spectrum
                  E_x : hdrl.core.Spectrum1D
                      Atmospheric Extinction
                  telluric_par : hdrl.func.ResponseTelluricParameter
                      Telluric correction parameter. NULL if telluric correction is
                      skipped.
                  velocity_par : hdrl.func.ResponseVelocityParameter
                      Doppler shift estimation and compensation. NULL if compensation
                      has to be skipped.
                  calc_par : hdrl.func.ResponseCalcParameter
                      Parameter for the core computation of the response, e.g. exposure
                      time.
                  fit_par : hdrl.func.ResponseFitParameter
                      Parameter for the final interpolation of the response.
        
                  Returns
                  -------
                  hdrl.func.ResponseResult
                      Response result
        """
    @staticmethod
    def evaluate_telluric_models(obs_s: cpl.hdrl.core.Spectrum1D, telluric_par: ResponseTelluricParameter) -> tuple[cpl.hdrl.core.Spectrum1D, float, float, float, int]:
        """
                  This function evaluates all the telluric models and picks the best model.
        
                  Parameters
                  ----------
                  obs_s : hdrl.core.Spectrum1D
                      Observed spectrum.
                  telluric_par : hdrl.func.ResponseTelluricParameter
                      Telluric correction parameter.
        
                  Returns
                  -------
                  tuple (hdrl.core.Spectrum1D, float, float, float, int)
                      The duplicated spectrum with the best telluric shift, mean,
                      standard devation, best model index.
        """
    @staticmethod
    def fit_parameter_create(radius: typing.SupportsInt | typing.SupportsIndex, fit_points: numpy.ndarray, wrange: typing.SupportsFloat | typing.SupportsIndex, high_abs_regions: tuple) -> ResponseFitParameter:
        """
                  Constructor for the hdrl_parameter for the final interpolation of the response.
        
                  Parameters
                  ----------
                  radius : int
                      Radius of the median filter used to smooth the response before
                      the final interpolation
                  fit_points : array
                      Median points where the fit will be calculated.
                  wrange : float
                      Range around the median point where the median is calculated.
                  high_abs_regions : tuple (array of float, array of float)
                      High absorption regions that should be skipped when calculating
                      the fit. If NULL no skipping is done.
        
                  Returns
                  -------
                  hdrl.func.ResponseFitParameter
        """
    @staticmethod
    def telluric_evaluation_parameter_create(telluric_models: cpl.hdrl.core.Spectrum1DList, w_step: typing.SupportsFloat | typing.SupportsIndex, half_win: typing.SupportsInt | typing.SupportsIndex, normalize: bool, shift_in_log_scale: bool, quality_areas: tuple, fit_areas: tuple, lmin: typing.SupportsFloat | typing.SupportsIndex, lmax: typing.SupportsFloat | typing.SupportsIndex) -> ResponseTelluricParameter:
        """
                  Constructor for Response Telluric parameter.
        
                  Parameters
                  ----------
                  telluric_models : hdrl.core.Spectrum1DList
                      The available telluric models.
                  w_step : float
                      Sampling step to use when upsampling model and observed spectrum
                      to calculate the cross correlations.
                  half_win : int
                      Half the search window to be used to find the peak of the cross
                      correlation.
                  normalize : boolean
                      True if the cross correlation should be normalized, False
                      otherwise.
                  shift_in_log_scale : boolean
                      True if the cross correlation has to be calculated in
                      logarithmic scale, False otherwise.
                  quality_areas : tuple (array of float, array of float)
                      Areas where the quality of the fit of the telluric model has to
                      be evaluated.
                  fit_areas : tuple (array of float, array of float)
                      Areas where the median points are extracted from, in order to
                      generate the final quality parameters of the telluric model.
                  lmin : float
                      Minimum wavelength used to calculate the cross-correlation (in
                      log scale if shift_in_log_scale = TRUE).
                  lmax : float
                      Maximum wavelength used to calculate the cross-correlation (in
                      log scale if shift_in_log_scale = TRUE).
        
                  Returns
                  -------
                  hdrl.func.ResponseTelluricParameter
        """
    @staticmethod
    def velocity_parameter_create(wguess: typing.SupportsFloat | typing.SupportsIndex, range_wmin: typing.SupportsFloat | typing.SupportsIndex, range_wmax: typing.SupportsFloat | typing.SupportsIndex, fit_wmin: typing.SupportsFloat | typing.SupportsIndex, fit_wmax: typing.SupportsFloat | typing.SupportsIndex, fit_half_win: typing.SupportsInt | typing.SupportsIndex) -> ResponseVelocityParameter:
        """
                  Constructor for Response Velocity parameter.
        
                  Parameters
                  ----------
                  wguess : float
                      Reference line wavelength position.
                  range_wmin : float
                      Minimum of wavelength box for line fit.
                  range_wmax : float
                      Maximum of wavelength box for line fit.
                  fit_wmin : float
                      Minimum wavelength value used to fit line slope.
                  fit_wmax : float
                      Maximum wavelength value used to fit line slope.
                  fit_half_win : int
                      Half box where polynomial fit is performed.
        
                  Returns
                  -------
                  hdrl.func.ResponseVelocityParameter
        """
class ResponseCalcParameter:
    def __init__(self, Ap: typing.SupportsFloat | typing.SupportsIndex, Am: typing.SupportsFloat | typing.SupportsIndex, G: typing.SupportsFloat | typing.SupportsIndex, Tex: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                   Constructor for Response Calc parameter.
        
                   Parameters
                   ----------
                   Ap : float
                       Parameter to indicate if the efficiency is computed at
                       airmass = 0, or at a given non zero value.
                   Am : float
                       Airmass at which the std star was observed.
                   G : float
                       Gain [ADU/e].
                   Tex : float
                       Exposure time [s].
        """
class ResponseFitParameter:
    def __init__(self, radius: typing.SupportsInt | typing.SupportsIndex, fit_points: numpy.ndarray, wrange: typing.SupportsFloat | typing.SupportsIndex, high_abs_regions: tuple) -> None:
        """
                   Constructor for the hdrl_parameter for the final interpolation of the response.
        
                   Parameters
                   ----------
                   radius : int
                       Radius of the median filter used to smooth the response before
                       the final interpolation
                   fit_points : array
                       Median points where the fit will be calculated.
                   wrange : float
                       Range around the median point where the median is calculated.
                   high_abs_regions : tuple (array of float, array of float)
                       High absorption regions that should be skipped when calculating
                       the fit. If NULL no skipping is done.
        """
class ResponseResult:
    def __repr__(self) -> str:
        ...
    def get_avg_diff_from_1(self) -> float:
        """
                   Get the value |mean - 1|, where mean is the average of the ratio
                   between the corrected observed spectrum and its smoothed fit.
                   This value can be used to assess the quality of the match of the
                   telluric model with the provided observed spectrum.
        
                   Returns
                   -------
                   float
                       The mean value.
        """
    def get_best_telluric_model_idx(self) -> int:
        """
                   Get the index of the telluric model used for telluric correction.
        
                   Returns
                   -------
                   int
                       The index.
        """
    def get_corrected_obs_spectrum(self) -> cpl.hdrl.core.Spectrum1D:
        """
                   Get the the corrected observed spectrum.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       The observed spectrum corrected by the telluric model.
        """
    def get_doppler_shift(self) -> float:
        """
                   Get the doppler shift used to correct the model.
        
                   Returns
                   -------
                   float
                       The value of doppler shift.
        """
    def get_final_response(self) -> cpl.hdrl.core.Spectrum1D:
        """
                   Get the final product of response calculations.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       Copy of the response spectrum.
        """
    def get_raw_response(self) -> cpl.hdrl.core.Spectrum1D:
        """
                   Get the raw response. The raw response is the ratio between the
                   observed spectrum and the reference one, corrected for e.g. gain,
                   atmospheric extinction, etc.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       Copy of the raw response.
        """
    def get_selected_response(self) -> cpl.hdrl.core.Spectrum1D:
        """
                   Get the selected response. The selected response is
                   the raw response sampled in the fit points. This response is
                   going then to be interpolated, creating the final response.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       Copy of the selected response.
        """
    def get_stddev(self) -> float:
        """
                   Get the standard deviation of the ratio between the corrected
                   observed spectrum and its smoothed fit.
                   This value can be used to assess the quality of the match of the
                   telluric model with the provided observed spectrum.
        
                   Returns
                   -------
                   float
                       The standard deviation value.
        """
    def get_telluric_shift(self) -> float:
        """
                   Get the shift applied to the telluric model.
                   This value can be used to assess the quality of the match of the
                   telluric model with the provided observed spectrum.
        
                   Returns
                   -------
                   float
                       The value of shift.
        """
class ResponseTelluricParameter:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, telluric_models: cpl.hdrl.core.Spectrum1DList, w_step: typing.SupportsFloat | typing.SupportsIndex, half_win: typing.SupportsInt | typing.SupportsIndex, normalize: bool, shift_in_log_scale: bool, quality_areas: tuple, fit_areas: tuple, lmin: typing.SupportsFloat | typing.SupportsIndex, lmax: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                   Constructor for Response Telluric parameter.
        
                   Parameters
                   ----------
                   telluric_models : hdrl.core.Spectrum1DList
                       The available telluric models.
                   w_step : float
                       Sampling step to use when upsampling model and observed spectrum
                       to calculate the cross correlations.
                   half_win : int
                       Half the search window to be used to find the peak of the cross
                       correlation.
                   normalize : boolean
                       True if the cross correlation should be normalized, False
                       otherwise.
                   shift_in_log_scale : boolean
                       True if the cross correlation has to be calculated in
                       logarithmic scale, False otherwise.
                   quality_areas : tuple (array of float, array of float)
                       Areas where the quality of the fit of the telluric model has to
                       be evaluated.
                   fit_areas : tuple (array of float, array of float)
                       Areas where the median points are extracted from, in order to
                       generate the final quality parameters of the telluric model.
                   lmin : float
                       Minimum wavelength used to calculate the cross-correlation (in
                       log scale if shift_in_log_scale = TRUE).
                   lmax : float
                       Maximum wavelength used to calculate the cross-correlation (in
                       log scale if shift_in_log_scale = TRUE).
        """
class ResponseVelocityParameter:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, wguess: typing.SupportsFloat | typing.SupportsIndex, range_wmin: typing.SupportsFloat | typing.SupportsIndex, range_wmax: typing.SupportsFloat | typing.SupportsIndex, fit_wmin: typing.SupportsFloat | typing.SupportsIndex, fit_wmax: typing.SupportsFloat | typing.SupportsIndex, fit_half_win: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                   Constructor for Response Velocity parameter.
        
                   Parameters
                   ----------
                   wguess : float
                       Reference line wavelength position.
                   range_wmin : float
                       Minimum of wavelength box for line fit.
                   range_wmax : float
                       Maximum of wavelength box for line fit.
                   fit_wmin : float
                       Minimum wavelength value used to fit line slope.
                   fit_wmax : float
                       Maximum wavelength value used to fit line slope.
                   fit_half_win : int
                       Half box where polynomial fit is performed.
        """
class Strehl:
    def __init__(self, wavelength: typing.SupportsFloat | typing.SupportsIndex, m1: typing.SupportsFloat | typing.SupportsIndex, m2: typing.SupportsFloat | typing.SupportsIndex, pixel_scale_x: typing.SupportsFloat | typing.SupportsIndex, pixel_scale_y: typing.SupportsFloat | typing.SupportsIndex, flux_radius: typing.SupportsFloat | typing.SupportsIndex, bkg_radius_low: typing.SupportsFloat | typing.SupportsIndex, bkg_radius_high: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                    The hdrl.func.Strehl class provides an interface to Strehl computation.
        
                    The most commonly used metrics for evaluating the AO correction is the Strehl
                    ratio. The Strehl ratio is defined as the ratio of the peak image intensity
                    from a point source compared to the maximum attainable intensity using an
                    ideal optical system limited only by diffraction over the telescope aperture.
                    The Strehl ratio is very frequently used to perform the quality control of
                    the scientific data obtained with the AO assisted instrumentation.
        
                    The class provides one main method:
                    - compute(): Compute the Strehl ratio on an image.
        
                    Parameters
                    ----------
                    wavelength : float
                        Nominal filter wavelength [m].
                    m1 : float
                        Primary mirror radius [m].
                    m2 : float
                        Obstruction radius [m].
                    pixel_scale_x : float
                        Image X pixel scale in [arcsec].
                    pixel_scale_y : float
                        Image Y pixel scale in [arcsec].
                    flux_radius : float
                        Radius used to sum the flux [arcsec].
                    bkg_radius_low : float
                        Radius used to determine the background [arcsec].
                    bkg_radius_high : float
                        Radius used to determine the background [arcsec].
        """
    def compute(self, himage: cpl.hdrl.core.Image) -> StrehlResult:
        """
              Compute the Strehl ratio on an image.
        
              The raw image is assumed to be pre-processed to remove the instrument
              signatures (bad pixels, etc.) and the natural noise sources (sky background,
              etc.). Nethertheless this function allows also the user to correct a residual
              background by setting the parameters bkg_radius_low, bkg_radius_high.
              The PSF is identified and its integrated flux (controlled by the parameter
              flux_radius) is normalized to 1. The
              PSF baricenter is computed and used to generate the ideal PSF (with
              integrated flux normalized to 1) which takes into account the telescope pupil
              characteristics (radius m1, central obstruction, m2, ...), the
              wavelength wavelength, at which the image has been obtained and the
              related pixel scale (pixel_scale_x, pixel_scale_y,). Finally the Strehl
              ratio is computed dividing the maximum intensity of the image PSF by the
              maximum intensity of the ideal PSF and the associated error is also computed.
        
              Parameters
              ----------
              himage : hdrl.core.Image
                  The image to process
        
              Returns
              -------
              StrehlResult
                  The Strehl value object
        """
    @property
    def bkg_radius_high(self) -> float:
        """
        Background radius high
        """
    @property
    def bkg_radius_low(self) -> float:
        """
        Background radius low
        """
    @property
    def flux_radius(self) -> float:
        """
        Flux radius
        """
    @property
    def m1(self) -> float:
        """
        M1
        """
    @property
    def m2(self) -> float:
        """
        M2
        """
    @property
    def pixel_scale_x(self) -> float:
        """
        Pixel scale x
        """
    @property
    def pixel_scale_y(self) -> float:
        """
        Pixel scale y
        """
    @property
    def wavelength(self) -> float:
        """
        Wavelength
        """
class StrehlResult:
    """
    
          The hdrl.func.StrehlResult class provides an interface to the results of the Strehl ratio computation.
    
          StrehlResult contains the computed Strehl value and its error.
    
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    @property
    def computed_background_error(self) -> float:
        """
        computed_background_error
        """
    @property
    def nbackground_pixels(self) -> int:
        """
        nbackground_pixels
        """
    @property
    def star_background(self) -> float:
        """
        star_background
        """
    @property
    def star_background_error(self) -> float:
        """
        star_background_error
        """
    @property
    def star_flux(self) -> float:
        """
        star_flux
        """
    @property
    def star_flux_error(self) -> float:
        """
        star_flux_error
        """
    @property
    def star_peak(self) -> float:
        """
        star_peak
        """
    @property
    def star_peak_error(self) -> float:
        """
        star_peak_error
        """
    @property
    def star_x(self) -> float:
        """
        star_x
        """
    @property
    def star_y(self) -> float:
        """
        star_y
        """
    @property
    def strehl_error(self) -> float:
        """
        strehl_error
        """
    @property
    def strehl_value(self) -> float:
        """
        strehl_value
        """
class Window:
    def __init__(self) -> None:
        ...
    def __repr__(self: tuple) -> str:
        ...
    @property
    def llx(self) -> int:
        ...
    @llx.setter
    def llx(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def lly(self) -> int:
        ...
    @lly.setter
    def lly(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def urx(self) -> int:
        ...
    @urx.setter
    def urx(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def ury(self) -> int:
        ...
    @ury.setter
    def ury(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
def fpn_compute(image: cpl.core.Image, mask: cpl.core.Mask = None, dc_mask_x: typing.SupportsInt | typing.SupportsIndex = 1, dc_mask_y: typing.SupportsInt | typing.SupportsIndex = 1) -> FpnResult:
    """
            Compute fixed pattern noise on a single image.
            
            This function detects fixed pattern noise on the input image. The algorithm
            first computes the power spectrum of the image using the Fast Fourier Transform (FFT),
            then computes the standard deviation and MAD-based standard deviation of the
            power spectrum excluding the masked region.
            
            Parameters
            ----------
            image : cpl.core.Image
                Input image (bad pixels are not allowed).
            mask : cpl.core.Mask, optional
                Optional input mask applied to the power spectrum. If None, no mask is used.
            dc_mask_x : int, optional
                X-pixel window (>= 1) to discard DC component starting from pixel (1, 1).
                Default is 1.
            dc_mask_y : int, optional
                Y-pixel window (>= 1) to discard DC component starting from pixel (1, 1).
                Default is 1.
            
            Returns
            -------
            FpnResult
                A result object containing:
                - power_spectrum: cpl.core.Image with the computed power spectrum
                - std: float, the standard deviation of the power spectrum
                - std_mad: float, the MAD-based standard deviation of the power spectrum
            
            Notes
            -----
            The power spectrum contains the DC component (the DC term is the 0 Hz
            term and is equivalent to the average of all the samples in the window)
            in pixel (1,1).
            
            The mask created on the fly by setting dc_mask_x and dc_mask_y and the
            optional mask are combined and are both taken into account when calculating
            std and std_mad.
            
            The final mask used to derive std and std_mad is attached to the
            power_spectrum image as a normal cpl mask and can be retrieved using
            power_spectrum.bpm.
            
            Raises
            ------
            hdrlcore.NullInputError
                If image is None.
            hdrlcore.IllegalInputError
                If dc_mask_x < 1 or dc_mask_y < 1, or if image contains bad pixels.
            hdrlcore.IncompatibleInputError
                If mask is not None and its size doesn't match the image size.
    """
