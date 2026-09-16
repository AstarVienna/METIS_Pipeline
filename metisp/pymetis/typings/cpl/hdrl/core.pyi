"""
HDRL Core submodule

  This module provides the core data types required by the HDRL algorithms,
  including Image (with error propagation) and ImageList.
  
"""
from __future__ import annotations
from abc import abstractmethod
import builtins as __builtins__
import collections.abc
from collections.abc import Sequence
import cpl.core
from inspect import getframeinfo
from inspect import stack
import numpy
import numpy.typing
import typing
__all__: list[str] = ['AKIMA', 'AccessOutOfRangeError', 'AssigningStreamError', 'BadFileFormatError', 'CSPLINE', 'CollapseResult', 'ContinueError', 'DataNotFoundError', 'DivisionByZeroError', 'DuplicatingStreamError', 'EOLError', 'Error', 'ErrorFrame', 'ErrorLostError', 'FileAlreadyOpenError', 'FileIOError', 'FileNotCreatedError', 'FileNotFoundError', 'IllegalInputError', 'IllegalOutputError', 'Image', 'ImageList', 'IncompatibleInputError', 'InterpolationMethod', 'InvalidTypeError', 'LINEAR', 'LOG', 'NoWCSError', 'NullInputError', 'Sequence', 'SingularMatrixError', 'Spectrum1D', 'Spectrum1DList', 'Spectrum1DResampleMethod', 'TypeMismatchError', 'UnspecifiedError', 'UnsupportedModeError', 'WaveScale', 'XCorrelationResult', 'abstractmethod', 'getframeinfo', 'stack']
class AccessOutOfRangeError(Error, LookupError):
    """
    Data were accessed beyond boundaries.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 11
class AssigningStreamError(Error, RuntimeError):
    """
    Could not associate a stream with a file descriptor.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 4
class BadFileFormatError(Error, RuntimeError):
    """
    Input file had not the expected format.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 6
class CollapseResult:
    def __repr__(self) -> str:
        ...
    @property
    def aligned_images(self) -> ImageList:
        """
        hdrl.core.CollapseResult.aligned_images : The aligned fluxes to be collapsed
        """
    @property
    def contrib(self) -> cpl.core.Image:
        """
        hdrl.core.CollapseResult.contrib : The output contribution mask
        """
    @property
    def result(self) -> Spectrum1D:
        """
        hdrl.core.CollapseResult.result: The resulting spectrum
        """
class ContinueError(Error, RuntimeError):
    """
    An iterative process did not converge.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 21
class DataNotFoundError(Error, RuntimeError):
    """
    Data searched within a valid object were not found.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 10
class DivisionByZeroError(Error, RuntimeError):
    """
    Attempted to divide a number by zero.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 18
class DuplicatingStreamError(Error, RuntimeError):
    """
    Could not duplicate output stream.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 3
class EOLError(Error, RuntimeError):
    """
    To permit extensibility of error handling.Do not raise this in Python as it will be a conding error in itself
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 23
class Error(Exception, collections.abc.Sequence):
    """
    
    **Abstract** base class of all CPL exceptions,
    Do not instantiate this class, instead use hdrl.core.NullInputError, hdrl.core.InvalidArgumentError, or any other subclass.
    **However** this class implements has all documentation for those error
    subclasses.
    
    In order to copy a cpl error, where you do not know the type of the
    error, use the hdrl.core.Error.create classmethod, as create can 
    dispatch to the relevant subclass.
    
    Examples
    --------
    .. code-block:: python
    
      try:
          # Some PyCPL functions are called here
      except hdrl.core.IllegalInputError as e:
          print(str(e.message))
      except hdrl.core.Error as e:
          print(str(e))
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    __hash__: typing.ClassVar[None] = None
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    @classmethod
    def create(cls, *args):
        """
        Create a subclass of Error, choosing subclass based on input arguments,
        so you don't need to know which subclass of error to create one.
        Instantiating a InvalidTypeError, FileIOError, etc... are preferred
        over using this function, when you know the error you're creating.
        
        This method has several overloads:
            * (copy: Error)
              Copy constructor copies the given error
        
            * (trace: List of Error)
              Given a list of Errors, this creates a stack trace out of those
              errors (essentially concatenating them) and produces a type the same 
              as the final error in the list
        
            * (code: int, function_name: str, file_name: str, line: unsigned, error_message: str)
              Creates a new error, (Only 1 frame in the trace) based on CPL error code
              Cpl error codes are available on subclasses as the 'code' class member
              e.g. IllegalInputError.code
        
            * (data: _Error_Data)
              Since Error is a wrapper around _Error_Data, this is the main constructor
        
        The class that is returned is a subclass of hdrl.core.Error
        """
    def __eq__(self, other):
        ...
    def __getitem__(self, index):
        ...
    def __init__(self, *args):
        """
        Use Error.create(...) or a known subclass e.g. InvalidTypeError;
        This class not instantiable by itself.
        
        This method has several overloads:
            * (function_name: str, file_name: str, line: unsigned, error_message: str)
              Creates a new error, (Only 1 frame in the trace)
        
            * (copy: Error)
              Copy constructor copies the given error
        
              If the given error does not match this Error subclass,
              an Value error is raised
        
            * (trace: List of Error)
              Given a list of Errors, this creates a stack trace out of those
              errors (essentially concatenating them) and produces a type the same 
              as the final error in the list
        
              If the last error in the trace does not match this Error subclass,
              an Value error is raised
        
            * (data: _Error_Data,)
              Used internally to create the Error from C++ hdrl::core::Error's
        
              If the given error does not match this Error subclass,
              an Value error is raised
        """
    def __len__(self):
        ...
    def __repr__(self):
        ...
    def __str__(self):
        ...
    @property
    def file(self):
        """
        C/C++ File where this error occurred or was re-thrown
        """
    @property
    def function(self):
        ...
    @property
    def line(self):
        """
        Line number (in a C/C++ file) where this error or was re-thrown
        """
    @property
    def message(self):
        ...
    @property
    def trace(self):
        ...
class ErrorFrame:
    __hash__: typing.ClassVar[None] = None
    @typing.overload
    def __eq__(self, arg0: ErrorFrame) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getstate__(self) -> tuple[int, str, str, int, str]:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def __str__(self) -> str:
        ...
    def error_class(self) -> typing.Any:
        ...
    @property
    def code(self) -> int:
        ...
    @property
    def file(self) -> str:
        ...
    @property
    def function(self) -> str:
        ...
    @property
    def line(self) -> int:
        ...
    @property
    def message(self) -> str:
        ...
class ErrorLostError(Error, RuntimeError):
    """
    Actual CPL error has been lost.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 2
class FileAlreadyOpenError(Error, RuntimeError):
    """
    Attempted to open a file twice.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 7
class FileIOError(Error, RuntimeError):
    """
    Access to file IO denied.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 5
class FileNotCreatedError(Error, RuntimeError):
    """
    Could not create a file.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 8
class FileNotFoundError(Error, RuntimeError):
    """
    A specified file or directory was not found.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 9
class IllegalInputError(Error, ValueError):
    """
    Illegal values were detected.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 14
class IllegalOutputError(Error, RuntimeError):
    """
    A given operation would have generated an illegal object.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 15
class Image:
    """
    
          A hdrl.core.Image is a HDRL image (a two-dimensional array
          object) containing data and its associated errors. It provides a similar
          API to cpl.core.Image and performs linear error propagation where it makes sense.
    
          The pixel indexing follows 0-indexing with the lower left corner having index (0, 0). The pixel
          buffer is stored row-wise so for optimum performance any pixel-wise access should be done likewise.
    
          The pixel ordering is of the order (y, x) to be consistent  with PyCPL
    
          Parameters
          ----------
          data : cpl.core.Image
                Image with data values.  
          error : cpl.core.Image
                Image with error values.
    
          Notes
          -----
          A new empty hdrl.core.Image of width x height dimension can be created using hdrl.core.Image.zeros
    
          See Also
          --------
          hdrl.core.Image.zeros : Create a new zeros filled hdrl.core.Image of width x height dimensions.
    
          
    """
    @staticmethod
    def zeros(width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex) -> Image:
        """
              Create a new zeros filled hdrl.core.Image of width x height dimensions.
        
              Parameters
              -----------
              width : int
                    Width of Image.
              height : int
                    Height of Image.
        
              Returns
              -------
              hdrl.core.Image
                    New hdrl.core.Image (width x height) initialised with all 0’s.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __deepcopy__(self, arg0: dict) -> Image:
        ...
    def __getitem__(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> tuple:
        ...
    def __init__(self, arg0: cpl.core.Image, arg1: cpl.core.Image) -> None:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def __repr__(self) -> str:
        ...
    def __setitem__(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex, value: tuple) -> None:
        ...
    def __str__(self) -> str:
        ...
    def accept(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Marks pixel as good.
        
              Parameters
              ----------
              ypos: int
                        y coordinate
              xpos: int
                        x coordinate
        
              See Also
              --------
              hdrl.core.Image.accept_all :  Returns the number of rejected pixels.
        """
    def accept_all(self) -> None:
        """
                    Returns the number of rejected pixels.
              
              See Also
              --------
              hdrl.core.Image.accept : Marks pixel as good.
        """
    def add_image(self, other: Image) -> None:
        """
              Adds values from Image other to self. Modified in place.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image added to self
        
              See Also
              --------
              hdrl.core.Image.add_image_create : Add two images and return the resulting image.
              hdrl.core.Image.add_scalar : Elementwise addition of a scalar to an image. Modified in place.
        """
    def add_image_create(self, other: Image) -> Image:
        """
              Add two images and return the resulting image.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image added to self
        
              Returns
              -------
              hdrl.core.Image
                    A newly allocated image.
        
              See Also
              --------
              hdrl.core.Image.add_image : Adds values from Image other to self. Modified in place.
              hdrl.core.Image.add_scalar : Elementwise addition of a scalar to an image. Modified in place.
        """
    def add_scalar(self, value: tuple) -> None:
        """
              Elementwise addition of a scalar to an image. Modified in place.
        
              Parameters
              ----------
              value : tuple(float, float)
                    Non-zero number to add to image values. The first component is the data value, the second is the error value.
        
              See Also
              --------
              hdrl.core.Image.add_image : Adds values from Image other to self. Modified in place.
              hdrl.core.Image.add_image_create : Add two images and return the resulting image.
        """
    def copy_into(self, other: Image, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Copy one hdrl.core.Image into another
        
              The two input images must be of the same type, namely one of
              cpl.core.Type.INT, cpl.core.Type.FLOAT, cpl.core.Type.DOUBLE.
        
              Parameters
              ----------
                other : hdrl.core.Image
                    The inserted image.
                ypos : int
                    the y pixel position in `self` where the lower left pixel of
                    `other` should go (from 0 to the y-1 size of `self`)
                xpos : int
                    the x pixel position in `self` where the lower left pixel of
                    `other` should go (from 0 to the x-1 size of `self`)
              
              See Also
              --------
              hdrl.core.Image.insert_into : Copy cpl.core.Image into an hdrl.core.Image
        """
    def count_rejected(self) -> int:
        """
              Returns the number of rejected pixels.
              
              See Also
              --------
              hdrl.core.Image.reject_from_mask : Sets the bad pixel mask of hdrl.core.Image
              hdrl.core.Image.reject : Marks pixel as bad.
              hdrl.core.Image.reject_value : Reject pixels with the specified special value(s)
              hdrl.core.Image.is_rejected : Return if the pixel is marked bad
        """
    def div_image(self, other: Image) -> None:
        """
              Divides self Image values by other Image values. Modified in place.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image that `self` is divided by.
        
              See Also
              --------
              hdrl.core.Image.div_scalar : Elementwise division of an image with a scalar. Modified in place.
              hdrl.core.Image.div_image_create : Divide two images and return the resulting image.
        """
    def div_image_create(self, other: Image) -> Image:
        """
              Divide two images and return the resulting image.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image that `self` is divided by.
        
              Returns
              -------
              hdrl.core.Image
                    A newly allocated image.
        
              See Also
              --------
              hdrl.core.Image.div_image : Divides self Image values by other Image values. Modified in place.
              hdrl.core.Image.div_scalar : Elementwise division of an image with a scalar. Modified in place.
        """
    def div_scalar(self, value: tuple) -> None:
        """
              Elementwise division of an image with a scalar. Modified in place.
        
              Parameters
              ----------
              value : tuple(float, float)
                    Non-zero number to divide with. The first component is the data value, the second is the error value.
        
              See Also
              --------
              hdrl.core.Image.div_image : Divides self Image values by other Image values. Modified in place.
              hdrl.core.Image.div_image_create : Divide two images and return the resulting image.
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', window: tuple | None = None, show: bool | None = True) -> str:
        """
              Dump the image contents to a file, stdout or a string.
        
              This function is intended just for debugging. It prints the contents of an image
              to the file path specified by `filename`. 
              If a `filename` is not specified, output goes to stdout (unless `show` is False). 
              In both cases, the contents are also returned as a string.
        
              Parameters
              ----------
              filename : str, optional
                    File to dump image contents to
              mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of
                    the file if it already exists), but can also be set to "a" (append, creating the file
                    if it does not already exist or appending to the end of it if it does).
              window : tuple(int,int,int,int), optional
                      Window to dump with `value` in the format (llx, lly, urx, ury) where:
                      - `llx` Lower left X coordinate
                      - `lly` Lower left Y coordinate
                      - `urx` Upper right X coordinate 
                      - `ury` Upper right Y coordinate
                      Defaults to None (no window).
              show : bool, optional
                  Send image contents to stdout. Defaults to True.
        
              Returns
              -------
              str 
                  A multiline string containing the dump of the image contents.
        """
    def duplicate(self) -> Image:
        """
              Copy hdrl.core.Image
              
              Returns
              -------
              hdrl.core.Image
                    New image with duplicate values.
              
              See Also
              --------
              hdrl.core.Image.copy_into : Copy one hdrl.core.Image into another.
        """
    def exp_scalar(self, base: tuple) -> None:
        """
              Computes the exponential of an image by a scalar. Modified in place.
        
              Parameters
              ----------
              base: tuple(float, float)
                    Base of the power. The first component is the data value, the second is the error value.
              
        
              See Also
              --------
              hdrl.core.Image.exp_scalar_create : Computes the exponential of an image by a scalar creating a new image.
        """
    def exp_scalar_create(self, base: tuple) -> Image:
        """
              Computes the exponential of an image by a scalar creating a new image.
        
              Parameters
              ----------
              base: tuple(float, float)
                    Base of the power. The first component is the data value, the second is the error value.
        
              Returns
              -------
              hdrl.core.Image
                    A new image containing the powered data.
        
              See Also
              --------
              hdrl.core.Image.exp_scalar : Computes the exponential of an image by a scalar. Modified in place.
        """
    def extract(self, window: tuple = None) -> Image:
        """
              Dump the image contents to a file, stdout or a string.
              Extract copy of window from hdrl.core.Image
        
              Parameters
              ----------
              window : tuple(int,int,int,int), optional
                      Window to dump with `value` in the format (llx, lly, urx, ury) where:
                      - `llx` Lower left X coordinate
                      - `lly` Lower left Y coordinate
                      - `urx` Upper right X coordinate 
                      - `ury` Upper right Y coordinate
                      Defaults to None (no window).
        
              Returns
              -------
              hdrl.core.Image
                  A newly allocated hdrl.core.Image containing the window.
        """
    def get_mean(self) -> tuple:
        """
              Computes mean pixel value and associated error of an image.
              
              Returns
              -------
               namedtuple
                 The namedtuple Value contains two doubles: It returns the mean (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
              
              
              See Also
              --------
              hdrl.core.Image.get_weighted_mean : Computes the weighted mean and associated error of an image.
              hdrl.core.Image.get_minmax_mean : Computes the minmax rejected mean and the associated error of an image.
              hdrl.core.Image.get_sigclip_mean : Computes the sigma-clipped mean and associated error of an image.
        """
    def get_median(self) -> tuple:
        """
              Computes the median and associated error of an image.
              
              Returns
              -------
               namedtuple
                 The namedtuple Value contains two doubles: It returns the median (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        """
    def get_minmax_mean(self, nlow: typing.SupportsFloat | typing.SupportsIndex, nhigh: typing.SupportsFloat | typing.SupportsIndex) -> tuple:
        """
              Computes the minmax rejected mean and the associated error of an image.
        
              Parameters
              ----------
              nlow: float
                    Number of low pixels to reject.
              nhigh: float
                    Number of high pixels to reject.
        
              Returns
              -------
              namedtuple
                 The namedtuple Value contains two doubles: It returns the  minmax rejected mean (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        
        
              See Also
              --------
              hdrl.core.Image.get_sigclip_mean : Computes the sigma-clipped mean and associated error of an image.
              hdrl.core.Image.get_weighted_mean : Computes the weighted mean and associated error of an image.
              hdrl.core.Image.get_mean : Computes mean pixel value and associated error of an image.
        """
    def get_mode(self, histo_min: typing.SupportsFloat | typing.SupportsIndex, histo_max: typing.SupportsFloat | typing.SupportsIndex, bin_size: typing.SupportsFloat | typing.SupportsIndex, method: hdrl_mode_type, niter: typing.SupportsInt | typing.SupportsIndex) -> tuple:
        """
              Computes the mode and the associated error of an image.
        
              Parameters
              ----------
              histo_min : float
                minimum value of low pixels to be uses
              histo_max : float
                 maximum value of high pixels to be used
              bin_size : float
                  the size of the histogram bin
              method : hdrl.func.Collapse.Method
                    method to use for the mode computation
              niter : int
               number of iterations to compute the error of the mode
        
              Returns
              -------
              namedtuple
                 The namedtuple Value contains two doubles: It returns the mode (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        """
    def get_pixel(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> tuple:
        """
              Gets pixel values of hdrl.core.Image
        
              Parameters
              ----------
              ypos: int
                        y coordinate
              xpos: int
                        x coordinate
        
              Returns
              -------
              namedtuple
                 The namedtuple Value contains two doubles:
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
                 It returns the pixel data value (data) and its error (error).
        
              See Also
              --------
              hdrl.core.Image.set_pixel : Sets pixel values of hdrl.core.Image
        """
    def get_sigclip_mean(self, kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, niter: typing.SupportsInt | typing.SupportsIndex) -> tuple:
        """
              Computes the sigma-clipped mean and associated error of an image.
        
              Parameters
              ----------
              kappa_low: float
                    low sigma bound
              kappa_high: float
                    high sigma bound.
              niter: int
                    maximum number of clipping iterators.
        
              Returns
              -------
               namedtuple
                 The namedtuple Value contains two doubles: It returns the clipped mean (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        
              See Also
              --------
              hdrl.core.Image.get_minmax_mean : Computes the minmax rejected mean and the associated error of an image.
              hdrl.core.Image.get_mean : Computes mean pixel value and associated error of an image.
              hdrl.core.Image.get_weighted_mean : Computes the weighted mean and associated error of an image.
        """
    def get_sqsum(self) -> tuple:
        """
              Computes the sum of all pixel values and the error of a squared image.
              
              Returns
              -------
              namedtuple
                 The namedtuple Value contains two doubles: It returns the squared sum (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        
              
              See Also
              --------
              hdrl.core.Image.get_sum : Computes the sum of all pixel values and the associated error of an image.
        """
    def get_stdev(self) -> float:
        """
              Computes the standard deviation of the data of an image
              
              Returns
              -------
               float
                    The standard deviation of the data of an image.
        """
    def get_sum(self) -> tuple:
        """
              Computes the sum of all pixel values and the associated error of an image.
        
              Returns
              -------
              namedtuple
                 The namedtuple Value contains two doubles: It returns the sum (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
        
              
              See Also
              --------
              hdrl.core.Image.get_sqsum : Computes the sum of all pixel values and the error of a squared image.
        """
    def get_weighted_mean(self) -> tuple:
        """
              Computes the weighted mean and associated error of an image.
              
              Returns
              -------
               namedtuple
                 The namedtuple Value contains two doubles: It returns the weighted mean (data) and its error (error).
                 If desired, the namedtuple can be converted to a dictionary using its ._asdict() method.
              
              
              See Also
              --------
              hdrl.core.Image.get_mean : Computes mean pixel value and associated error of an image.
              hdrl.core.Image.get_minmax_mean : Computes the minmax rejected mean and the associated error of an image.
              hdrl.core.Image.get_sigclip_mean : Computes the sigma-clipped mean and associated error of an image.
        """
    def insert_into(self, image: cpl.core.Image, error: cpl.core.Image | None, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Copy cpl.core.Image into an hdrl.core.Image
        
              Parameters
              ----------
              image :      cpl.core.Image
                    the inserted image
              error :      cpl.core.Image
                    the inserted error, may be NULL
              ypos : int
                    the y pixel position in image 1 where the lower left pixel of
                    image 2 should go (from 1 to the y size of image 1)
              xpos : int
                    the x pixel position in image 1 where the lower left pixel of
                    image 2 should go (from 1 to the x size of image 1)
        
              See Also
              --------
              hdrl.core.Image.copy_into : Copy one hdrl.core.Image into another
        """
    def is_rejected(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
              Return if the pixel is marked bad
        
              Parameters
              ----------
              ypos: int
                        y coordinate
              xpos: int
                        x coordinate
        
              See Also
              --------
              hdrl.core.Image.reject_from_mask : Sets the bad pixel mask of hdrl.core.Image
              hdrl.core.Image.reject : Marks pixel as bad.
              hdrl.core.Image.reject_value : Reject pixels with the specified special value(s).
              hdrl.core.Image.count_rejected : Returns the number of rejected pixels.
        """
    def mul_image(self, other: Image) -> None:
        """
              Multiplies self Image values by other Image values. Modified in place.
              
              Parameters
              ----------
              other : hdrl.core.Image
                    Image that `self` is multiplied by.
              See Also
              --------
              hdrl.core.Image.mul_image_create :  Multiply two images and return the resulting image.
              hdrl.core.Image.mul_scalar : Elementwise multiplication of an image by a scalar. Modified in place.
        """
    def mul_image_create(self, other: Image) -> Image:
        """
              Multiply two images and return the resulting image.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image that `self` is multiplied by.
        
              Returns
              -------
              hdrl.core.Image
                    A newly allocated image.
        
              See Also
              --------
              hdrl.core.Image.mul_image :  Multiplies self Image values by other Image values. Modified in place.
              hdrl.core.Image.mul_scalar : Elementwise multiplication of an image by a scalar. Modified in place.
        """
    def mul_scalar(self, value: tuple) -> None:
        """
              Elementwise multiplication of an image by a scalar. Modified in place.
        
              Parameters
              ----------
              value : tuple(float, float)
                    Non-zero number to multiply with. The first component is the data value, the second is the error value.
        
              See Also
              --------
              hdrl.core.Image.mul_image_create :  Multiply two images and return the resulting image.
              hdrl.core.Image.mul_image : Multiplies self Image values by other Image values. Modified in place.
        """
    def pow_scalar(self, exponent: tuple) -> None:
        """
              Computes the power of an image by a scalar. Modified in place.
        
              Parameters
              ----------
              exponent : tuple(float, float)
                 Exponent of the power. The first component is the data value, the second is the error value.
        
              
              See Also
              --------
              hdrl.core.Image.pow_scalar_create : Computes the power of an image by a scalar creating a new image.
        """
    def pow_scalar_create(self, exponent: tuple) -> Image:
        """
              Computes the power of an image by a scalar creating a new image.
        
              Parameters
              ----------
              exponent : tuple(float, float)
                 Exponent of the power. The first component is the data value, the second is the error value.
        
              Returns
              -------
              hdrl.core.Image
                   A new image containing the powered data.
        
              
              See Also
              --------
              hdrl.core.Image.pow_scalar : Computes the power of an image by a scalar. Modified in place.
        """
    def reject(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Marks pixel as bad.
        
              Parameters
              ----------
              ypos: int
                        y coordinate
              xpos: int
                        x coordinate
        
              See Also
              --------
              hdrl.core.Image.reject_from_mask : Sets the bad pixel mask of hdrl.core.Image
              hdrl.core.Image.is_rejected : Return if the pixel is marked bad
              hdrl.core.Image.reject_value : Reject pixels with the specified special value(s).
              hdrl.core.Image.count_rejected : Returns the number of rejected pixels.
        """
    def reject_from_mask(self, map: cpl.core.Mask) -> None:
        """
              Sets the bad pixel mask of hdrl.core.Image
        
              Parameters
              ----------
              map : cpl.core.Mask
                    Bad pixel mask to set.
        
              
              See Also
              --------
              hdrl.core.Image.reject : Marks pixel as bad.
              hdrl.core.Image.is_rejected : Return if the pixel is marked bad
              hdrl.core.Image.reject_value : Reject pixels with the specified special value(s).
              hdrl.core.Image.count_rejected : Returns the number of rejected pixels.
        """
    def reject_value(self, values: set) -> None:
        """
                Reject pixels with the specified special value(s)
        
              Parameters
              ----------
                values: set
                  The set of special values that should be marked as rejected pixels.
                  The supported special values are 0, math.inf, -math.inf, math.nan
                  and their numpy equivalents, and any combination is allowed.
        
              Raises
              ------
                hdrl.core.UnsupportedModeError
                  If something other than one of the supported special values is in
                  the values parameter.
                hdrl.core.InvalidTypeError
                  If the image is a complex type.
              
        
              See Also
              --------
              hdrl.core.Image.reject_from_mask : Sets the bad pixel mask of hdrl.core.Image
              hdrl.core.Image.reject : Marks pixel as bad.
              hdrl.core.Image.is_rejected : Return if the pixel is marked bad.
              hdrl.core.Image.count_rejected : Returns the number of rejected pixels.
        """
    def set_pixel(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex, value: tuple) -> None:
        """
              Sets pixel values of hdrl.core.Image
        
              Parameters
              ----------
              ypos: int
                        y coordinate
              xpos: int
                        x coordinate
              value: tuple(float, float)
                    Data value to set. The first component is the data value, the second is the error value.
        
              See Also
              --------
              hdrl.core.Image.get_pixel : Gets pixel values of hdrl.core.Image
        """
    def sub_image(self, other: Image) -> None:
        """
              Subtracts other Image values from self Image values. Modified in place.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image subtracted from self.
        
              See Also
              --------
              hdrl.core.Image.sub_image_create : Subtract two images and return the resulting image.
              hdrl.core.Image.sub_scalar : Elementwise subtraction of a scalar from an image. Modified in place.
        """
    def sub_image_create(self, other: Image) -> Image:
        """
              Subtract two images and return the resulting image.
        
              Parameters
              ----------
              other : hdrl.core.Image
                    Image subtracted from self.
              Returns
              -------
              hdrl.core.Image
                    A newly allocated image.
        
              See Also
              --------
              hdrl.core.Image.sub_image : Subtracts other Image values from self Image values. Modified in place.
              hdrl.core.Image.sub_scalar : Elementwise subtraction of a scalar from an image. Modified in place.
        """
    def sub_scalar(self, value: tuple) -> None:
        """
              Elementwise subtraction of a scalar from an image. Modified in place.
        
              Parameters
              ----------
              value : tuple(float, float)
                    Non-zero number to subtract from self. The first component is the data value, the second is the error value.
        
              See Also
              --------
              hdrl.core.Image.sub_image : Subtracts other Image values from self Image values. Modified in place.
              hdrl.core.Image.sub_image_create : Subtract two images and return the resulting image.
        """
    def turn(self, rot: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
              Rotate an image by a multiple of 90 degrees clockwise. Modified in place.
        
              Parameters
              ----------
              rot : int
                  Value for rotating image by 90 deg in the counterclockwise direction.
        """
    @property
    def error(self) -> cpl.core.Image:
        """
        cpl.core.Image: the error image
        """
    @property
    def height(self) -> int:
        """
        int: Height of the image
        """
    @property
    def image(self) -> cpl.core.Image:
        """
        cpl.core.Image: The primary image
        """
    @property
    def mask(self) -> cpl.core.Mask:
        """
        cpl.core.Mask: The image mask
        """
    @property
    def size(self) -> int:
        """
        int : Total number of pixels in the image (width*height)
        """
    @property
    def width(self) -> int:
        """
        int: Width of the image
        """
class ImageList:
    """
    
          A hdrl.core.ImageList is an HDRL imagelist containing a list of equally dimensioned HDRL Images. The API is similar
          to cpl.core.Imagelist and simple arithmetic and collapse operations propagate errors linearly. It follows 0-indexing
          and must have the same pixel type.
    
          Parameters
          ----------
          datalist : cpl.core.ImageList
              Data cpl.core.ImageList to store in `self` on init.
          errorlist : cpl.core.ImageList
              cpl.core.ImageList of corresponding errors to store in `self` on init.
    
          Notes
          -----
          A new empty hdrl.core.ImageList can be created using hdrl.core.ImageList()
    
          
    """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __deepcopy__(self, arg0: dict) -> ImageList:
        ...
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Image:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: cpl.core.ImageList, arg1: cpl.core.ImageList) -> None:
        ...
    def __len__(self) -> int:
        """
        int: size of the imagelist
        """
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def __repr__(self) -> str:
        ...
    def __setitem__(self, himg: typing.SupportsInt | typing.SupportsIndex, index: Image) -> None:
        ...
    def __str__(self) -> str:
        ...
    def add_image(self, himg: Image) -> None:
        """
                Add an Image to this ImageList. Modified in place.
        
                The input image must have the same size as those in this Imagelist, the input image is added elementwise to each image in this list.
        
                Parameters
                ----------
                himg : hdrl.core.Image
                    Image to add
        
                See Also
                --------
                hdrl.core.Image.add_image : Adds values from Image other to self. Modified in place.
        """
    def add_imagelist(self, himagelist: ImageList) -> None:
        """
                Add this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is added to the image number n in this list.
        
                Parameters
                ----------
                himglist : hdrl.core.ImageList
                    ImageList to add
        
                See Also
                ---------
                hdrl.core.Image.add_image : Adds values from Image other to self. Modified in place.
        """
    def add_scalar(self, val: tuple) -> None:
        """
                Elementwise addition of a scalar to each image in the ImageList. Modified in place
        
                Parameters
                ----------
                value : tuple (float, float)
                    Value to add. The first component is the scalar number to add, the second is the error value.
        
                See Also
                --------
                hdrl.core.Image.add_scalar : Elementwise addition of a scalar to an image. Modified in place.
        """
    def append(self, to_append: Image) -> None:
        """
                Append an HDRL image to the end of `self`. To insert an image into a specific position then set via index
                (e.g. self[i] = new_image). It is not allowed to insert images of different sizes or types into a list.
        
                Parameters
                ----------
                to_append : hdrl.core.Image
                    The image to append
        """
    def collapse(self, collapse: ...) -> typing.Any:
        """
                Collapse an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts the
                        number of pixels contributed to each pixel image.
        
                See Also
                ----------
                hdrl.func.Collapse : Interface for Collapse operations.
                hdrl.func.Collapse.compute : Perform Collapse operations on an HDRL Image or ImageList.
        """
    def collapse_mean(self) -> typing.Any:
        """
                Mean collapse of an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
                Returns
                -------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts the
                        number of pixels contributed to each pixel image.
        
                See Also
                ----------
                hdrl.func.Collapse.Mean : Interface for performing Collapse operation on HDRL ImageList or Image with Mean parameters.
                hdrl.func.Collapse.compute : Perform Collapse operations on an HDRL Image or ImageList.
        """
    def collapse_median(self) -> typing.Any:
        """
                The median collapse of an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
                Returns
                --------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                ---------
                hdrl.func.Collapse.Median : Interface for Median Collapse on an HDRL Image or ImageList.
                hdrl.func.Collapse.compute : Perform Collapse operations on an HDRL Image or ImageList.
        """
    def collapse_minmax(self, nlow: typing.SupportsFloat | typing.SupportsIndex, nhigh: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
        """
                The min-max clipped collapse of an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
                Parameters
                ----------
                nlow : float
                    low number of pixels to reject
                nhigh : float
                    high number of pixels to reject
        
                Returns
                --------
                     namedtuple
                        The namedtuple has four components- one hdrl.core.Image (out), containing the collapsed image,
                        one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image, one cpl.core.Image (reject_low) containing low rejection threshold,
                        one cpl.core.Image (reject_high) containing high rejection threshold.
        
                See Also
                ----------
                hdrl.func.Collapse.compute : Perform Collapse operation on an HDRL ImageList to create one HDRL Image.
                hdrl.func.Collapse.MinMax : Interface for Min-max Clipped Collapse on an HDRL Image or ImageList.
        """
    def collapse_mode(self, histo_min: typing.SupportsFloat | typing.SupportsIndex, histo_max: typing.SupportsFloat | typing.SupportsIndex, bin_size: typing.SupportsFloat | typing.SupportsIndex, mode_method: hdrl_mode_type, error_niter: typing.SupportsInt | typing.SupportsIndex) -> typing.Any:
        """
                The mode collapse of an imagelist to a single image.
        
                The error is calculated from the data, depending on the `error_niter`.
        
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
                --------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                Notes
                ----------
                If the `error_niter` parameter is set to 0, it is doing an analytically error estimation. If the parameter is larger
                than 0, the error is calculated by a bootstrap Montecarlo simulation from the input data with the value of the parameter
                specifying the number of simulations. In this case the input data are perturbed with the bootstrap technique and
                the mode is calculated error_niter times. From this modes the standard deviation is calculated and returned as error.
        
                See Also
                ----------
                hdrl.func.Collapse.compute : Perform Collapse operation on an HDRL ImageList to create one HDRL Image.
                hdrl.func.Collapse.Mode : Perform Mode Collapse function on an HDRL ImageList or Image.
        """
    def collapse_sigclip(self, kappa_low: typing.SupportsFloat | typing.SupportsIndex, kappa_high: typing.SupportsFloat | typing.SupportsIndex, niter: typing.SupportsInt | typing.SupportsIndex) -> typing.Any:
        """
                The sigma clipped collapse of an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
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
                --------
                hdrl.func.Collapse.Sigclip : Interface for Sigma Clipped Collapse on an HDRL Image or ImageList.
                hdrl.func.Collapse.compute : Perform Collapse function on an HDRL ImageList to create one HDRL Image.
        """
    def collapse_weighted_mean(self) -> typing.Any:
        """
                The weighted mean collapse of an imagelist to a single image.
        
                Error propagation is taken to account where the propagation formula is well defined.
        
                Returns
                ----------
                     namedtuple
                        The namedtuple has two components- one hdrl.core.Image (out), containing the collapsed image
                        and one cpl.core.Image (contrib), which is the output mask containing an integer map that counts how
                        many pixels contributed to each pixel image.
        
                See Also
                --------
                hdrl.func.Collapse.WeightedMean : Perform Weighted Mean collapse on an HDRL Image or ImageList.
                hdrl.func.Collapse.compute : Perform Collapse operations on an HDRL Image or ImageList.
        """
    def div_image(self, himg: Image) -> None:
        """
                Divide this ImageList by an Image. Modified in place.
        
                The input image must have the same size as those in this Imagelist, each image in this list is divided elementwise by the input image.
        
                Parameters
                ----------
                himglist : hdrl.core.Image
                    Image to divide with
        
                See Also
                --------
                hdrl.core.Image.div_image : Divides self Image values by other Image values. Modified in place.
        """
    def div_imagelist(self, himagelist: ImageList) -> None:
        """
                Divide this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other divides the image number n in this list.
        
                Parameters
                ----------
                himglist : hdrl.core.ImageList
                    ImageList to divide with
        
                See Also
                --------
                hdrl.core.Image.div_image : Divides self Image values by other Image values. Modified in place.
        """
    def div_scalar(self, val: tuple) -> None:
        """
                Elementwise division of each image in the ImageList with a scalar.
        
                Parameters
                ----------
                value : tuple (float, float)
                    Non-zero number to divide with. The first component is the divisor, the second is the error value.
        
                See Also
                --------
                hdrl.core.Image.div_scalar : Elementwise division of an image with a scalar. Modified in place.
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', window: tuple | None = None, show: bool | None = True) -> str:
        """
                    Dump the contents of each image in the ImageList to a file, stdout or a string.
        
                    This function is intended just for debugging. It prints the contents of an image
                    to the file path specified by `filename`.
                    If a `filename` is not specified, output goes to stdout (unless `show` is False).
                    In both cases the contents are also returned as a string.
        
                    Parameters
                    ----------
                    filename : str, optional
                        File to dump file image contents to
                    mode : str, optional
                        Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                        but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                        it if it does).
                    window : tuple(int,int,int,int), optional
                      Window to dump with `value` in the format (llx, lly, urx, ury) where:
                      - `llx` Lower left X coordinate
                      - `lly` Lower left Y coordinate
                      - `urx` Upper right X coordinate
                      - `ury` Upper right Y coordinate
                    show : bool, optional
                        Send image contents to stdout. Defaults to True.
        
                    Returns
                    --------
                    str
                        Multiline string containing the dump of the image contents in the ImageList.
        """
    def duplicate(self) -> ImageList:
        """
                Copy the HDRL imagelist into a new HDRL imagelist. The pixels and errors are also copied.
        
                This method is also used when performing a deepcopy on an image.
        
                Returns
                ----------
                himlist : hdrl.core.ImageList
                    New HDRL ImageList that is a copy of the original ImageList.
        """
    def empty(self) -> None:
        """
                Empty an imagelist and deallocate all its images. After the call the image list can be populated again.
        """
    def is_consistent(self) -> int:
        """
                Determine if an ImageList contains images of equal size and type.
        
                Returns
                ----------
                result : int
                    0 if ok, positive if not consistent and negative on error. The function returns 1 if the list is empty.
        """
    def mul_image(self, himg: Image) -> None:
        """
                Multiply this ImageList by an Image. Modified in place.
        
                The input image must have the same size as those in this Imagelist, each image in this list is multiplied elementwise by the input image.
        
                Parameters
                ----------
                himglist : hdrl.core.Image
                    Image to multiply with
        
                See Also
                --------
                hdrl.core.Image.mul_image :  Multiplies self Image values by other Image values. Modified in place.
        """
    def mul_imagelist(self, himagelist: ImageList) -> None:
        """
                Multiply this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is multiplied the image number n in this list.
        
                Parameters
                ----------
                himglist : hdrl.core.ImageList
                    ImageList to multiply with
        
                See Also
                --------
                hdrl.core.Image.mul_image :  Multiplies self Image values by other Image values. Modified in place.
        """
    def mul_scalar(self, val: tuple) -> None:
        """
                Elementwise multiplication of a scalar to each image in the ImageList.
        
                Parameters
                ----------
                value : tuple (float, float)
                    Value to multiply with. The first component is the multiplicator, the second is the error value.
        
                See Also
                --------
                hdrl.core.Image.mul_scalar : Elementwise multiplication of an image by a scalar. Modified in place.
        """
    def pop(self, index: typing.SupportsInt | typing.SupportsIndex | None = None) -> Image:
        """
                Remove and return the image at the `index`.
        
                Parameters
                ----------
                position : int, optional
                    Index to pop image from the image list. Defaults to the last image.
        
                Returns
                ----------
                himg : hdrl.core.Image
                        Image at `index`.
        
                Raises
                ----------
                IndexError
                    If the `index` is out of range.
        """
    def pow_scalar(self, val: tuple) -> None:
        """
                Compute the elementwise exponential of each image in `self`. Modified in place.
        
                Parameters
                ----------
                base : tuple (float, float)
                    Base of the exponential. The first component is the base, the second is the error value.
        
                See Also
                --------
                hdrl.core.Image.pow_scalar : Computes the power of an image by a scalar. Modified in place.
        """
    def sub_image(self, himg: Image) -> None:
        """
                Subtract an Image from this ImageList. Modified in place.
        
                The input image must have the same size as those in this Imagelist, each image in this list is subtracted elementwise by the input image.
        
                Parameters
                ----------
                himglist : hdrl.core.Image
                    Image to subtract with
        
                See Also
                --------
                hdrl.core.Image.sub_image : Subtracts other Image values from self Image values. Modified in place.
        """
    def sub_imagelist(self, himagelist: ImageList) -> None:
        """
                Elementwise subtract this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is subtracted from the image number n in this list.
        
                Parameters
                ----------
                himglist: hdrl.core.ImageList
                    ImageList to subtract with
        
                See Also
                ---------
                hdrl.core.Image.sub_image : Subtracts other Image values from self Image values. Modified in place.
        """
    def sub_scalar(self, val: tuple) -> None:
        """
                Elementwise subtraction of a scalar to each image in the ImageList. Modified in place.
        
                Parameters
                ----------
                value : tuple (float, float)
                    Value to subtract. The first component is the scalar number to subtract, the second is the error value.
        
                See Also
                --------
                hdrl.core.Image.sub_scalar : Elementwise subtraction of a scalar from an image. Modified in place.
        """
    @property
    def size_x(self) -> int:
        """
        int: number of columns of images in an imagelist
        """
    @property
    def size_y(self) -> int:
        """
        int: number of rows of images in an imagelist
        """
class IncompatibleInputError(Error, ValueError):
    """
    Data that had to be processed together did not match.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 13
class InterpolationMethod:
    """
    Members:
    
      LINEAR
    
      CSPLINE
    
      AKIMA
    """
    AKIMA: typing.ClassVar[InterpolationMethod]  # value = <InterpolationMethod.AKIMA: 2>
    CSPLINE: typing.ClassVar[InterpolationMethod]  # value = <InterpolationMethod.CSPLINE: 1>
    LINEAR: typing.ClassVar[InterpolationMethod]  # value = <InterpolationMethod.LINEAR: 0>
    __members__: typing.ClassVar[dict[str, InterpolationMethod]]  # value = {'LINEAR': <InterpolationMethod.LINEAR: 0>, 'CSPLINE': <InterpolationMethod.CSPLINE: 1>, 'AKIMA': <InterpolationMethod.AKIMA: 2>}
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
class InvalidTypeError(Error, RuntimeError):
    """
    Data type was unsupported or invalid.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 20
class NoWCSError(Error, RuntimeError):
    """
    The WCS functionalities are missing.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 22
class NullInputError(Error, ValueError):
    """
    A __null pointer was found where a valid pointer was expected .Shouldnt appear in PyCPL but present in case such an error arises.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 12
class SingularMatrixError(Error, RuntimeError):
    """
    Could not invert a matrix.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 17
class Spectrum1D:
    """
    
          A hdrl.core.Spectrum1D is an HDRL spectrum1D containing the wavelengths, the fluxes at
          each wavelength together with their errors, and a wavelength scale.
      
    """
    def __add__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        ...
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __deepcopy__(self, arg0: dict) -> Spectrum1D:
        ...
    def __iadd__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
        ...
    def __imul__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
        ...
    @typing.overload
    def __init__(self, func: collections.abc.Callable, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], scale: str = 'linear') -> None:
        """
                   Constructor for the hdrl.core.Spectrum1D class in the case of a spectrum defined by an analytical function.
        
                   Parameters
                   ----------
                   func : function
                       The analytical function defining the spectrum.
                   wavelengths : array of float
                       The frequencies.
                   scale : string
                       The scale of the spectrum (logarithmic `log` or linear `linear`).
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       A newly alocated spectrum.
        """
    @typing.overload
    def __init__(self, flux: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], half_window: int, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], scale: str = 'linear') -> None:
        """
                   Constructor for the hdrl.core.Spectrum1D class when no error information is available, in this case we use DER_SNR to esimate the error.
        
                   Parameters
                   ----------
                   flux : array of float
                       The flux.
                   half_window : int
                       The half window the DER_SNR is calculated on.
                   wavelengths : array of float
                       The frequencies.
                   scale : string
                       The scale of the spectrum (logarithmic `log` or linear `linear`).
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       A newly alocated spectrum.
        
                   See Also
                   --------
                   The documentation of the function estimate_noise_DER_SNR().
        """
    @typing.overload
    def __init__(self, flux: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], flux_error: typing.Any, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], scale: str = 'linear') -> None:
        """
                   Constructor for the hdrl.core.Spectrum1D class when error information is available.
        
                   Parameters
                   ----------
                   flux : array of float
                       The flux.
                   flux_error : object
                       The error for the flux.
                   wavelengths : array of float
                       The frequencies.
                   scale : string
                       The scale of the spectrum (logarithmic `log` or linear `linear`).
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       A newly alocated spectrum.
        """
    def __isub__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
        ...
    def __itruediv__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
        ...
    def __len__(self) -> int:
        ...
    def __mul__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def __repr__(self) -> str:
        ...
    def __sub__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        ...
    def __truediv__(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        ...
    def add_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise addition of a spectrum by a scalar.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def add_spectrum(self, other: Spectrum1D) -> None:
        """
                  Sum two spectra.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        """
    def add_spectrum_create(self, other: Spectrum1D) -> Spectrum1D:
        """
                  Sum two spectra.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    def compute_shift_fit(self, wguess: typing.SupportsFloat | typing.SupportsIndex, wrange: typing.Annotated[collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex], "FixedSize(2)"], fitrange: typing.Annotated[collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex], "FixedSize(2)"], halfsize: typing.SupportsFloat | typing.SupportsIndex) -> float:
        """
                   Compute the spectral shift of the spectrum with respect to
                   an expected position of a spectral line.
        
                   Parameters
                   ----------
                   wguess : float
                       Expected wavelength of the spectral line.
                   wrange : tuple(float, float)
                       Tuple of the minimum and maximum wavelength defining the
                       wavelength range of the spectrum to use.
                   fitrange : tuple(float, float)
                       Tuple of the minimum and maximum wavelength defining the
                       wavelength range that is ignored when fitting the ratio of
                       the spectrum and the fitted model with a polynomial.
                   halfsize : float
                       Window half size defining the wavelength limits for
                       the polynomial fit.
        
                   Returns
                   -------
                   float
                       The relative shift of the spectrum with respect to the reference
                       wavelength.
        """
    def compute_shift_xcorrelation(self, other: Spectrum1D, half_win: typing.SupportsInt | typing.SupportsIndex, normalize: bool = True) -> XCorrelationResult:
        """
                  Calculate cross-correlation.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other spectrum.
                  half_win : int
                      The half search window where the correlation is calculated.
                  normalize : boolean
                      Flag, `true` if normalize correlation in mean and rms.
        
                  Returns
                  -------
                  XCorrelationResult
                      Object with cross-correlation results.
        """
    def div_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise division of a spectrum by a scalar.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def div_spectrum(self, other: Spectrum1D) -> None:
        """
                  Divide one spectrum by another spectrum.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The denumerator.
        """
    def div_spectrum_create(self, other: Spectrum1D) -> Spectrum1D:
        """
                  Divide one spectrum by another spectrum.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The denumerator.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    def duplicate(self) -> Spectrum1D:
        """
                  Create a duplicate of the spectrum.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      A new copy of the Spectrum1D.
        """
    def exp_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise power of the scalar to the flux.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def is_compatible_with(self, other: Spectrum1D) -> bool:
        """
                  Checks if two spectrum wavelengths are equal.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The spectrum to be compared with.
        
                  Returns
                  -------
                  boolean
                      The flag, true if compatible.
        """
    def is_uniformly_sampled(self) -> tuple[bool, float]:
        """
                   Checks if the spectrum is defined on uniformly sampled wavelengths.
        
                   Returns
                   -------
                   std.pair
                       The flag if the spectrum is defined on uniformly sampled
                       wavelengths and bin width.
        """
    def mul_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise multiplication of a spectrum by a scalar.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def mul_spectrum(self, other: Spectrum1D) -> None:
        """
                  Multiply one spectrum by another spectrum.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        """
    def mul_spectrum_create(self, other: Spectrum1D) -> Spectrum1D:
        """
                  Multiply one spectrum by another spectrum.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    def pow_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise power of of the flux to the scalar.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def reject_pixels(self, bad_samples: typing.Annotated[numpy.typing.ArrayLike, numpy.int32]) -> Spectrum1D:
        """
                  For every i-th element in bad_samples having value true,
                  the i-th pixel in the 1D spectrum is marked as bad.
        
                  Parameters
                  ----------
                  bad_samples : array of int
                      The flags indicating whether the pixel is bad.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The spectrum having the appropriate bad pixels selected.
        """
    def resample(self, other: Spectrum1D, method: InterpolationMethod = ...) -> Spectrum1D:
        """
                  Resample a spectrum with a provided method.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The spectrum to be resampled.
                  method : hdrl.core.InterpolationMethod
                      The interpolation method used in resampling.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The resampled spectrum.
        """
    def resample_fit(self, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], k: typing.SupportsInt | typing.SupportsIndex, nCoeff: typing.SupportsInt | typing.SupportsIndex) -> Spectrum1D:
        """
                  Resample a spectrum on the wavelengths with B-spline fit.
        
                  Parameters
                  ----------
                  wavelengths: vector of float
                      The wavelengths the spectrum has to be resampled on.
                  k : int
                      The order of the B-spline.
                  nCoeff : int
                      The number of coefficients used for the fit.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The resampled spectrum.
        """
    def resample_integrate(self, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64]) -> Spectrum1D:
        """
                  Resample a spectrum on the wavelengths with integration.
        
                  Parameters
                  ----------
                  wavelengths : vector of float
                      The wavelengths the spectrum has to be resampled on.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The resampled spectrum.
        """
    def resample_to_wavelengths(self, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], method: InterpolationMethod = ...) -> Spectrum1D:
        """
                  Resample a spectrum on the wavelengths with a provided method.
        
                  Parameters
                  ----------
                  wavelengths: vector of float
                      The wavelengths the spectrum has to be resampled on.
                  method : hdrl.core.InterpolationMethod
                      The interpolation method used in resampling.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The resampled spectrum.
        """
    def resample_windowed_fit(self, wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], k: typing.SupportsInt | typing.SupportsIndex, nCoeff: typing.SupportsInt | typing.SupportsIndex, window: typing.SupportsInt | typing.SupportsIndex, factor: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        """
                  Resample a spectrum on the wavelengths with B-spline fit.
        
                  Parameters
                  ----------
                  wavelengths: vector of float
                      The wavelengths the spectrum has to be resampled on.
                  k : int
                      The order of the B-spline.
                  nCoeff : int
                      The number of coefficients used for the fit.
                  window : int
                      The number of destination wavelengths whose flux values
                      are computed using the same model.
                  factor : float
                      Given window2 = window * factor. window2 is the number of source
                      wavelengths used to compute the fit model.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The resampled spectrum.
        """
    def save(self, filename: str) -> None:
        """
                  Save the spectrum to file.
        
                  Parameters
                  ----------
                  filename : std.filesystem.path
                      The filename where spectrum will be saved.
        """
    def select_window(self, lambda_min: typing.SupportsFloat | typing.SupportsIndex, lambda_max: typing.SupportsFloat | typing.SupportsIndex, is_internal: bool = False) -> Spectrum1D:
        """
                  Selects or discards flux values according to whether the value of the
                  corresponding wavelength belongs to the interval [lambda_min,
                  lambda_max].
        
                  Parameters
                  ----------
                  lambda_min : double
                      The lower limit of the interval required for selection.
                  lambda_max : double
                      The upper limit of the interval required for selection.
                  is_internal : boolean
                      Specify if selection is internal to the interval
                      or external to the interval.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The selected subset of spectrum.
        """
    def sub_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise subtraction of a spectrum by a scalar.
                  Spectrum is modified.
        
                  Parameters
                  ----------
                  scalar : float
                      The scalar factor.
        """
    def sub_spectrum(self, other: Spectrum1D) -> None:
        """
                  Subtract two spectra.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        """
    def sub_spectrum_create(self, other: Spectrum1D) -> Spectrum1D:
        """
                  Subtract two spectra.
        
                  Parameters
                  ----------
                  other : hdrl.core.Spectrum1D
                      The other second factor.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    def wavelength_convert_to_linear(self) -> None:
        """
        Converts the wavelength scale to linear.
        """
    def wavelength_convert_to_linear_create(self) -> Spectrum1D:
        """
                   Converts the wavelength scale to linear.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       The modified copy of spectrum.
        """
    def wavelength_convert_to_log(self) -> None:
        """
        Converts the wavelength scale to log.
        """
    def wavelength_convert_to_log_create(self) -> Spectrum1D:
        """
                   Converts the wavelength scale to log.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1D
                       The modified copy of spectrum.
        """
    def wavelength_mult_scalar_linear(self, scale: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise multiplication of the scalar for the
                  wavelength.
        
                  Parameters
                  ----------
                  scale : float
                      The scalar factor.
        """
    def wavelength_mult_scalar_linear_create(self, scale: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        """
                  Computes the elementwise multiplication of the scalar for the
                  wavelength.
        
                  Parameters
                  ----------
                  scale : float
                      The scalar factor.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    def wavelength_shift(self, shift: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Computes the elementwise shift of the wavelength by the shift
                  parameter.
        
                  Parameters
                  ----------
                  shift : float
                      The shift scalar factor.
        """
    def wavelength_shift_create(self, shift: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1D:
        """
                  Computes the elementwise shift of the wavelength by the shift
                  parameter.
        
                  Parameters
                  ----------
                  shift : float
                      The shift scalar factor.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      The modified copy of spectrum.
        """
    @property
    def bad_pixel_map(self) -> numpy.typing.NDArray[numpy.int32]:
        """
        array of float: Bad pixel map
        """
    @property
    def flux(self) -> numpy.typing.NDArray[numpy.float64]:
        """
        array of float: Flux
        """
    @property
    def flux_error(self) -> numpy.typing.NDArray[numpy.float64]:
        """
        array of float: Error of flux
        """
    @property
    def scale(self) -> WaveScale:
        """
        string: Scale
        """
    @property
    def size(self) -> int:
        """
        int: Number of samples the 1D spectrum is made of
        """
    @property
    def wavelengths(self) -> numpy.typing.NDArray[numpy.float64]:
        """
        array of float: Wavelengths the spectrum is defined on
        """
class Spectrum1DList:
    """
    
          A hdrl.core.Spectrum1DList is a container for storing hdrl.core.Spectrum1D objects. It provides basic
          list management features. It corresponds to the HDRL spectrum1Dlist.
        
    """
    def __deepcopy__(self, arg0: dict) -> Spectrum1DList:
        ...
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Spectrum1D:
        ...
    @typing.overload
    def __init__(self) -> None:
        """
        Create an empty spectrum list
        """
    @typing.overload
    def __init__(self, arg0: collections.abc.Sequence[Spectrum1D]) -> None:
        """
        Create a spectrum list
        """
    @typing.overload
    def __init__(self, spectra: list) -> None:
        """
        Create a spectrum list from an array of spectra
        """
    def __len__(self) -> int:
        """
        int: number of spectra in the list
        """
    def __setitem__(self, index: typing.SupportsInt | typing.SupportsIndex, spectrum: Spectrum1D) -> None:
        """
        Assign a spectrum to a list element
        """
    def collapse(self, stacking_par: ..., wavelengths: typing.Annotated[numpy.typing.ArrayLike, numpy.float64], resample_par: Spectrum1DResampleMethod, mark_bpm_in_interpolation: bool = False) -> ...:
        """
                  Collapsing a hdrl.core.Spectrum1DList.
        
                  Parameters
                  ----------
                  stacking_par : hdrl.func.Collapse
                      Parameter regulating the stacking.
                  wavelengths : array of float
                      Wavelengths the resulting spectrum is defined on.
                  resample_par : Spectrum1DResampleMethod
                      Parameter regulating the resampling.
                  mark_bpm_in_interpolation : boolean, default = False
                      If true interpolated pixels whose neighbors (in the original
                      spectrum) are rejected, are not considered during collapsing.
        
                  Returns
                  -------
                  hdrl.core.CollapseResult
                      The collapse result object, containing the resulting spectrum,
                      output contribution mask, and resampled and aligned fluxes to be
                      collapsed.
        """
    def duplicate(self) -> Spectrum1DList:
        """
                   Create a duplicate of the spectrum list.
        
                   Returns
                   -------
                   hdrl.core.Spectrum1DList
                       A new copy of the Spectrum1DList.
        """
    def pop(self, index: typing.SupportsInt | typing.SupportsIndex | None = None) -> Spectrum1D:
        """
                  Remove and return the spectrum at the `index`.
        
                  Parameters
                  ----------
                  index : int, optional
                      Index of spectrum to remove from the list. If no index is given
                      the last spectrum in the list is removed.
        
                  Returns
                  -------
                  hdrl.core.Spectrum1D
                      Spectrum at `index`.
        
                  Raises
                  ------
                  IndexError
                      If the `index` is out of range.
        """
class Spectrum1DResampleMethod:
    @staticmethod
    def Fit(k: typing.SupportsInt | typing.SupportsIndex, n_coeff: typing.SupportsInt | typing.SupportsIndex) -> Spectrum1DResampleMethod:
        """
                  Constructor for the hdrl_parameter in the case of interpolation.
        
                  Parameters
                  ----------
                  k : int
                      The order of the B-spline.
                  n_coeff : int
                      The number of coefficients used for the fit.
        """
    @staticmethod
    def FitWindowed(k: typing.SupportsInt | typing.SupportsIndex, n_coeff: typing.SupportsInt | typing.SupportsIndex, window: typing.SupportsInt | typing.SupportsIndex, factor: typing.SupportsFloat | typing.SupportsIndex) -> Spectrum1DResampleMethod:
        """
                  Constructor for the hdrl_parameter in the case of interpolation.
        
                  Parameters
                  ----------
                  k : int
                      The order of the B-spline.
                  n_coeff : int
                      The number of coefficients used for the fit.
                  window : int
                      The number of destination wavelengths whose flux values are
                      computed using the same model.
                  factor : double
                      The given window2 = window * factor. window2 is the number of
                      source wavelengths used to compute the fit model.
        """
    @staticmethod
    def Integrate() -> Spectrum1DResampleMethod:
        """
                  Constructor for the hdrl_parameter in the case of integration.
        """
    @staticmethod
    def Interpolate(method: InterpolationMethod) -> Spectrum1DResampleMethod:
        """
                  Constructor for the hdrl_parameter in the case of interpolation.
        
                  Parameters
                  ----------
                  method : hdrl.core.InterpolationMethod
                      The interpolation methods.
        """
    def __repr__(self) -> str:
        ...
class TypeMismatchError(Error, RuntimeError):
    """
    Data were not of the expected type.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 19
class UnspecifiedError(Error, RuntimeError):
    """
    Unspecified error
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 1
class UnsupportedModeError(Error, RuntimeError):
    """
    The requested functionality is not supported.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See hdrl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 16
class WaveScale:
    """
    Members:
    
      LINEAR
    
      LOG
    """
    LINEAR: typing.ClassVar[WaveScale]  # value = <WaveScale.LINEAR: 0>
    LOG: typing.ClassVar[WaveScale]  # value = <WaveScale.LOG: 1>
    __members__: typing.ClassVar[dict[str, WaveScale]]  # value = {'LINEAR': <WaveScale.LINEAR: 0>, 'LOG': <WaveScale.LOG: 1>}
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
class XCorrelationResult:
    def __init__(self, shift: typing.SupportsFloat | typing.SupportsIndex, error: typing.SupportsFloat | typing.SupportsIndex, quality: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def __repr__(self) -> str:
        ...
    @property
    def error(self) -> float:
        """
        float: Estimated standard deviation of the correlation
        """
    @property
    def quality(self) -> float:
        """
        float: Mean squared error of the best fit
        """
    @property
    def shift(self) -> float:
        """
        float: Index where the cross correlation reaches its maximum, with sub-pixel precision
        """
class _Error_Data:
    __hash__: typing.ClassVar[None] = None
    @typing.overload
    def __eq__(self, arg0: _Error_Data) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getstate__(self) -> list[ErrorFrame]:
        ...
    @typing.overload
    def __init__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: str, arg2: str, arg3: typing.SupportsInt | typing.SupportsIndex, arg4: str) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: collections.abc.Iterable) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: _Error_Data) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: collections.abc.Sequence[ErrorFrame]) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def last(self) -> ErrorFrame:
        ...
    @property
    def trace(self) -> list[ErrorFrame]:
        ...
AKIMA: InterpolationMethod  # value = <InterpolationMethod.AKIMA: 2>
CSPLINE: InterpolationMethod  # value = <InterpolationMethod.CSPLINE: 1>
LINEAR: InterpolationMethod  # value = <InterpolationMethod.LINEAR: 0>
LOG: WaveScale  # value = <WaveScale.LOG: 1>
