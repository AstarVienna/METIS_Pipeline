"""
CPL Core submodule
  
  This module provides the core functionality of CPL. It provides the basic
  data types, like images and tables, the basic operations defined for
  these types, and also basic input and output operations. In addition
  the module provides utilities for basic image and signal processing,
  error handling, log message output.
  
"""
from __future__ import annotations
from abc import abstractmethod
import builtins as __builtins__
import collections.abc
from collections.abc import Collection
from collections.abc import MutableSequence
from collections.abc import Sequence
import cpl.ui
from inspect import getframeinfo
from inspect import stack
import numpy
import numpy as np
import numpy.typing
import typing
import cpl.core
__all__: list[str] = ['AccessOutOfRangeError', 'AssigningStreamError', 'BadFileFormatError', 'Bivector', 'Border', 'Collection', 'ContinueError', 'DataNotFoundError', 'DivisionByZeroError', 'DuplicatingStreamError', 'EOLError', 'Error', 'ErrorFrame', 'ErrorLostError', 'FileAlreadyOpenError', 'FileIOError', 'FileNotCreatedError', 'FileNotFoundError', 'Filter', 'FitMode', 'IllegalInputError', 'IllegalOutputError', 'Image', 'ImageList', 'ImageRow', 'IncompatibleInputError', 'InvalidTypeError', 'Kernel', 'Mask', 'Matrix', 'MatrixRow', 'Msg', 'MutableSequence', 'NoWCSError', 'NullInputError', 'Polynomial', 'Property', 'PropertyList', 'Sequence', 'SingularMatrixError', 'Sort', 'SortMode', 'Table', 'Type', 'TypeMismatchError', 'UnspecifiedError', 'UnsupportedModeError', 'Vector', 'VectorIterator', 'abstractmethod', 'getframeinfo', 'io', 'np', 'stack']
class AccessOutOfRangeError(Error, LookupError):
    """
    Data were accessed beyond boundaries.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 6
class Bivector:
    """
    
            Class for pairs of ordered sequences of numbers.
    
            A `cpl.core.Bivector` is composed of two `cpl.core.Vectors` of the same size.
            It can be used to store 1d functions, with the x and y positions of the samples,
            offsets in x and y or simply positions in an image.
            
            These Vectors are stored in properties `x` and `y`, however they can also be
            accessed using 0 and 1 indexes (and by extension through `__iter__`) for x and y
            respectfully.
    
            A Bivector can be created from any iterable object that containins two
            equal length sequences of floating point numbers. Examples include tuples
            containing two lists of numbers, lists containing two Vectors, and existing
            Bivectors.
    
            A Bivector can also be created using the zeros class method.
    
            Parameters
            ----------
            data : iterable of iterables of floats
                An iterable object which yields two items, both of which are iterables
                yielding an equal number of floating point values. 
    
            See Also
            --------
            cpl.core.Vector: Class for ordered sequences of numbers.
            cpl.core.Bivector.zeros: Create a Bivector of given length, initialised with 0's.
    
            Examples
            --------
            >>> bivector_list_of_tuples = cpl.core.Bivector([(1, 3, 5), (2, 4, 6)])
            ... bivector_tuple_of_lists = cpl.core.Bivector(([1, 2, 3], [4, 6, 8]))
            ... bivector_copy = cpl.core.Bivector(bivector_tuple_of_lists)
            ... vector_x = cpl.core.Vector((1, 2, 3))
            ... vector_y = cpl.core.Vector.zeros(3)        
            ... bivector_vectors = cpl.core.Bivector((vector_x, vector_y))
            ... bivector_zeros = cpl.core.Bivector.zeros(5)
        
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def read(filename: str) -> Bivector:
        """
                Read a list of values from an ASCII file and create a cpl_bivector
        
                The input ASCII file must contain two values per line.
        
                Two columns of numbers are expected in the input file.
        
                In addition to normal files, FIFO (see man mknod) are also supported.
        
                Parameters
                ----------
                filename : cpl.core.std::string
                    Name of the input ASCII file
        
                Returns
                -------
                cpl.core.Bivector
                    New Bivector with the values writen in the input ASCII file
                Raises
                ------
                cpl.core.FileIOError
                    if the file cannot be read
        """
    @staticmethod
    def zeros(size: typing.SupportsInt | typing.SupportsIndex) -> Bivector:
        """
                    Create a Bivector of given length, initialised with 0's.
        
                    Parameters
                    ----------
                    size : int
                        size of the new Bivector
        
                    Returns
                    -------
                    cpl.core.Biector
                        New cpl.core.Bivector, length `size`, initialised with 0's
        
                    Raises
                    ------
                    cpl.core.IllegalInputError
                        size is non-positive            
        """
    @typing.overload
    def __eq__(self, arg0: Bivector) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        ...
    @typing.overload
    def __init__(self, arg0: Bivector) -> None:
        ...
    @typing.overload
    def __init__(self, data: collections.abc.Iterable) -> None:
        ...
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.Any) -> Vector:
        ...
    def __str__(self) -> str:
        ...
    def copy(self) -> Bivector:
        """
                Copy the contents of the Bivector into a new Bivector object.
        
                Bivectors can also be copied by passing a Bivector to the
                Bivector constructor.
        
                Returns
                -------
                cpl.core.Bivector
                    New Bivector containing a copy of the contents of the original.
        
                See Also
                --------
                cpl.core.Bivector : Class for pairs of ordered sequences of numbers.
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a vector contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump bivector contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send bivector contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the bivector contents.
        
                Notes
                -----
                In principle a bivector can be saved using :py:meth:`dump` re-read using :py:meth:`read`.
                This will however introduce significant precision loss due to the limited
                accuracy of the ASCII representation.
        """
    def interpolate_linear(self, xout: Vector) -> Bivector:
        """
                Linear interpolation of a 1D-function
        
                Here `self` is interpreted as samples of a one dimensional function, with `x`
                containing abscissa values and `y` containing the corresponding ordinate values.
                The argument to this function, `xout`, is a `cpl.core.Vector` containing a set
                of abscissa values for which interpolated ordinate values are to be calculated.
                Linear interpolation is used to calculate the new ordinate values and the
                result is returned in a new `cpl.core.Bivector` object containing a copy of
                `xout` and the corresponding interpolated ordinate values.
        
                For each abscissa point in `xout`, `self.x` must either have two neigboring 
                abscissa points such that `self.x[i] < xout[j] < self.x[i+1]`, or a single
                identical abscissa point, such that `self.x[i] == xout[j]`. This is ensured
                by having monotonically increasing abscissa points in both `self.x` and `xout`,
                and by `min(self.x) <= min(xout)` and `max(xout) < max(self.x)`. However, for
                efficiency reasons (since `self.x` can be very long) the monotonicity is only
                verified to the extent necessary to actually perform the interpolation. This
                input requirement implies that extrapolation is not allowed.
        
                Parameters
                ----------
                xout : cpl.core.Vector
                    abcissa points to interpolate the ordinate values from `self` to.
        
                Returns
                -------
                cpl.core.Bivector
                      New Bivector containing the abscissa and ordinate values of the
                      interpolated function as `x` and `y` attributes.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if xout has an endpoint which is out of range
                cpl.core.IllegalInputError
                    if the monotonicity requirement on the 2 input abcissa Vectors is not met.
        """
    def sort(self, reverse: bool = False, mode: SortMode = ...) -> None:
        """
                Sort the Bivector in place.
        
                The values are sorted in either ascending or descending order, using either
                `x` or `y` as the key. The sorting is done in place, modifying the Bivector.
        
                Parameters
                ----------
                reverse : bool, default False
                    If `True` values will be sorted in descending order, otherwise they will
                    be sorted in ascending order.
                mode : cpl.core.SortMode
                    `cpl.core.SortMode.BY_X` to sort by the values in `x`, or
                    `cpl.core.SortMode.BY_Y` to sort by the values in `y`.
        
                Raises
                ------
                TypeError
                    if `mode` is neither `cpl.core.SortMode.BY_X` or `cpl.core.SortMode.BY_Y`
        
                See Also
                --------
                cpl.core.Bivector.sorted : Return a sorted copy of the Bivector.
        
                Notes
                -----
                If two members compare as equal their order in the sorted Bivector is undefined.
        """
    def sorted(self, reverse: bool = False, mode: SortMode = ...) -> Bivector:
        """
                Return a sorted copy of the Bivector.
        
                The values are sorted in either ascending or descending order, using either
                `x` or `y` as the key. The result is returned in a new `cpl.core.Bivector`,
                the original is not modified.
        
                Parameters
                ----------
                reverse : bool, default False
                    If `True` values will be sorted in descending order, otherwise they will
                    be sorted in ascending order.
                mode : cpl.core.SortMode
                    `cpl.core.SortMode.BY_X` to sort by the values in `x`, or
                    `cpl.core.SortMode.BY_Y` to sort by the values in `y`.
        
                Raises
                ------
                TypeError
                    if `mode` is neither `cpl.core.SortMode.BY_X` or `cpl.core.SortMode.BY_Y`
        
                See Also
                --------
                cpl.core.Bivector.sort : Sort the Bivector in place.
        
                Notes
                -----
                If two members compare as equal their order in the sorted Bivector is undefined.
        """
    @property
    def size(self) -> int:
        """
        Length of the bivector and in turn the length of x and y
        """
    @property
    def x(self) -> Vector:
        """
        x vector
        """
    @x.setter
    def x(self, arg1: typing.Any) -> Vector:
        ...
    @property
    def y(self) -> Vector:
        """
        y vector
        """
    @y.setter
    def y(self, arg1: typing.Any) -> Vector:
        ...
class Border:
    """
    Supported border modes for use with filtering functions in cpl.core.Image and cpl.core.Mask. For a kernel of width 2n+1, the n left- and rightmost image/mask columns do not have elements for the whole kernel. The same holds for the top and bottom image/mask rows.The border mode defines the filtering of such border pixels.
    
    Members:
    
      FILTER : Filter the border using the reduced number of pixels. If in median filtering the number of pixels is even choose the mean of the two central values, after the borders have been filled with a chess-like pattern of +- inf 
    
      ZERO : Set the border of the filtered image/mask to zero.
    
      CROP : Crop the filtered image/mask.
    
      NOP : Do not modify the border of the filtered image/mask.
    
      COPY : Copy the border of the input image/mask. For an in-place operation this has the no effect, identical to CPL_BORDER_NOP.
    """
    COPY: typing.ClassVar[Border]  # value = <Border.COPY: 4>
    CROP: typing.ClassVar[Border]  # value = <Border.CROP: 2>
    FILTER: typing.ClassVar[Border]  # value = <Border.FILTER: 0>
    NOP: typing.ClassVar[Border]  # value = <Border.NOP: 3>
    ZERO: typing.ClassVar[Border]  # value = <Border.ZERO: 1>
    __members__: typing.ClassVar[dict[str, Border]]  # value = {'FILTER': <Border.FILTER: 0>, 'ZERO': <Border.ZERO: 1>, 'CROP': <Border.CROP: 2>, 'NOP': <Border.NOP: 3>, 'COPY': <Border.COPY: 4>}
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
class ContinueError(Error, RuntimeError):
    """
    An iterative process did not converge.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 23
class Error(Exception, collections.abc.Sequence):
    """
    
    **Abstract** base class of all CPL exceptions,
    Do not instantiate this class, instead use cpl.core.NullInputError, cpl.core.InvalidArgumentError, or any other subclass.
    **However** this class implements has all documentation for those error
    subclasses.
    
    In order to copy a cpl error, where you do not know the type of the
    error, use the cpl.core.Error.create classmethod, as create can 
    dispatch to the relevant subclass.
    
    Examples
    --------
    .. code-block:: python
    
      try:
          # Some PyCPL functions are called here
      except cpl.core.IllegalInputError as e:
          print(str(e.message))
      except cpl.core.Error as e:
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
        
        The class that is returned is a subclass of cpl.core.Error
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
              Used internally to create the Error from C++ cpl::core::Error's
        
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
    def __getstate__(self) -> tuple:
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 9
class Filter:
    """
    Supported filter modes for use with filtering functions in cpl.core.Image and cpl.core.Mask.
    
    Members:
    
      EROSION : Erosion filter for cpl.core.Mask filtering (see cpl.core.Mask.filter)
    
      DILATION : Dilation filter for cpl.core.Mask filtering (see cpl.core.Mask.filter)
    
      OPENING : Opening filter for cpl.core.Mask filtering (see cpl.core.Mask.filter)
    
      CLOSING : Closing filter for cpl.core.Mask filtering (see cpl.core.Mask.filter)
    
      LINEAR : 
            A linear filter (for a cpl.core.Image.filter). The kernel elements are normalized
            with the sum of their absolute values. This implies that there must be
            at least one non-zero element in the kernel. The normalisation makes the
            kernel useful for filtering where flux conservation is desired.
    
            The kernel elements are thus used as weights like this::
    
                Kernel              Image
                                      ...
                1 2 3         ... 1.0 2.0 3.0 ...
                4 5 6         ... 4.0 5.0 6.0 ...
                7 8 9         ... 7.0 8.0 9.0 ...
                                      ...
    
            The filtered value corresponding to the pixel whose value is 5.0 is:
    
            .. math::
    
                \\frac{(1*1.0+2*2.0+3*3.0+4*4.0+5*5.0+6*6.0+7*7.0+8*8.0+9*9.0)} {1+2+3+4+5+6+7+8+9}
    
            Filtering with cpl.core.Filter.LINEAR and a flat kernel can be done faster with cpl.core.Filter.AVERAGE.
            
    
      LINEAR_SCALE : 
            A linear filter (for a cpl.core.Image.filter). Unlike cpl.core.Filter.LINEAR the kernel elements are not
            normalized, so the filtered image will have its flux scaled with the sum of the weights of the kernel.
            Examples of linear, scaling kernels are gradient operators and edge detectors.
    
            The kernel elements are thus applied like this::
    
                Kernel              Image
                                      ...
                1 2 3         ... 1.0 2.0 3.0 ...
                4 5 6         ... 4.0 5.0 6.0 ...
                7 8 9         ... 7.0 8.0 9.0 ...
                                      ...
    
            The filtered value corresponding to the pixel whose value is 5.0 is:
    
            .. math::
    
                1*1.0+2*2.0+3*3.0+4*4.0+5*5.0+6*6.0+7*7.0+8*8.0+9*9.0
    
            
    
      AVERAGE : 
            An average filter, i.e. the output pixel is the arithmetic average of the surrounding
            (1 + 2 * hsizex)(1 + 2 * hsizey) pixels. The cost per pixel is O(hsizex*hsizey).
    
            The two images may have different pixel types. When the input and output pixel types are identical,
            the arithmetic is done with that type, e.g. int for two integer images. When the input and output pixel
            types differ, the arithmetic is done in double precision when one of the two images have pixel type
            cpl.core.Type.DOUBLE, otherwise float is used.
            
    
      AVERAGE_FAST : 
            The same as cpl.core.Filter.AVERAGE, except that it uses a running average, which will lead to a significant
            loss of precision if there are large differences in the magnitudes of the input pixels. The cost per pixel
            is O(1) if all elements in the kernel are used, otherwise the filtering is done as for cpl.core.Filter.AVERAGE.
            
    
      MEDIAN : A median filter (for a cpl.core.Image). The pixel types of the input and output images must be identical.
    
      STDEV : 
            The filtered value is the standard deviation of the included input pixels::
    
                Kernel                      Image
                                            ...
                1   0   1           ... 1.0 2.0 3.0 ...
                0   1   0           ... 4.0 5.0 6.0 ...
                1   0   1           ... 7.0 8.0 9.0 ...
                                            ...
    
            The pixel with value 5.0 will have a filtered value of: std_dev(1.0, 3.0, 5.0, 7.0, 9.0)
            
    
      STDEV_FAST : 
            The same as cpl.core.Filter.STDEV, except that it uses the same running method employed in cpl.core.Filter.AVERAGE_FAST,
            which will lead to a significant loss of precision if there are large differences in the magnitudes of the input pixels.
            As with cpl.core.Filter.AVERAGE_FAST, the cost per pixel is O(1) if all elements in the kernel are used, otherwise the
            filtering is done as for cpl.core.Filter.AVERAGE.
            
    
      MORPHO : 
            A morphological filter (for a cpl.core.Image). The kernel elements are
            normalized with the sum of their absolute values. This implies that
            there must be at least one non-zero element in the kernel. The
            normalisation makes the kernel useful for filtering where flux
            conservation is desired.
    
            The kernel elements are used as weights on the sorted values covered by the kernel::
    
                Kernel                Image
                                      ...
                1 2 3         ... 4.0 6.0 5.0 ...
                4 5 6         ... 3.0 1.0 2.0 ...
                7 8 9         ... 7.0 8.0 9.0 ...
                                      ...
    
            The filtered value corresponding to the pixel whose value is 5.0 is:
            .. math::
    
                \\frac{(1*1.0+2*2.0+3*3.0+4*4.0+5*5.0+6*6.0+7*7.0+8*8.0+9*9.0)}{1+2+3+4+5+6+7+8+9}
    
            
    
      MORPHO_SCALE : 
            A morphological filter (for a cpl.core.Image). Unlike cpl.core.Filter.MORPHO
            the kernel elements are not normalized, so the filtered image will have
            its flux scaled with the sum of the weights of the kernel.
    
            The kernel elements are thus applied to the the sorted values covered by the kernel::
    
                Kernel                Image
                                      ...
                1 2 3         ... 4.0 6.0 5.0 ...
                4 5 6         ... 3.0 1.0 2.0 ...
                7 8 9         ... 7.0 8.0 9.0 ...
                                      ...
    
            The filtered value corresponding to the pixel whose value is 5.0 is:
            .. math::
    
                1*1.0+2*2.0+3*3.0+4*4.0+5*5.0+6*6.0+7*7.0+8*8.0+9*9.0
    
            
    """
    AVERAGE: typing.ClassVar[Filter]  # value = <Filter.AVERAGE: 6>
    AVERAGE_FAST: typing.ClassVar[Filter]  # value = <Filter.AVERAGE_FAST: 7>
    CLOSING: typing.ClassVar[Filter]  # value = <Filter.CLOSING: 3>
    DILATION: typing.ClassVar[Filter]  # value = <Filter.DILATION: 1>
    EROSION: typing.ClassVar[Filter]  # value = <Filter.EROSION: 0>
    LINEAR: typing.ClassVar[Filter]  # value = <Filter.LINEAR: 4>
    LINEAR_SCALE: typing.ClassVar[Filter]  # value = <Filter.LINEAR_SCALE: 5>
    MEDIAN: typing.ClassVar[Filter]  # value = <Filter.MEDIAN: 8>
    MORPHO: typing.ClassVar[Filter]  # value = <Filter.MORPHO: 11>
    MORPHO_SCALE: typing.ClassVar[Filter]  # value = <Filter.MORPHO_SCALE: 12>
    OPENING: typing.ClassVar[Filter]  # value = <Filter.OPENING: 2>
    STDEV: typing.ClassVar[Filter]  # value = <Filter.STDEV: 9>
    STDEV_FAST: typing.ClassVar[Filter]  # value = <Filter.STDEV_FAST: 10>
    __members__: typing.ClassVar[dict[str, Filter]]  # value = {'EROSION': <Filter.EROSION: 0>, 'DILATION': <Filter.DILATION: 1>, 'OPENING': <Filter.OPENING: 2>, 'CLOSING': <Filter.CLOSING: 3>, 'LINEAR': <Filter.LINEAR: 4>, 'LINEAR_SCALE': <Filter.LINEAR_SCALE: 5>, 'AVERAGE': <Filter.AVERAGE: 6>, 'AVERAGE_FAST': <Filter.AVERAGE_FAST: 7>, 'MEDIAN': <Filter.MEDIAN: 8>, 'STDEV': <Filter.STDEV: 9>, 'STDEV_FAST': <Filter.STDEV_FAST: 10>, 'MORPHO': <Filter.MORPHO: 11>, 'MORPHO_SCALE': <Filter.MORPHO_SCALE: 12>}
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
class FitMode:
    """
    Members:
    
      CENTROID
    
      STDEV
    
      AREA
    
      OFFSET
    
      ALL
    """
    ALL: typing.ClassVar[FitMode]  # value = <FitMode.ALL: 30>
    AREA: typing.ClassVar[FitMode]  # value = <FitMode.AREA: 8>
    CENTROID: typing.ClassVar[FitMode]  # value = <FitMode.CENTROID: 2>
    OFFSET: typing.ClassVar[FitMode]  # value = <FitMode.OFFSET: 16>
    STDEV: typing.ClassVar[FitMode]  # value = <FitMode.STDEV: 4>
    __members__: typing.ClassVar[dict[str, FitMode]]  # value = {'CENTROID': <FitMode.CENTROID: 2>, 'STDEV': <FitMode.STDEV: 4>, 'AREA': <FitMode.AREA: 8>, 'OFFSET': <FitMode.OFFSET: 16>, 'ALL': <FitMode.ALL: 30>}
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
class IllegalInputError(Error, ValueError):
    """
    Illegal values were detected.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 15
class Image:
    """
    
    
        A cpl.core.Image is a 2-dimensional data structure with a pixel type and an optional bad pixel map.
    
        The pixel indexing follows 0-indexing with the lower left corner having index (0, 0). The pixel
        buffer is stored row-wise so for optimum performance any pixel-wise access should be done likewise.
    
        Functionality include: FITS I/O Image arithmetic, casting, extraction, thresholding, filtering,
        resampling Bad pixel handling Image statistics Generation of test images Special functions, such as
        the image quality estimator.
    
        Supported cpl.core.Types:
    
        - cpl.core.Type.INT (32-bit integer)
        - cpl.core.Type.FLOAT
        - cpl.core.Type.DOUBLE
        - cpl.core.FLOAT_COMPLEX
        - cpl.core.DOUBLE_COMPLEX
    
        Parameters
        ----------
        data : iterable
          A 1d or 2d iterable containing image data to copy from, and either infers or in the case of a numpy array
          copies its type. Any iterable should be compatible as long as it implements python's buffer protocol
          with a SINGLE c-type per element, and an appropriate .dtype.
          If a 1d iterable is given, width must also be given to properly split the data into image rows.
        dtype : cpl.core.Type, optional
          Cast all pixels (numbers) in the array to given type to create the image.
          List must be homogenous sized. If not given the type will be extracted directly in the case of a
          numpy array or inferred.
        width : int, optional
          Width of the new image. This will split `data` into `width` sized rows to initialise the rows of
          the new image. Should only be given if `data` is 1d, otherwise a ValueError exception is thrown.
    
        Raises
        ------
        cpl.core.InvalidTypeError
            dtype is not a supported image type.
        cpl.core.IllegalInputError
            `data` is in an invalid format, or could not be reshaped with `width` widthg
        
    """
    class Normalise:
        """
        Members:
        
          SCALE
        
          MEAN
        
          FLUX
        
          ABSFLUX
        """
        ABSFLUX: typing.ClassVar[Image.Normalise]  # value = <Normalise.ABSFLUX: 3>
        FLUX: typing.ClassVar[Image.Normalise]  # value = <Normalise.FLUX: 2>
        MEAN: typing.ClassVar[Image.Normalise]  # value = <Normalise.MEAN: 1>
        SCALE: typing.ClassVar[Image.Normalise]  # value = <Normalise.SCALE: 0>
        __members__: typing.ClassVar[dict[str, Image.Normalise]]  # value = {'SCALE': <Normalise.SCALE: 0>, 'MEAN': <Normalise.MEAN: 1>, 'FLUX': <Normalise.FLUX: 2>, 'ABSFLUX': <Normalise.ABSFLUX: 3>}
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
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def create_gaussian(width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, dtype: Type, xcen: typing.SupportsFloat | typing.SupportsIndex, ycen: typing.SupportsFloat | typing.SupportsIndex, norm: typing.SupportsFloat | typing.SupportsIndex, sig_x: typing.SupportsFloat | typing.SupportsIndex, sig_y: typing.SupportsFloat | typing.SupportsIndex) -> Image:
        """
                Generate an image from a 2d gaussian function.
        
                This function generates an image of a 2d gaussian. The gaussian is
                defined by the position of its centre, given in pixel coordinates inside
                the image with the FITS convention (x from 0 to nx-1, y from 0 to ny-1), its
                norm and the value of sigma in x and y.
        
                .. math::
                      f(x, y) = (norm/(2*pi*sig_x*sig_y)) * exp(-(x-xcen)^2/(2*sig_x^2)) * exp(-(y-ycen)^2/(2*sig_y^2))
        
                Parameters
                ----------
                width : int
                    width of the new image
                height : int
                    height of the new image
                dtype : cpl.core.Type
                    type of the new image, must be cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE
                xcen : int, float
                    x position of the center
                ycen : int, float
                    y position of the center
                norm : int, float
                    norm of the gaussian.
                sig_x : int, float
                    sigma in x for the gaussian distribution.
                sig_y : int, float
                    sigma in y for the gaussian distribution.
        
                Returns
                -------
                cpl.core.Image
                    New Image containing the gaussian function.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If `self` is not of a supported image type.
        
                See Also
                --------
                cpl.core.Image.create_gaussian_like : Generate an image from a 2d gaussian function with width, height and type matching another image.
        """
    @staticmethod
    def create_gaussian_like(other: Image, xcen: typing.SupportsFloat | typing.SupportsIndex, ycen: typing.SupportsFloat | typing.SupportsIndex, norm: typing.SupportsFloat | typing.SupportsIndex, sig_x: typing.SupportsFloat | typing.SupportsIndex, sig_y: typing.SupportsFloat | typing.SupportsIndex) -> Image:
        """
                Generate an image from a 2d gaussian function with width, height and type matching another image.
        
                This function generates an image of a 2d gaussian. The gaussian is
                defined by the position of its centre, given in pixel coordinates inside
                the image with the FITS convention (x from 0 to nx-1, y from 0 to ny-1), its
                norm and the value of sigma in x and y.
        
                .. math::
                      f(x, y) = (norm/(2*pi*sig_x*sig_y)) * exp(-(x-xcen)^2/(2*sig_x^2)) * exp(-(y-ycen)^2/(2*sig_y^2))
        
                Parameters
                ----------
                other : cpl.core.Image
                    Other Image with the desired width, height and data type.
                xcen : int, float
                    x position of the center
                ycen : int, float
                    y position of the center
                norm : int, float
                    norm of the gaussian.
                sig_x : int, float
                    sigma in x for the gaussian distribution.
                sig_y : int, float
                    sigma in y for the gaussian distribution.
        
                Returns
                -------
                cpl.core.Image
                    New Image containing the gaussian function.
        
                See Also
                --------
                cpl.core.Image.create_gaussian :  Generate an image from a 2d gaussian function.
        """
    @staticmethod
    def create_jacobian(deltax: Image, deltay: Image) -> Image:
        """
                Compute area change ratio for a transformation map.
        
                Parameters
                ----------
                deltax : cpl.core.Image
                    x shift of each pixel
                deltay : cpl.core.Image
                    y shift of each pixel
        
                Returns
                -------
                cpl.core.Image
                    New Image containing the are change ratios
        
                Notes
                -----
                The shifts Images deltax and deltay, describing the transformation, must be of type cpl.core.Type.DOUBLE. For each pixel (u, v) of the
                output image, the deltax and deltay code the following transformation:
        
                u - deltax(u,v) = x
                v - deltay(u,v) = y
        
                This function writes the density of the (u, v) coordinate system relative to the (x, y) coordinates for each (u, v) pixel of image out.
        
                This is trivially obtained by computing the absolute value of the determinant of the Jacobian of the transformation for each pixel of
                the (u, v) image self.
        
                The partial derivatives are estimated at the position (u, v) in the following way:
        
                    dx/du = 1 + 1/2 ( deltax(u-1, v) - deltax(u+1, v) )
                    dx/dv =     1/2 ( deltax(u, v-1) - deltax(u, v+1) )
                    dy/du =     1/2 ( deltay(u-1, v) - deltay(u+1, v) )
                    dy/dv = 1 + 1/2 ( deltay(u, v-1) - deltay(u, v+1) )
        
                Typically this function would be used to determine a flux-conservation factor map for the target image specified in function warp().
        
                The map produced by this function is not applicable for flux conservation in case the transformation implies severe undersampling of the original signal.
        
                Raises
                ----------
                cpl.core.IllegalInputError
                    if the shift Images are not 2 dimensional
                cpl.core.InvalidTypeError
                    if the shift Images are not cpl.core.Type.DOUBLE type.
        """
    @staticmethod
    def create_jacobian_polynomial(width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, dtype: Type, poly_x: ..., poly_y: ...) -> Image:
        """
                Compute area change ratio for a 2D polynomial transformation.
        
                Parameters
                ----------
                width : int
                    width of the new image
                height : int
                    height of the new image
                dtype : cpl.core.Type
                    type of the new image, must be `cpl.core.Type.FLOAT` or `cpl.core.Type.DOUBLE`    
                poly_x : cpl.core.Polynomial
                    defines source x-pos corresponding to destination (u,v).
                poly_y : cpl.core.Polynomial
                    defines source y-pos corresponding to destination (u,v).
        
                Returns
                -------
                cpl.core.Image
                    New Image containing the computed area change ratios.
        
                Notes
                -----
                For an image with pixel coordinates (x, y) which is mapped into an output image with pixel coordinates (u, v), and the
                polynomial inverse transformation (u, v) to (x, y) as in warp_polynomial(), this function writes the density of the (u, v)
                coordinate system relative to the (x, y) coordinates for each (u, v) pixel of the output image.
        
                This is trivially obtained by computing the absolute value of the determinant of the Jacobian of the transformation for each
                pixel of the (u, v) self.
        
                Typically this function would be used to determine a flux-conservation factor map for the target image specified in function warp_polynomial().
        
                The map produced by this function is not applicable for flux conservation in case the transformation implies severe undersampling of the original signal.
        
                Raises
                ----------
                cpl.core.IllegalInputError
                  if the polynomial dimensions are not 2
                cpl.core.InvalidTypeError
                  if the image type is not supported
        
                See Also
                --------
                cpl.core.Image.create_jacobian_polynomial_like : Compute area change ratio for a 2D polynomial transformation with width, height and type matching another image.
        """
    @staticmethod
    def create_jacobian_polynomial_like(other: Image, poly_x: ..., poly_y: ...) -> Image:
        """
                Compute area change ratio for a 2D polynomial transformation with width, height and type matching another image.
        
                Parameters
                ----------
                other : cpl.core.Image
                    other Image with the desired width, height and data type. The type of `other` must be `cpl.core.Type.FLOAT` or `cpl.core.Type.DOUBLE` 
                poly_x : cpl.core.Polynomial
                    defines source x-pos corresponding to destination (u,v).
                poly_y : cpl.core.Polynomial
                    defines source y-pos corresponding to destination (u,v).
        
                Returns
                -------
                cpl.core.Image
                    New Image containing the computed area change ratios.
        
                Notes
                -----
                For an image with pixel coordinates (x, y) which is mapped into an output image with pixel coordinates (u, v), and the
                polynomial inverse transformation (u, v) to (x, y) as in warp_polynomial(), this function writes the density of the (u, v)
                coordinate system relative to the (x, y) coordinates for each (u, v) pixel of the output image.
        
                This is trivially obtained by computing the absolute value of the determinant of the Jacobian of the transformation for each
                pixel of the (u, v) self.
        
                Typically this function would be used to determine a flux-conservation factor map for the target image specified in function warp_polynomial().
        
                The map produced by this function is not applicable for flux conservation in case the transformation implies severe undersampling of the original signal.
        
                Raises
                ----------
                cpl.core.IllegalInputError
                  if the polynomial dimensions are not 2
                cpl.core.InvalidTypeError
                  if the image type is not supported
        
                See Also
                --------
                cpl.core.image.create_jacobian_polynomial : Compute area change ratio for a 2D polynomial transformation.
        """
    @staticmethod
    def create_noise_uniform(width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, type: Type, min_pix: typing.SupportsFloat | typing.SupportsIndex, max_pix: typing.SupportsFloat | typing.SupportsIndex) -> Image:
        """
                Create an image with uniform random noise distribution.
        
                Pixel values are within the provided bounds.
        
                Parameters
                ----------
                width : int
                    width of the new image
                height : int
                    height of the new image
                dtype : cpl.core.Type
                    type of the new image, must be `cpl.core.Type.INT`, `cpl.core.Type.FLOAT` or `cpl.core.Type.DOUBLE`   
                min_pix : float
                    minimum output pixel value.
                max_pix : float
                    maximum output pixel value.
        
                Returns
                -------
                cpl.core.Image
                    New image containing random noise.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If `min_pix` > `max_pix`
                cpl.core.InvalidTypeError
                    If the image is not of a supported image type.
        
                See Also
                --------
                cpl.core.Image.create_noise_uniform_like : Create an image with uniform random noise distribution with width, height and type matching another image.
        """
    @staticmethod
    def create_noise_uniform_like(other: Image, min_pix: typing.SupportsFloat | typing.SupportsIndex, max_pix: typing.SupportsFloat | typing.SupportsIndex) -> Image:
        """
                Create an image with uniform random noise distribution with width, height and type matching another image.
        
                Pixel values are within the provided bounds.
        
                Parameters
                ----------
                other : cpl.core.Image
                    other Image with the desired width, height and data type. The type of `other` must be
                    `cpl.core.Type.INT`, `cpl.core.Type.FLOAT` or `cpl.core.Type.DOUBLE`   
                min_pix : float
                    minimum output pixel value.
                max_pix : float
                    maximum output pixel value.
        
                Returns
                -------
                cpl.core.Image
                    New image containing random noise.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If `min_pix` > `max_pix`
                cpl.core.InvalidTypeError
                    If the image is not of a supported image type.
        
                See Also
                --------
                cpl.core.Image.create_noise_uniform : Create an image with uniform random noise distribution.
        """
    @staticmethod
    def from_accepted(image_list: ...) -> Image:
        """
                Create a contribution map from the bad pixel maps of the images.
        
                The returned map counts for each pixel the number of good pixels in the list.
        
                Parameters
                ----------
                imlist : cpl.core.ImageList
                    Images to generate a contribution map from.
        
                Returns
                -------
                cpl.core.Image
                    Output contribution map with the pixel type cpl.core.Type.INT
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the input image list is not valid
        """
    @staticmethod
    def hypot(first: Image, second: Image) -> Image:
        """
                The pixel-wise Euclidean distance between two images.
        
                The Euclidean distance function is useful for gaussian error propagation
                on addition/subtraction operations.
        
                For pixel values a and b the Euclidean distance c is defined as:
                :math:'c = \\sqrt{a^2 + b^2}'
        
                If both input operands are of type cpl.core.Type.FLOAT the distance is computed
                in single precision, otherwise in double precision.
        
                Parameters
                ----------
                first : cpl.core.Image
                    First input image. Must be type `cpl.core.Type.CPL_TYPE_FLOAT` or
                    `cpl.core.Type.CPL_TYPE_DOUBLE`.
                second : cpl.core.Image
                    Second input image. Must be type `cpl.core.Type.CPL_TYPE_FLOAT` or
                    `cpl.core.Type.CPL_TYPE_DOUBLE`.
        
                Returns
                -------
                cpl.core.Image
                  A new Image containing the Euclidean distance between `first` and `second`.
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the images have different sizes        
                cpl.core.InvalidType
                    if the images are not both CPL_TYPE_FLOAT or CPL_TYPE_DOUBLE type.
        """
    @staticmethod
    def labelise_create(mask: typing.Any) -> tuple[Image, int]:
        """
                Labelise a mask to differentiate different objects
        
                Separate the given mask into contiguous regions, labelling each region
                a different number. 0 Doesn't count as a region.
        
                Labelises all blobs: 4-neighbor connected zones set to 1 in this mask
                will end up in the image as zones where all pixels are the same, unique
                integer.
        
                Parameters
                ----------
                mask : cpl.core.Mask
                    mask to labelise
        
                Returns
                -------
                tuple(cpl.core.Image, int)
                    The image making up the labelled regions, and the amount of regions.
        
                Notes
                -----
                Non-recursive flood-fill is applied to label the zones, dimensioned by the
                number of lines in the image, and the maximal number of lines possibly
                covered by a blob.
        """
    @staticmethod
    def load(filename: str, dtype: Type = ..., extension: typing.SupportsInt | typing.SupportsIndex = 0, plane: typing.SupportsInt | typing.SupportsIndex = 0, area: tuple = None) -> Image:
        """
                            Load an image from a file.
        
                            Load image data from the extension `extension` of the FITS
                            file `filename`. The FITS extenstion may be an image
                            (``NAXIS`` = 2) or a data cube (``NAXIS`` = 3). By default
                            the image is read from the primary HDU of the FITS file.
        
                            When the specified extension is a data cube, the slice of
                            the data cube to load may be given by `plane`. By default
                            the first plane is loaded.
        
                            By default the whole image is loaded. If a particular
                            region of an image should be loaded, the region to load
                            may be provided by the argument `area`.
        
                            The argument `dtype` specifies the pixel data type of the
                            result image. When the image is loaded the pixel data type
                            in the input FITS file is converted into `dtype`. By default
                            the image data of the input file is converted to
                            cpl.core.Type.DOUBLE. To load the image without converting
                            the pixel data type use cpl.core.Type.UNSPECIFIED.
        
                            Valid pixel data types which may be used for `dtype` are:
        
                            - cpl.core.Type.INT (32-bit integer)
                            - cpl.core.Type.FLOAT
                            - cpl.core.Type.DOUBLE
        
                            Parameters
                            ----------
                            filename : str
                              Name of the input file
                            dtype : cpl.core.Type, default=cpl.core.Type.DOUBLE
                              Data type of the pixels of the returned image
                            extension : int, default=0
                              Index of the FITS extension to load (the primary data unit
                              has index 0)
                            plane : int, default=0
                              Index of the plane of a data cube to load (counting
                              starts from 0)
                            area : Tuple, default=None
                              Region of interest to load given as a tuple specifying
                              the lower left x, the lower left y, the upper right x (inclusive)
                              and the upper right y coordinate (inclusive) in this order.
                              Numbering of the pixel x and y positions starts at 0
                              (pycpl convention). If `None` then the entire image will be loaded.
        
        
                            Returns
                            -------
                            cpl.core.Image
                              New image instance of loaded data
        
                            Raises
                            ------
                            cpl.core.FileIOError
                              If the file cannot be opened, or does not exist.
                            cpl.core.BadFileFormatError
                              If the data cannot be loaded from the file.
                            cpl.core.InvalidTypeError
                              If the requested pixel data type is not supported.
                            cpl.core.IllegalInputError
                              If the requested extension number is invalid (negative),
                              the plane number is out of range, or if the given image region
                              is invalid.
                            cpl.core.DataNotFoundError
                              If the specified extension has no image data.
        """
    @staticmethod
    def zeros(width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, dtype: Type) -> Image:
        """
              Create an image of width × height dimensions, all 0’s, as type dtype
        
              Parameters
              ----------
              width : int
                  width of the new image
              height : int
                  height of the new image
              dtype : cpl.core.Type
                  Type of the new image (see supported cpl.core.Types in class summary)
        
              Returns
              -------
              cpl.core.Image
                  New width x height image of dtype initialised with all 0’s
        
              Raises
              ------
              cpl.core.InvalidTypeError
                  dtype is not a supported image type.
        """
    @staticmethod
    def zeros_like(other: Image) -> Image:
        """
              Create an image filled with 0's with width, height and type matching another image.
        
              Parameters
              ----------
              other : cpl.core.Image
                  Other Image with the desired width, height and data type.
        
              Returns
              -------
              cpl.core.Image
                  New Image initialised with all 0’s
        """
    def __array__(self, **kwargs) -> numpy.ndarray:
        ...
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __bytes__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> bytes:
        ...
    def __contains__(self, arg0: typing.Any) -> bool:
        ...
    def __deepcopy__(self, arg0: dict) -> Image:
        ...
    def __eq__(self, arg0: Image) -> bool:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> ImageRow:
        ...
    def __init__(self, data: collections.abc.Iterable, dtype: cpl.core.Type | None = None, width: typing.SupportsInt | typing.SupportsIndex | None = None) -> None:
        ...
    def __iter__(self) -> typing.Any:
        ...
    def __len__(self) -> int:
        ...
    def __next__(self) -> ImageRow:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def __repr__(self) -> str:
        ...
    def __reversed__(self) -> None:
        """
        Unsupported
        """
    def __str__(self) -> str:
        ...
    def abs(self) -> None:
        """
                Take the absolute value of an image. Modified in place.
        
                Set each pixel to its absolute value.
        """
    def accept(self, y: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Set a pixel as good in an image
        
                Parameters
                ----------
                y : int
                    the y pixel position in the image (first pixel is 0)
                y : int
                    the x pixel position in the image (first pixel is 0)
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    if the specified position is outside of image `self`
        """
    def accept_all(self) -> None:
        """
        Set all pixels in the image as good
        """
    def add(self, other: Image) -> None:
        """
                Adds values from Image other to self. Modified in place.
        
        
                The bad pixel map of the `self` becomes the union of the bad pixel
                maps of the input images.
        
                Parameters
                ----------
                other : cpl.core.Image
                    Image to add to `self`
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the input images have different sizes
                cpl.core.TypeMismatchError
                    if the `other` has complex type
        """
    def add_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise addition of a scalar to an image. Modified in place.
        
                Modifies the image by adding a number to each of its pixels.
        
                The operation is always performed in double precision, with a final
                cast of the result to the image pixel type.
        
                Parameters
                ----------
                scalar : float
                    Number to add
        """
    def and_scalar(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                The bit-wise AND of a scalar and an image with integer pixels. Modified in place.
        
                Parameters
                ----------
                value : int
                    scalar value to bitwise AND with the pixels
        
                Notes
                -----
                cpl.core.Type.INT is required
        """
    def and_with(self, other: Image) -> None:
        """
                Takes the bit-wise AND of the image with another image, pixel by pixel.
        
                Both images must be integer type. The AND is done in place, overwriting the
                original image.
        
                Parameters
                ----------
                other : cpl.core.Image
                    Second operand
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the images have different sizes
                cpl.core.InvalidTypeError
                    If either image type is not cpl.core.Type.INT
        """
    def as_array(self) -> typing.Any:
        """
                Returns a copy of the Image as a numpy array.
        
                Returns
                -------
                numpy.ndarray
                    New numpy array containing the pixel values from the Image. The data type
                    of the array will be the same as the data type of the Image.  
        """
    def cast(self, dtype: Type) -> Image:
        """
                Returns a copy of the image converted to a given data type.
        
                Casting to non-complex types is only supported for non-complex types.
        
                Parameters
                ----------
                dtype : cpl.core.Type
                    The destination type
        
                Returns
                -------
                cpl.core.Image
                    New image that is a copy of the original image converted to the given `dtype`
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the passed type is invalid
                cpl.core.TypeMismatchError
                    If the image type is complex and requested casting type is non-complex.
        
                See Also
                --------
                cpl.core.Image.to_type : Converts an image to a given type. Modified in place.
        """
    def conjugate(self) -> None:
        """
                Complex conjugate the pixels in a complex image. Modified in place.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the image is not of a complex type
        """
    def copy_into(self, other: Image, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Copy one image into another
        
                (ypos, xpos) must be a valid position in `self`. If `other` is bigger than the place
                left in `self`, the part that falls outside of `self` is simply ignored, an no
                error is raised.
                The bad pixels are inherited from `other` in the concerned `self` zone.
        
                The two input images must be of the same type, namely one of
                cpl.core.Type.INT, cpl.core.Type.FLOAT, cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                other : cpl.core.Image
                    the inserted image
                ypos : int
                    the y pixel position in `self` where the lower left pixel of
                    `other` should go (from 0 to the y-1 size of `self`)
                xpos : int
                    the x pixel position in `self` where the lower left pixel of
                    `other` should go (from 0 to the x-1 size of `self`)
        
                Raises
                ------
                cpl.core.TypeMismatchError
                    if the input images are of different types
                cpl.core.InvalidTypeError
                    if the image type is not supported
                cpl.core.AccessOutOfRangeError
                    if xpos or ypos are outside the specified range
        
                Notes
                -----
                The two pixel buffers may not overlap
        """
    def count_rejected(self) -> int:
        """
                Count the number of bad pixels declared in an image
        
                Returns
                -------
                int
                    the number of bad pixels
        """
    def divide(self, other: Image) -> None:
        """
                Divide `self` by another image
        
                Parameters
                ----------
                other : cpl.core.Image
                    image to divide with
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the input images have different sizes
                cpl.core.TypeMismatchError
                    if the second input image has complex type
        
                Notes
                -----
                The result of division with a zero-valued pixel is marked as a bad pixel.
        """
    def divide_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise division of an image with a scalar
        
                Modifies the image by dividing each of its pixels with a number.
        
                Parameters
                ----------
                scalar : float
                    Non-zero number to divide with
        
                Raises
                ------
                cpl.core.DivsionByZeroError
                    scalar is 0.0
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', window: tuple | None = None, show: bool | None = True) -> str:
        """
                    Dump the image contents to a file, stdout or a string.
        
                    This function is intended just for debugging. It prints the contents of an image
                    to the file path specified by `filename`. 
                    If a `filename` is not specified, output goes to stdout (unless `show` is False). 
                    In both cases the contents are also returned as a string.
        
                    Parameters
                    ----------
                    filename : str, optional
                        File to dump image contents to
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
                    -------
                    str 
                        Multiline string containing the dump of the image contents.
        """
    def duplicate(self) -> Image:
        """
                Copy the image.
        
                Copy the image into a new image object. The pixels and the bad pixel map are also copied.
        
                This method is also used when performing a deepcopy on an image.
        
                Returns
                -------
                cpl.core.Image
                    New image object that is a copy of the original image.
        """
    def equals(self, other: Image) -> bool:
        """
            Check if one image is equivalent to another.
        
            Two images are considered equal if they share the same dimensions, type, and values (in the same positions as each other).
        
            Can also be called using the equality operator i.e. `self`==`other`
        
            Parameters
            ----------
            - other : cpl.core.Image
                Image to compare to `self`.
        
            Return
            ------
            bool
                True if `self` is equal to `other`. False otherwise.
        
            Notes
            -----
            In comparison to numpy array equality, this function is more strict in that the properties of the array (type and dimensions)
            need to be equal, contrary to numpy which does elementwise comparisons and does not require the arrays to be of the same
            data type. Numpy functions are still however compatible with images as input arguments for numpy and thus equality functions
            can be used with the images (e.g. `np.array_equals(im1, im2)`)
        """
    def exponential(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
              Compute the elementwise exponential of the image.
        
              Modifies the image by computing the base-scalar exponential of each of its
              pixels.
        
              Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
              Pixels for which the power of the given base is not defined are
              rejected and set to zero.
        
              Parameters
              ----------
              base : float
                  Base of the exponential.
        """
    def extract(self, window: tuple) -> Image:
        """
                Extract a rectangular zone from an image into another image.
        
                The input coordinates define the extracted region by giving the coordinates
                of the lower left and upper right corners (inclusive).
        
                Coordinates must be provided in the FITS convention: lower left
                corner of the image is at (1,1), x increasing from left to right,
                y increasing from bottom to top.
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                If the input image has a bad pixel map and if the extracted rectangle has
                bad pixel(s), then the extracted image will have a bad pixel map, otherwise
                it will not.
        
                Parameters
                ----------
                window : tuple(int,int,int,int)
                  Window in the format (llx, lly, urx, ury) where:
                  - `llx` Lower left X coordinate
                  - `lly` Lower left Y coordinate
                  - `urx` Upper right X coordinate
                  - `ury` Upper right Y coordinate
        
                Returns
                -------
                cpl.core.Image
                    New image instance of the extracted area.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the window coordinates are not valid
        """
    def extract_subsample(self, xstep: typing.SupportsInt | typing.SupportsIndex, ystep: typing.SupportsInt | typing.SupportsIndex) -> Image:
        """
                Sub-sample an image
        
                step represents the sampling step in x and y: both steps = 2 will create an
                image with a quarter of the pixels of the input image.
        
                image type can be cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
                If the image has bad pixels, they will be resampled in the same way.
        
                The flux of the sampled pixels will be preserved, while the flux of the
                pixels not sampled will be lost. Using steps = 2 in each direction on a
                uniform image will thus create an image with a quarter of the flux.
        
                Parameters
                ----------
                ystep : int
                    Take every xstep pixel in y
                xstep : int
                    Take every ystep pixel in x
        
                Returns
                -------
                cpl.core.Image
                    New sub-sampled image
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if xstep, ystep are not positive
                cpl.core.InvalidTypeError
                    if the image type is not supported
        """
    def fft(self, imag: cpl.core.Image | None = None, inverse: bool = False, unnormalized: bool = False, swap_halves: bool = False) -> Image:
        """
                Fast Fourier Transform a square, power-of-two sized image. Modified in place.
        
                `self` must be either of type cpl.core.Type.DOUBLE_COMPLEX or cpl.core.Type.DOUBLE.
                If `self` is passed as cpl.core.Type.DOUBLE, the imaginary component can be passed via `imag`, which must also be
                cpl.core.Type.DOUBLE. `imag` is unused otherwise.
        
                Any rejected pixel is used as if it were a good pixel.
        
                The image must be square with a size that is a power of two.
        
                Different FFT options can be set via the kwargs (see Parameters).
        
                Parameters
                ----------
                imag : cpl.core.Image, optional
                    The imaginary part of the image. Only used when the image's type is cpl.core.Type.DOUBLE
                    If not given, a 0 value image will be set in its place.
                inverse : bool, optional
                    True to perform Inverse FFT transform
                unnormalized : bool, optional
                    True to not normalize (with N*N for N-by-N image) on `inverse` = False. Has no effect on forward transform (`inverse` = True).
                swap_halves : bool, optional
                    Swap the four quadrants of the result image.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the image is not square or if the image size is not a power of 2.
                cpl.core.UnsupportedModeError
                    if mode is otherwise different from the allowed FFT options.
                cpl.core.InvalidTypeError
                    if the passed image type is not supported.
        
                Warning
                -------
                When comparing to cpl.drs.fft.fft_image and numpy's fft, with any image
                dimensions greater than or equal to 4x4 the values of the imaginary
                component of the resulting image is sign flipped. This is due to the differing implementation of
                the fft algorithm which is being looked into.
        
                If possible is recommended to use cpl.drs.fft.fft_image as its a more up to date
                and well more maintained implementation of fft using fftw.
        """
    def fill_rejected(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Set the bad pixels in an image to a fixed value.
        
                Images can be of type cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE
        
                Parameters
                ----------
                value : int, float
                    Value to replace bad pixels
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the image's type is not supported, i.e. `cpl.core.Type.FLOAT_COMPLEX` or `cpl.core.Type.DOUBLE_COMPLEX`
        """
    def fill_window(self, window: tuple, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Fill an image window with a constant
        
                Any bad pixels in the window are accepted.
        
                Upper boundaries are inclusive and will also be filled with `value`.
        
                Images can be of type cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE
        
                Parameters
                ----------
                window : tuple(int,int,int,int)
                  Window to fill with `value` in the format (llx, lly, urx, ury) where:
                  - `llx` Lower left X coordinate
                  - `lly` Lower left Y coordinate
                  - `urx` Upper right X coordinate (inclusive)
                  - `ury` Upper right Y coordinate (inclusive)
                value : float
                    Value to fill with
        
                Raises
                ------
                cpl.core.IllegalInputError
                    The specified window is not valid
        """
    def filter(self, kernel: ..., filter: Filter, border: Border = ..., dtype: Type = ...) -> Image:
        """
                Filter the image using a floating-point kernel
        
                The kernel must have an odd number of rows and an odd number of columns and at least one non-zero element.
        
                For scaling filters (cpl.core.Filter.LINEAR_SCALE and cpl.core.Filter.MORPHO_SCALE) the flux of the filtered image will be
                scaled with the sum of the weights of the kernel. If for a given input pixel location the kernel covers only bad
                pixels, the filtered pixel value is flagged as bad and set to zero.
        
                For flux-preserving filters (cpl.core.Filter.LINEAR and cpl.core.Filter.MORPHO) the filtered pixel must have at least one input
                pixel with a non-zero weight available. Output pixels where this is not the case are set to zero and flagged as bad.
        
                Supported pixel types are: cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
        
                Supported filters: cpl.core.Filter.LINEAR, cpl.core.Filter.MORPHO, cpl.core.Filter.LINEAR_SCALE and cpl.core.Filter.MORPHO_SCALE
        
                The result is returned in a new Image.
        
                Parameters
                ----------
                kernel : cpl.core.Matrix
                    Pixel weights
                filter : cpl.core.Filter
                    cpl.core.Filter.LINEAR or cpl.core.Filter.MORPHO, cpl.core.Filter.LINEAR_SCALE and cpl.core.Filter.MORPHO_SCALE
                border : cpl.core.Border, optional
                    Filtering border mode. Currently only supports cpl.core.Border.FILTER and thus is set to that by default
                dtype : cpl.core.Type
                    Data type to use for the output image, can be cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                Returns
                -------
                cpl.core.Image
                    The filtered image.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the kernel has a side of even length.
                cpl.core.DivisionByZeroError
                    If the kernel is a zero-matrix.
                cpl.core.AccessOutOfRangeError
                    If the kernel has a side longer than the input image.
                cpl.core.InvalidTypeError
                    if the image type is not supported.
                cpl.core.TypeMismatchError
                    if in median filtering the input and output pixel types differ.
                cpl.core.UnsupportedModeError
                    If the output pixel buffer overlaps the input one (or the kernel), or the border/filter mode is unsupported.
        
                See Also
                --------
                cpl.core.Image.filter_mask : For filtering using a binary kernel (cpl.core.Mask)
        """
    def filter_mask(self, kernel: typing.Any, filter: Filter, border: Border, dtype: Type) -> Image:
        """
                Filter an image using a binary kernel
        
                Parameters
                ----------
                kernel : cpl.core.Mask
                    Mask of Pixels to use
                filter : cpl.core.Filter
                    Filter to use, can be:
                    cpl.core.Filter.MEDIAN, cpl.core.Filter.AVERAGE and more, see notes
                border :
                    border to use, can be cpl.core.Border.FILTER and more, see Notes
                dtype : cpl.core.Type
                    Data type to use for the output image. Can be cpl.core.Type.INT, cpl.core.Type.FLOAT
                    or cpl.core.Type.DOUBLE but see Notes for restrictions.
        
                Returns
                -------
                cpl.core.Image
                    The filtered image.
        
                Notes
                -----
                The kernel must have an odd number of rows and an odd number of columns.
        
                The output image will have equal dimensions to the original image, except
                for the border mode CPL_BORDER_CROP, where the output image must have 
                2 * hx columns fewer and 2 * hy rows fewer than the original image,
                where the kernel has size (1 + 2 * hx, 1 + 2 * hy).
        
                In standard deviation filtering the kernel must have at least two elements
                set to True, for others at least one element must be set to
                True.
        
                Supported pixel types are: cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
        
                In median filtering the two images must have the same pixel type.
        
                In standard deviation filtering a filtered pixel must be computed from at
                least two input pixels, for other filters at least one input pixel must be
                available. Output pixels where this is not the case are set to zero and
                flagged as rejected.
        
                In-place filtering is not supported.
        
                Supported modes:
        
                cpl.core.Filter.MEDIAN:
                cpl.core.Border.FILTER, cpl.core.Border.ZERO, cpl.core.Border.COPY, cpl.core.Border.CROP.
        
                cpl.core.Filter.AVERAGE:
                cpl.core.Border.FILTER
        
                cpl.core.Filter.AVERAGE_FAST:
                cpl.core.Border.FILTER
        
                cpl.core.Filter.STDEV:
                cpl.core.Border.FILTER
        
                cpl.core.Filter.STDEV_FAST:
                cpl.core.Border.FILTER
        
                Note that in PyCPL the supported border modes for median filtering includes
                `ZERO` but not `NOP` as in CPL's `cpl_image_filter_mask`. This is because the
                `NOP` mode preserves pixel values from the border regions of a pre-allocated
                results Image, but this method uses a new Image to store the results so there
                are no pre-existing pixel values to preserve. See PIPE-11042 for more details.
        
                To shift an image 1 pixel up and 1 pixel right with the cpl.core.Filter.MEDIAN
                filter and a 3 by 3 kernel, one should set to CPL_BINARY_1 the bottom
                leftmost kernel element - at row 3, column 1, i.e.
        
                .. code-block:: python
        
                  kernel=cpl.core.Mask(3,3)
                  kernel[0][0] = True
        
                The kernel required to do a 5 x 5 median filtering is created like this:
        
                .. code-block:: python
        
                  kernel=~cpl.core.Mask(5,5)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the kernel has a side of even length.
                cpl.core.DataNotFoundError
                    If the kernel is empty, or in case of cpl.core.Filter.STDEV if the kernel has only one element set to True.
                cpl.core.AccessOutOfRangeError
                    If the kernel has a side longer than the input image.
                cpl.core.InvalidTypeError
                    if the image type is not supported.
                cpl.core.TypeMismatchError
                    if in median filtering the input and output pixel types differ.
                cpl.core.UnsupportedModeError
                    If the output pixel buffer overlaps the input one (or the kernel), or the border/filter mode is unsupported.
        """
    def flip(self, angle: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Flip an image on a given mirror line.
        
                This function operates locally on the pixel buffer.
        
                angle can take one of the following values:
                - 0 (theta=0) to flip the image around the horizontal
                - 1 (theta=pi/4) to flip the image around y=x
                - 2 (theta=pi/2) to flip the image around the vertical
                - 3 (theta=3pi/4) to flip the image around y=-x
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                angle : int
                    mirror line in polar coord. is theta = (PI/4) * angle
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the angle is different from the allowed values
                cpl.core.InvalidTypeError
                    if the image type is not supported
        """
    def get_absflux(self, window: tuple | None = None) -> float:
        """
                Computes the sum of absolute values over an entire image or sub window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the absolute flux (sum of pixels) value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                cpl.core.InvalidTypeError
                    If the image they are called on has complex data type.
        """
    def get_centroid_x(self, window: tuple | None = None) -> float:
        """
                Computes the x centroid value over the whole image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
        
                Returns
                -------
                float
                    the x centroid value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                cpl.core.InvalidTypeError
                    If the image they are called on has complex data type.
        
                See Also
                --------
                cpl.core.Image.get_centroid_y : Compute the y centroid value over the whole image or sub-window.
        """
    def get_centroid_y(self, window: tuple | None = None) -> float:
        """
                Computes the y centroid value over the whole image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
        
                Returns
                -------
                float
                    the y centroid value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                cpl.core.InvalidTypeError
                    If the image they are called on has complex data type.
                
                See Also
                --------
                cpl.core.Image.get_centroid_x : Compute the x centroid value over the whole image or sub-window.
        """
    def get_flux(self, window: tuple | None = None) -> float:
        """
                Computes the sum of pixel values over an entire image or sub window
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the flux (sum of pixels) value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                Notes
                -----
                Does not work on complex images.
        """
    def get_fwhm(self, ypos: typing.SupportsInt | typing.SupportsIndex, xpos: typing.SupportsInt | typing.SupportsIndex) -> tuple:
        """
                Compute the FWHM of an object in a cpl.core.Vector
        
                For the FWHM in x (resp. y) to be computed, the image size in the x (resp.
                y) direction should be at least of 5 pixels.
        
                If for any reason, one of the FHWMs cannot be computed, its returned value
                is None with no exception raised. For example, if a 4 column image is passed,
                the x component of the return tuple would be None, while the y component
                would be correctly computed, and no exception would be raised.
        
                Parameters
                ----------
                ypos : int
                    the y position of the object (0 for the first pixel)
                xpos : int
                    the x position of the object (0 for the first pixel)
                Returns
                -------
                tuple(float, float)
                    fwhm y, x, which are the computed FWHM in y or x directions
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if (`ypos`, `xpos`) specifies a rejected pixel or a pixel with a non-positive value
                cpl.core.AccessOutOfRangeError
                    if (`ypos` or `xpos`) is outside the image size range
        
                Notes
                -----
                The return value may be None with no error condition
        
                This function uses a basic method: start from the center of the object
                and go away until the half maximum value is reached in x and y.
        """
    def get_interpolated(self, ypos: typing.SupportsFloat | typing.SupportsIndex, xpos: typing.SupportsFloat | typing.SupportsIndex, yprofile: ..., yradius: typing.SupportsFloat | typing.SupportsIndex, xprofile: ..., xradius: typing.SupportsFloat | typing.SupportsIndex) -> tuple[float, float]:
        """
            Interpolate a pixel
        
            Parameters
            ----------
            ypos : int
              Pixel y floating-point position (FITS convention)
            xpos : int
              Pixel x floating-point position (FITS convention)
            yprofile : cpl.core.Vector
              Interpolation weight vector as a function of the distance in Y
            yradius : float
              Positive inclusion radius in the Y-dimension
            xprofile :cpl.core.Vector
              Interpolation weight as a function of the distance in X
            xradius : float
              Positive inclusion radius in the X-dimension
        
            Returns
            -------
            tuple(float, float)
              Tuple of (interpolated, confidence), where `interpolated` represents the
              interpolated pixel value and `confidence` represents the confidence level
              of the interpolated value (range 0 to 1)
        
            Raises
            ------
            cpl.core.IllegalInputError
              If xradius, xprofile, yprofile and yradius are not as requested
            cpl.core.InvalidTypeError
              If the image type is not supported
        
            Notes
            -----
            If the X- and Y-radii are identical the area of inclusion is a circle,
            otherwise it is an ellipse, with the larger of the two radii as the
            semimajor axis and the other as the semiminor axis.
        
            A good profile length is 2001, using radius 2.0.
        
            The radii are only required to be positive. However, for small radii,
            especially radii less than 1/sqrt(2), (xpos, ypos) may be located such that
            no source pixels are included in the interpolation, causing the interpolated
            pixel value to be undefined.
        
            The X- and Y-profiles can be generated with
            cpl.core.Vector.fill_kernel_profile(profile, radius).
            For profiles generated with cpl_vector_fill_kernel_profile() it is
            important to use the same radius both there and in
            cpl.core.Image.get_interpolated().
        
            On error *pconfid* is negative (unless pconfid is NULL).
            Otherwise, if *pconfid* is zero, the interpolated pixel-value is undefined.
            Otherwise, if *pconfid* is less than 1, the area of inclusion is close to the
            image border or contains rejected pixels.
        
            The input image type can be cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
        """
    def get_mad(self, window: tuple | None = None) -> tuple[float, float]:
        """
                Computes median and median absolute deviation (MAD) on an image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                tuple(float,float)
                    The median of the non-bad pixels and the median absolute deviation of the good pixels in the format (median, MAD)
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the image type is not supported
                cpl.core.DataNotFoundError
                    If all pixels in the image are bad
                
                See Also
                --------
                cpl.core.Image.get_median_dev : for calculating median and mean absolute median deviation on an image or sub-window.
                cpl.core.Image.get_median : for calculating median on all pixels or sub-window.
        """
    def get_max(self, window: tuple | None = None) -> float:
        """
                Computes the maximum pixel value over an entire image or image sub window
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate (inclusive)
                    - `ury` Upper right Y coordinate (inclusive)
        
                Returns
                -------
                float
                    the maximum value
        
                Notes
                -----
                Does not work on complex images.
                
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                See Also
                --------
                cpl.core.Image.get_min : Get the minimum pixel value over the entire image or image sub window.
        """
    def get_maxpos(self, window: tuple | None = None) -> tuple[int, int]:
        """
                Computes maximum pixel value and position over an image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                tuple(int, int)
                    the x coordinate and y coordinate of the maximum position in the format (x,y)
                
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
                cpl.core.InvalidTypeError
                    If `self`'s pixel type is invalid
        
                See Also
                --------
                cpl.core.Image.get_minpos : get the position of the minimum value in the image
        """
    def get_mean(self, window: tuple | None = None) -> float:
        """
                Computes the mean pixel value over an entire image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the mean value
        
                Notes
                -----
                Does not work on complex images.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        """
    def get_median(self, window: tuple | None = None) -> float:
        """
                Computes the median pixel value over an entire image or sub-window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the median value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                Notes
                -----
                The median value is calculated using integer arithmetic if the image has integer data type,
                in which case the median value may differ from some other Python libraries such as numpy.
                For integer images the behaviour of get_median is equivalent to `np.floor(np.median(int_image))`. 
        """
    def get_median_dev(self, window: tuple | None = None) -> tuple[float, float]:
        """
                Computes median and mean absolute median deviation on an image or sub-window.
        
                For each non-bad pixel in the window the absolute deviation from the median is computed.
                The mean absolute median deviation is however still sensitive to outliers.
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                tuple(float,float)
                    The median of the non-bad pixels and the mean absolute median deviation
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
                cpl.core.DataNotFoundError
                    If all pixels in the specified window are bad
        
                See Also
                --------
                cpl.core.Image.get_mad : for calculating median and median absolute median deviation (MAD) on the image
                cpl.core.Image.get_median: for calculating median on all pixels
        """
    def get_min(self, window: tuple | None = None) -> float:
        """
                Computes the minimum pixel value over an entire image or image sub window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate (inclusive)
                    - `ury` Upper right Y coordinate (inclusive)
        
                Returns
                -------
                float
                    the minimum value
        
                Notes
                -----
                Does not work on complex images.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                See Also
                --------
                cpl.core.Image.get_max : Get the maximum pixel value over an image or image sub window.
        """
    def get_minpos(self, window: tuple | None = None) -> tuple[int, int]:
        """
                Computes minimum pixel value position over an image or sub-window.
        
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                tuple(int, int)
                    the x coordinate and y coordinate of the minimum position in the format (x,y)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
                cpl.core.InvalidTypeError
                    If `self`'s pixel type is invalid
        
                See Also
                --------
                cpl.core.Image.get_maxpos : get the position of the maximum value in the image
        """
    def get_pixel(self, y: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex) -> float | int | float | complex | complex | None:
        """
                Get a pixel at the specified coordinates.
        
                This is equivalent to getting the pixel via image index using im[y][x]
        
                Parameters
                ----------
                y : int
                    Row to extract value,  0 being the BOTTOMmost row of the image
                x : int
                    Column to extract value. 0 being the leftmost column of image
        
                Returns
                -------
                None, float, or complex
                    Value at the specified index.
                    - `None` if the value is invalid.
                    - float if the image if the image is of a numerical type (cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE).
                    - complex if the image is of a complex type (cpl.core.Type.FLOAT_COMPLEX or cpl.core.Type.DOUBLE_COMPLEX)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    Coordinates are invalid
        """
    def get_pixels(self, index: typing.SupportsInt | typing.SupportsIndex, length: typing.SupportsInt | typing.SupportsIndex) -> list[float | int | float | complex | complex | None]:
        """
                Get a list of pixel data from the image from a given index along the image.
        
                Indices are in reference to a 1D representation of the image starting from 0. 
                When converting from 2D coordinates this is equal to (row*width+column)
        
                Parameters
                ----------
                index : int
                    Zero-based index along the Image data to begin extracting pixel data. When converting from 2D coordinates this is equal to (row*width+column)
                length : int
                    Number of values to extract starting from `index`
        
                Returns
                -------
                list
                    `length` number of values in the image starting from pixel `index`
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    If the range specified using `index` and `length` is outside of the image.
        """
    def get_sqflux(self, window: tuple | None = None) -> float:
        """
                Computes the sum of squared values over an entire image or sub-window
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the square flux
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
        
                cpl.core.InvalidTypeError
                    If the image they are called on has complex data type.
        """
    def get_stdev(self, window: tuple | None = None) -> float:
        """
                Computes the pixel standard deviation over an image or sub window.
                
                Images can be cpl.core.Type.FLOAT, cpl.core.Type.INT or cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                window : tuple(int,int,int,int), optional
                    Window to operate on in the format (llx, lly, urx, ury) where:
                    - `llx` Lower left X coordinate (0 for leftmost)
                    - `lly` Lower left Y coordinate (0 for lowest)
                    - `urx` Upper right X coordinate
                    - `ury` Upper right Y coordinate
        
                Returns
                -------
                float
                    the standard deviation value
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the specified window is illegal
                Notes
                -----
                get_stdev calculates the "sample standard deviation" rather than the "ensemble standard
                deviation", i.e. the divisor in calculations is N - 1, where N is the number of pixels.
                This is equivalent to `np.std(image, ddof=1)` in numpy. 
        """
    def image_quality_est(self, window: tuple) -> ...:
        """
            Compute an image quality estimation for an object
        
            Parameters
            ----------
            window: tuple(int, int, int, int)
                The zone window in the format (x1, y1, x2, y2)
        
            Returns
            -------
            cpl.core.BiVector
              The IQE result, which contains in the first vector (x) the computed values, and in the second
              one (y), the associated errors.
        
              The computed values are:
              - x position of the object
              - y position of the object
              - FWHM along the major axis
              - FWHM along the minor axis
              - the angle of the major axis with the horizontal in degrees
              - the peak value of the object
              - the background computed
        
            Raises
            ------
            cpl.core.IllegalInputError
              if the input zone is not valid or if the computation fails on the zone
            cpl.core.InvalidTypeError
              if the input image has the wrong type
        
            Notes
            -----
            This function makes internal use of the iqe() MIDAS function (called
            here cpl_iqe()) written by P. Grosbol. Refer to the MIDAS documentation
            for more details. This function has proven to give good results over
            the years when called from RTD. The goal is to provide the exact same
            functionality in CPL as the one provided in RTD. The code is simply
            copied from the MIDAS package, it is not maintained by the CPL team.
        
            The bad pixels map of the image is not taken into account.
            The input image must be of type float.
        """
    def is_rejected(self, y: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
                Test if a pixel is good or bad
        
                Parameters
                ----------
                y : int
                    the y pixel position in the image (first pixel is 0)
                x : int
                    the x pixel position in the image (first pixel is 0)
        
                Returns
                -------
                bool
                    True if the pixel is bad, False if the pixel is good
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    if the specified position is outside of image `self`
        """
    def logarithm(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the elementwise logarithm of the image. Modified in place.
        
                Modifies the image by computing the base-scalar logarithm of each of its
                pixels.
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                Pixels for which the logarithm is not defined are
                rejected and set to zero.
        
                Parameters
                ----------
                base : float
                    Base of the logarithm.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the image type is not supported
                cpl.core.IllegalInputError
                    if base is non-positive
        """
    def move(self, nb_cut: typing.SupportsInt | typing.SupportsIndex, new_pos: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        """
                Permute tiles in an image
        
                nb_cut^2 defines in how many tiles the images will be permuted. Each tile will
                then be moved to another place defined in new_pos. nb_cut equal 1 will leave
                the image unchanged, 2 is used to permute the four image quadrants, etc..
                new_pos contains nb_cut^2 values between 1 and nb_cut^2, i.e. a permutation
                of the values from 1 to nb_cut^2.
                The zone positions are counted from the lower left part of the image.
                It is not allowed to move two tiles to the same position (the relation
                between th new tiles positions and the initial position is bijective !).
                The array with the permuted positions must contain nb_cut^2 values, the
                function is unable to verify this.
        
                The image x and y sizes have to be multiples of nb_cut.
        
                16   17   18           6    5    4
                13   14   15           3    2    1
        
                10   11   12   ---->  12   11   10
                7    8    9           9    8    7
        
                4    5    6          18   17   16
                1    2    3          15   14   13
        
                image 3x6            image.move(3, new_pos);
                with new_pos = [9,8,7,6,5,4,3,2,1];
        
                The bad pixels are moved in the same way.
        
                Parameters
                ----------
                nb_cut : int
                    The number of cuts in x and y
                new_pos : list of ints
                    Array with the nb_cut^2 permuted positions
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if `nb_cut` is not strictly positive or cannot divide one of the image sizes or
                    if the new_pos array specifies to move two tiles to the same position.
                cpl.core.InvalidTypeError
                    if the image type is not supported
        
                Notes
                -----
                The permutation array _must_ contain `nb_cut`-squared elements
        """
    def multiply(self, other: Image) -> None:
        """
                Multiply `self` by another image
        
                Parameters
                ----------
                other : cpl.core.Image
                    Image to multiply with `self`
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the input images have different sizes
                cpl.core.TypeMismatchError
                    if the `other` has complex type
        """
    def multiply_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise multiplication of an image with a scalar. Modified in place.
        
                Parameters
                ----------
                scalar : float
                    Number to multiply with
        """
    def negate(self) -> None:
        """
                Takes the bit-wise complement (NOT) of the image, pixel by pixel.
        
                The image must be integer type. The NOT is doen in place, overwriting the original image.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the image's type is not `cpl.core.Type.INT`
        """
    def normalise(self, mode: Image.Normalise) -> None:
        """
                Normalise pixels in an image. Modified in place.
        
                Normalises an image according to a given criterion.
        
                Possible normalisations are:
                - cpl.core.Image.Normalise.SCALE sets the pixel interval to [0,1].
                - cpl.core.Image.Normalise.MEAN sets the mean value to 1.
                - cpl.core.Image.Normalise.FLUX sets the flux to 1.
                - cpl.core.Image.Normalise.ABSFLUX sets the absolute flux to 1.
        
                Parameters
                ----------
                mode : cpl.core.Image.Normalise
                    Normalisation mode.
        """
    def or_scalar(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                The bit-wise OR of a scalar and an image with integer pixels. Modified in place.
        
                Parameters
                ----------
                value : int
                    scalar value to bit-wise OR with the pixels
        
                Notes
                -----
                cpl.core.Type.INT is required
        """
    def or_with(self, other: Image) -> None:
        """
                Takes the bit-wise OR of the image with another image, pixel by pixel.
        
                Both images must be integer type. The OR is done in place, overwriting the
                original image.
        
                Parameters
                ----------
                other : cpl.core.Image
                    Second operand
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the images have different sizes
                cpl.core.InvalidTypeError
                    If either image type is not cpl.core.Type.INT
        """
    def power(self, exponent: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the elementwise power of the image.
        
                Modifies the image by lifting each of its pixels to exponent.
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                Pixels for which the power to the given exponent is not defined are
                rejected and set to zero.
        
                Parameters
                ----------
                exponent : float
                    Scalar exponent.
        """
    def rebin(self, ystart: typing.SupportsInt | typing.SupportsIndex, xstart: typing.SupportsInt | typing.SupportsIndex, ystep: typing.SupportsInt | typing.SupportsIndex, xstep: typing.SupportsInt | typing.SupportsIndex) -> Image:
        """
                Rebin an image
        
                If both bin sizes in x and y are = 2, an image with (about) a quarter
                of the pixels of the input image will be created. Each new pixel
                will be the sum of the values of all contributing input pixels.
                If a bin is incomplete (i.e., the input image size is not a multiple
                of the bin sizes), it is not computed.
        
                xstep and ystep must not be greater than the sizes of the rebinned
                region.
        
                The input image type can be cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
                If the image has bad pixels, they will be propagated to the rebinned
                image "pessimistically", i.e., if at least one of the contributing
                input pixels is bad, then the corresponding output pixel will also
                be flagged "bad". If you need an image of "weights" for each rebinned
                pixel, just cast the input image bpm into a cpl.core.Type.INT image, and
                apply cpl.core.Image.rebin() to it too.
        
                Parameters
                ----------
                ystart : int
                    start y position of binning (starting from 0...)
                xstart : int
                    start x position of binning (starting from 0...)
                ystep : int
                    Bin size in y.
                xstep : int
                    Bin size in x.
        
                Returns
                -------
                cpl.core.Image
                    New rebinned image
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if xstep, ystep, xstart, ystart are not positive
                cpl.core.InvalidTypeError
                    if the image type is not supported
        """
    def reject(self, y: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Set a pixel as bad in an image
        
                Parameters
                ----------
                y : int
                    the y pixel position in the image (first pixel is 0)
                x : int
                    the x pixel position in the image (first pixel is 0)
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    if the specified position is outside of the image
        """
    def reject_from_mask(self, map: typing.Any) -> None:
        """
            Set the bad pixels in an image as defined in a mask
        
            If the input image has a bad pixel map prior to the call, it is overwritten.
        
            Parameters
            ----------
            map : cpl.core.Mask
                the mask defining the bad pixels
        
            Raises
            ------
            cpl.core.IncompatibleInputError
              if the image and the map have different sizes
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
                cpl.core.UnsupportedModeError
                  If something other than one of the supported special values is in
                  the values parameter.
                cpl.core.InvalidTypeError
                  If the image is a complex type.
        """
    def save(self, filename: str, pl: ..., mode: typing.SupportsInt | typing.SupportsIndex, dtype: Type = ...) -> None:
        """
                Save an image to a FITS file
        
                This function saves an image to a FITS file. If a property list is provided, it is written to the header where the image is written.
        
                Supported image types are cpl.core.Type.DOUBLE, cpl.core.Type.FLOAT, cpl.core.Type.INT.
        
                The type used in the file can be one of: cpl.core.Type.UCHAR (8 bit unsigned), cpl.core.Type.SHORT (16 bit signed), cpl.core.Type.USHORT
                (16 bit unsigned), cpl.core.Type.INT (32 bit signed), cpl.core.Type.FLOAT (32 bit floating point), or cpl.core.Type.DOUBLE (64 bit floating point).
                By default the saved type is cpl.core.Type.UNSPECIFIED. This value means that the type used for saving is the pixel type
                of the input image. Using the image pixel type as saving type ensures that the saving incurs no loss of information.
        
                Supported output modes are cpl.core.io.CREATE (create a new file) and cpl.core.io.EXTEND (append a new extension to an existing file)
        
                Note that in append mode the file must be writable (and do not take for granted that a file is writable just because it was created by the same
                application, as this depends from the system umask).
        
                The output mode cpl.core.io.EXTEND can be combined (via bit-wise OR) with an option for tile-compression. This compression is lossless.
                The options are: cpl.core.io.COMPRESS_GZIP, cpl.core.io.COMPRESS_RICE, cpl.core.io.COMPRESS_HCOMPRESS, cpl.core.io.COMPRESS_PLIO.
        
                Upon success the image will reside in a FITS data unit with NAXIS = 2. Is it possible to save a single image in a FITS data unit with NAXIS = 3
        
                Parameters
                ----------
                filename : str
                    Name of the file to write
                pl : cpl.core.PropertyList, optional
                    Property list for the output header. None by default.
                mode : unsigned int
                    Desired output options, determined by bit-wise OR of cpl.core.io enums
                dtype : cpl.core.Type, optional
                    The type used to represent the data in the file. By default it saves using the image's current dtype
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the type or the mode is not supported
                cpl.core.InvalidTypeError
                    if the passed pixel type is not supported
                cpl.core.FileNotCreatedError
                    If the output file cannot be created
                cpl.core.FileIOError
                    if the data cannot be written to the file
        
                See Also
                --------
                cpl.core.ImageList.save : for saving imagelists to a fits file
        """
    def set_pixel(self, y: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex, value: typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Set a pixel at the specified coordinates.
        
                This is equivalent to setting the pixel via image index using im[y][x] = value
        
                Parameters
                ----------
                y : int
                    Row to extract value, 0 being the BOTTOMmost row of the image
                x : int
                    Column to extract value. 0 being the leftmost column of image
                value : int, float, complex
                    Value to set. Must be compatible with the image type (int, float for numerical, complex for complex)
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    if the passed position is not within the image.
                cpl.core.InvalidTypeError
                    If the type `value` is not compatible with the image type of `self`
        """
    def set_pixels(self, pixels: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | None], index: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Set a list of pixel data from the image from a given index along the image.
        
                Indices are in reference to a 1D representation of the image starting from 0. When converting from 2D coordinates this is equal to (row*width+column)
        
                Some input `pixels` can be set to `None` to set as bad instead of setting a value. This will be reflected in the corresponding location
                in the bad pixel map.
        
                Parameters
                ----------
                pixels : int
                    `length` number of values to set in the image starting from pixel `index`
        
                index : int
                    Zero-based index along the Image data to begin setting pixel data. When converting from 2D coordinates this is equal to (row*width+column)
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    If the range specified using `index` and `length` is outside of the image.
        """
    def shift(self, dy: typing.SupportsInt | typing.SupportsIndex, dx: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Shift an image by integer offsets
        
                The new zones (in the result image) where no new value is computed are set
                to 0 and flagged as bad pixels.
                The shift values have to be valid:
                -nx < dx < nx and -ny < dy < ny
        
                Parameters
                ----------
                dy : int
                    The shift in Y
                dx : int
                    The shift in X
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the requested shift is bigger than the
        """
    def split_abs_arg(self) -> tuple[Image, Image]:
        """
                Split this complex image into its absolute and argument part(s)
        
                Any bad pixels are also processed.
        
                The split creates new image instances and will not modify the original image
        
                Returns
                -------
                tuple(cpl.core.Image, cpl.core.Image)
                    absolute and argument parts of the image in the format (absolute, argument)
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the image is not of a complex type
        
                Notes
                -----
                This corresponds to the `cpl_image_fill_abs_arg` function in the CPL C API
        """
    def split_real_imag(self) -> tuple[Image, Image]:
        """
                Split this complex image into its real and/or imaginary part(s)
        
                Any bad pixels are also processed.
        
                The split creates new image instances and will not modify the original image
        
                Returns
                -------
                tuple(cpl.core.Image, cpl.core.Image)
                    Real and Imaginary parts of the image in the format (real, imaginary).
                    Images will be of type `cpl.core.Type.DOUBLE` if `self` is of type `cpl.core.Type.DOUBLE_COMPLEX`.
                    Likewise images will be of type `cpl.core.Type.FLOAT` if `self` is of type `cpl.core.Type.FLOAT_COMPLEX`.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the image is not of a complex type
        
                Notes
                -----
                This corresponds to the `cpl_image_fill_re_im` function in the CPL C API
        """
    def subtract(self, other: Image) -> None:
        """
                Subtract image values from `self`
        
                Parameters
                ----------
                other : cpl.core.Image
                    Image to subtract from `self`
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the input images have different sizes
                cpl.core.TypeMismatchError
                    if the `other` has complex type
        """
    def subtract_scalar(self, scalar: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise subtraction of a scalar from an image. Modified in place.
        
                Parameters
                ----------
                scalar : float
                    Number to subtract
        """
    def threshold(self, assign_lo_cut: typing.SupportsFloat | typing.SupportsIndex, assign_hi_cut: typing.SupportsFloat | typing.SupportsIndex, lo_cut: typing.SupportsFloat | typing.SupportsIndex | None = None, hi_cut: typing.SupportsFloat | typing.SupportsIndex | None = None) -> None:
        """
            Threshold an image to a given interval. Thresholding is performed inplace.
        
            Pixels outside of the provided interval are assigned the given values.
        
            By default `lo_cut` and `hi_cut` are set to the minimum and maximum value of the image data type.
            Therefore `assign_lo_cut` will not be applied to any pixels if `lo_cut` is also not set,
            and `assign_hi_cut` will not be applied to any pixels if `hi_cut` is also not set.
        
            Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
            lo_cut must be smaller than or equal to hi_cut.
        
            Parameters
            ----------
            assign_lo_cut : float
                Value to assign to pixels below low bound.
            assign_hi_cut : float
                Value to assign to pixels above high bound.
            lo_cut : float, optional
                Lower bound.
            hi_cut : float, optional
                Higher bound.
        
            Raises
            ------
            cpl.core.InvalidTypeError
                if the image type is not supported
        """
    def to_type(self, dtype: Type) -> Image:
        """
                Convert an image to a given type. Modified in place.
        
                Casting to non-complex types is only supported for non-complex types.
        
                Parameters
                ----------
                dtype : cpl.core.Type
                    The destination type
        
                Returns
                -------
                cpl.core.Image
                    New image that is a copy of the original image converted to the given `dtype`
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If the passed type is invalid
                cpl.core.TypeMismatchError
                    If the image type is complex and requested casting type is non-complex.
        
                See Also
                --------
                cpl.core.Image.cast : Get the minimum pixel value over the entire image
        """
    def turn(self, rot: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Rotate an image by a multiple of 90 degrees clockwise.
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                The definition of the rotation relies on the PyCPL convention: The lower left corner of the image is at (0,0), x increasing from left to right, y increasing from bottom to top.
        
                For rotations of +90 or -90 degrees on rectangular non-1D-images, the pixel buffer is temporarily duplicated.
        
                rot may be any integer value, its modulo 4 determines the rotation:
        
                    -3 to turn 270 degrees counterclockwise.
                    -2 to turn 180 degrees counterclockwise.
                    -1 to turn 90 degrees counterclockwise.
                    0 to not turn
                    +1 to turn 90 degrees clockwise (same as -3)
                    +2 to turn 180 degrees clockwise (same as -2).
                    +3 to turn 270 degrees clockwise (same as -1).
        
                Parameters
                ----------
                rot : int
                    Number of clockwise rotations. -1 is a rotation of 90 deg counterclockwise.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the image type is not supported e
        """
    def vector_from_column(self, pos: typing.SupportsInt | typing.SupportsIndex) -> ...:
        """
                Extract a column from an image.
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                The bad pixels map is not taken into account in this function.
        
                Parameters
                ----------
                pos : int
                    Position of the column (0 for leftmost column)
        
                Returns
                -------
                cpl.core.Vector
                    Vector of values from column `pos` of the image
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `pos` is not valid
                cpl.core.IllegalTypeError
                    If the image is a type that is not supported
        """
    def vector_from_row(self, pos: typing.SupportsInt | typing.SupportsIndex) -> ...:
        """
                Extract a row from an image.
        
                Images can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE.
        
                The bad pixels map is not taken into account in this function.
        
                Parameters
                ----------
                pos : int
                    Position of the row (0 for bottom row)
        
                Returns
                -------
                cpl.core.Vector
                    Vector of values from row `pos` of the image
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `pos` is not valid
                cpl.core.IllegalTypeError
                    If the image is a type that is not supported
        """
    def warp(self, deltay: Image, deltax: Image, yprofile: ..., yradius: typing.SupportsFloat | typing.SupportsIndex, xprofile: ..., xradius: typing.SupportsFloat | typing.SupportsIndex) -> Image:
        """
                Generate a warped version of this image
        
                Parameters
                ----------
                deltax : int
                    The x shift of each pixel, must be same size as `deltay` and type `cplcore.Type.DOUBLE`
                deltay : int
                    The y shift of each pixel, must be same size as `deltax` and type `cplcore.Type.DOUBLE`
                xprofile : cpl.core.Vector
                    Interpolation weight as a function of the distance in Y
                xradius : float
                    Positive inclusion radius in the X-dimension
                yprofile : cpl.core.Vector
                    Interpolation weight as a function of the distance in Y
                yradius : float
                    Positive inclusion radius in the Y-dimension
                xprofile : cpl.core.Vector
                    Interpolation weight as a function of the in X
                xradius: float
                    Positive inclusion radius in the X-dimension
        
                Returns
                -------
                cpl.core.Image
                    new warped image, same size as `deltax` and `deltay` and same type as self
        
                Raises
                ----------
                cpl.core.IllegalInputError
                   if the input images sizes are incompatible or if the delta images are not of type cplcore.Type.DOUBLE
                cpl.core.InvalidTypeError
                   if the image type is not supported
        
                See Also
                --------
                cpl.core.Image.create_jacobian : Compute area change ratio for a transformation map.
        
                Notes
                -----
                The pixel value at the (integer) position (u, v) in the destination image is interpolated
                from the (typically non-integer) pixel position (x, y) in the source image, where:
        
                x = u - deltax(u, v),
                y = v - deltay(u, v).
        
                The identity transform is thus given by `deltax` and `deltay` filled with zeros.
        
                `deltax` and `deltay` may be a different size than self, but must be the same size
                as each other. 
        
                self may be of the type cplcore.Type.INT, cplcore.Type.FLOAT or cplcore.Type.DOUBLE
        
                If case a correction for flux conservation is required please create a correction map using
                the function `cpl.core.Image.create_jacobian()`.
        """
    def warp_polynomial(self, poly_y: ..., poly_x: ..., yprofile: ..., yradius: typing.SupportsFloat | typing.SupportsIndex, xprofile: ..., xradius: typing.SupportsFloat | typing.SupportsIndex, out_dim: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex] | None = None, out_type: cpl.core.Type | None = None) -> Image:
        """
            Warp an image according to a 2D polynomial transformation.
        
            Parameters
            ----------
            poly_y : cpl.dfs.Polynomial
                Polynomial defining source y-pos corresponding to destination (u,v).
            poly_x :  cpl.dfs.Polynomial
                Polynomial defining source x-pos corresponding to destination (u,v).
            yprofile :  cpl.dfs.Vector
                Interpolation weight vector as a function of the distance in Y
            yradius : float
                Positive inclusion radius in the Y-dimension
            xprofile : cpl.dfs.Vector
                Interpolation weight as a function of the distance in X
            xradius : float
                Positive inclusion radius in the X-dimension
            out_dim : (size, size)
                output dimensions. If not given then will default to the same dimensions of
                self
            out_type : cpl.core.Type
                Output type. If not given then will default to the same type of self. Will
                cause errors if output type is not compatible with input
        
            Returns
            -------
            cpl.core.Image
                New warped image
        
            Notes
            -----
            'out' and 'in'  may have different dimensions and types.
        
            The pair of 2D polynomials are used internally like this:
        
            .. code-block:: python
        
              x = poly_x.eval(cpl.core.Vector([u,v]))
              y = poly_y.eval(cpl.core.Vector([u,v]))
        
            where (u,v) are (integer) pixel positions in the destination image and (x,y)
            are the corresponding pixel positions (typically non-integer) in the source
            image.
        
            The identity transform (poly_x(u,v) = u, poly_y(u,v) = v) would thus
            overwrite the 'out' image with the 'in' image, starting from the lower left
            if the two images are of different sizes.
        
            Beware that extreme transformations may lead to blank images.
        
            The input image type can be cpl.core.Type.INT, cpl.core.Type.FLOAT and cpl.core.Type.DOUBLE.
        
            In case a correction for flux conservation were required, please create
            a correction map using the function `cpl.core.Image.create_jacobian_polynomial()`.
        
            Raises
            ------
            cpl.core.IllegalInputError
              if the polynomial dimensions are not 2
            cpl.core.InvalidTypeError
              if the output image type is incompatible with the input image
        """
    def xor_scalar(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                The bit-wise XOR of a scalar and an image with integer pixels. Modified in place.
        
                Parameters
                ----------
                value : int
                    scalar value to bit-wise XOR with the pixels
        
                Notes
                -----
                cpl.core.Type.INT is required
        """
    def xor_with(self, other: Image) -> None:
        """
                Takes the bit-wise XOR of the image with another image, pixel by pixel.
        
                Both images must be integer type. The XOR is done in place, overwriting the
                original image.
        
                Parameters
                ----------
                other : cpl.core.Image
                    Second operand
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the images have different sizes
                cpl.core.InvalidTypeError
                    If either image type is not cpl.core.Type.INT
        """
    @property
    def bpm(self) -> typing.Any:
        """
        cpl.core.Mask : Bad Pixel Mask of this image to mark locations of bad pixels often used during filtering
        """
    @bpm.setter
    def bpm(self, arg1: typing.Any) -> cpl.core._Mask1D | None:
        ...
    @property
    def height(self) -> int:
        """
        int : height of the image
        """
    @property
    def is_complex(self) -> bool:
        """
        bool : True if the image is of type cpl.core.Type.FLOAT_COMPLEX or cpl.core.Type.DOUBLE_COMPLEX
        """
    @property
    def shape(self) -> tuple:
        """
        tuple(int, int) : tuple detailing the shape of the image in the format (height, width)
        """
    @property
    def size(self) -> int:
        """
        int : Total number of pixels in the image (width*height)
        """
    @property
    def type(self) -> Type:
        """
        cpl.core.Type : Pixel type of the image
        """
    @property
    def width(self) -> int:
        """
        int : width of the image
        """
class ImageList:
    """
    
              This module provides functions to create and use a cpl_imagelist.
    
              A CPL ImageList is an ordered list of CPL Images. All images in a list must have the same pixel-type and the same dimensions.
    
              It is allowed to insert the same image into different positions in the list. Different images in the list are allowed to share the same bad pixel map.
    
              Parameters
              ----------
              from : iterable of cpl.core.Image, optional
                  Images to store in `self` on init. If not given the ImageList is initialised withou any images.
    
              Raises
              ------
              cpl.core.TypeMismatchError
                  images in `from` are of varying types
              cpl.core.IncompatibleInputError
                  images in `from` are of varying sizes
        
    """
    class Collapse:
        """
        Members:
        
          MEAN
        
          MEDIAN
        
          MEDIAN_MEAN
        """
        MEAN: typing.ClassVar[ImageList.Collapse]  # value = <Collapse.MEAN: 0>
        MEDIAN: typing.ClassVar[ImageList.Collapse]  # value = <Collapse.MEDIAN: 1>
        MEDIAN_MEAN: typing.ClassVar[ImageList.Collapse]  # value = <Collapse.MEDIAN: 1>
        __members__: typing.ClassVar[dict[str, ImageList.Collapse]]  # value = {'MEAN': <Collapse.MEAN: 0>, 'MEDIAN': <Collapse.MEDIAN: 1>, 'MEDIAN_MEAN': <Collapse.MEDIAN: 1>}
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
    class SwapAxis:
        """
        Members:
        
          XZ
        
          YZ
        """
        XZ: typing.ClassVar[ImageList.SwapAxis]  # value = <SwapAxis.XZ: 0>
        YZ: typing.ClassVar[ImageList.SwapAxis]  # value = <SwapAxis.YZ: 1>
        __members__: typing.ClassVar[dict[str, ImageList.SwapAxis]]  # value = {'XZ': <SwapAxis.XZ: 0>, 'YZ': <SwapAxis.YZ: 1>}
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
    def load(filename: str, dtype: Type = ..., extension: typing.SupportsInt | typing.SupportsIndex = 0, area: tuple = None) -> ImageList:
        """
                            Load an image list from a file.
        
                            Load data from the extension `extension` of the FITS file
                            `filename` into a list of images. The FITS extension may be
                            an image (``NAXIS`` = 2) or a data cube (``NAXIS`` =  3). Each
                            image plane in the input data is loaded as a separate image.
                            By default the data is read from the primary HDU of the FITS
                            file.
        
                            By default the full area (extent in x and y) of the data is
                            loaded. This may be restricted to a particular region of
                            the data by providing an appropriate argument `area`.
        
                            The argument `dtype` specifies the pixel data type of the result
                            image list. When the data is loaded the pixel data type in the
                            input FITS file is converted into `dtype`. By default the data in
                            the input extension is converted to cpl.core.Type.DOUBLE. To load
                            the data without converting the pixel data type use
                            cpl.core.Type.UNSPECIFIED.
        
                            Valid pixel data types which may be used for `dtype` are:
        
                            - cpl.core.Type.INT  (32-bit integer)
                            - cpl.core.Type.FLOAT
                            - cpl.core.Type.DOUBLE
        
                            Parameters
                            ----------
        
                            filename : str
                              Name of the input file
                            dtype : cpl.core.Type, optional
                              Data type of the pixels in the returend list of images. Is cpl.core.Type.DOUBLE by default.
                            extension : int, default=0
                              Index of the FITS extension to load (the primary data
                              unit has index 0)
                            Area : Tuple, default=None
                              Region of interest to load given as a tuple specifying
                              the lower left x, the lower left y, the upper right x (inclusive)
                              and the upper right y coordinate (inclusive) in this order.
                              Numbering of the pixel x and y positions starts at 1
                              (FITS convention)
        
                            Returns
                            -------
                            cpl.core.ImageList
                              New image list instance of loaded data
        
                            Raises
                            ------
                            cpl.core.FileIOError
                              If the file cannot be opened, or does not exist.
                            cpl.core.BadFileFormatError
                              If the data cannot be loaded from the file.
                            cpl.core.InvalidTypeError
                              If the requested pixel data type is not supported.
                            cpl.core.IllegalInputError
                              If the requested extension number is invalid (negative),
                              the plane number is out of range, or if the given image region
                              is invalid.
                            cpl.core.DataNotFoundError
                              If the specified extension has no image data.
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
    def __init__(self, from_: collections.abc.Sequence[Image]) -> None:
        ...
    @typing.overload
    def __init__(self, from_: collections.abc.Iterable) -> None:
        ...
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, index: typing.SupportsInt | typing.SupportsIndex, image: Image) -> None:
        """
        Set image at index position
        """
    def __str__(self) -> str:
        ...
    def add(self, other: ImageList) -> None:
        """
                Add this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is added to the image number n in this list.
        
                Parameters
                ----------
                other : cpl.core.ImageList
                    ImageList to add
        """
    def add_image(self, img: Image) -> None:
        """
                Add an image list by an image.
        
                Parameters
                ----------
                img : cpl.core.Image
                    image to add
        """
    def add_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise addition of a scalar to each image in the imlist. Modified in place
        
                Parameters
                ----------
                value : float
                    Number to add
        
                Returns
                -------
                None
        """
    def append(self, to_append: Image) -> None:
        """
                Append an image to the end of `self`
        
                It is allowed to insert the same image into two different positions in a list.
        
                To insert an image a specific position then set via index (e.g. self[i] = new_image)
        
                It is not allowed to insert images of different sizes or types into a list.
        
                Parameters
                ----------
                to_append : cpl.core.Image
                    The image to append
        
                Raises
                ------
                cpl.core.TypeMismatchError
                    if `to_append` and `self` are of different types
                cpl.core.IncompatibleInputError
                    if `to_append` and `self` have different sizes
        """
    def astype(self, dtype: Type) -> ImageList:
        """
                Cast an imagelist to a different CPL type
        
                Parameters
                ----------
                dtype : cpl.core.Type
                    Type to cast the imagelist to
        
                Returns
                -------
                New ImageList, containing images cast to the specified type
        """
    def collapse_create(self) -> Image:
        """
                Average an imagelist to a single image.
        
                The bad pixel maps of the images in the input list are taken into account, the result image pixels are flagged as rejected for
                those where there were no good pixel at the same position in the input image list.
        
                For integer pixel types, the averaging is performed using integer division.
        
                Returns
                -------
                cpl.core.Image
                  The average Image
        """
    def collapse_median_create(self) -> Image:
        """
                Create a median image from the Imagelist
        
                The image list can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT and
                cpl.core.Type.DOUBLE.
        
                On success each pixel in the created image is the median of the values on
                the same pixel position in the images in the list. If for a given pixel all
                values in the input image list are rejected the resulting pixel is set to
                zero and flagged as rejected.
        
                The median is defined here as the middle value of an odd number of sorted
                samples and for an even number of samples as the mean of the two central
                values. Note that with an even number of samples the median may not be
                among the input samples.
        
                Also note that in the case of an even number of integer images the mean
                value will be computed using integer arithmetic. Cast your integer data
                to a floating point pixel type if that is not the desired behavior.
        
                Returns
                -------
                cpl.core.Image
                    The median image of the input pixel type
        
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the image list is not valid
        """
    def collapse_minmax_create(self, nlow: typing.SupportsInt | typing.SupportsIndex, nhigh: typing.SupportsInt | typing.SupportsIndex) -> Image:
        """
                Average with rejection an imagelist to a single image
        
                The input images are averaged, for each pixel position the nlow lowest pixels
                and the nhigh highest pixels are discarded for the average computation.
        
                The input image list can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT and
                cpl.core.Type.DOUBLE. The created image will be of the same type.
        
                On success each pixel in the created image is the mean of the non-rejected
                values on the pixel position in the input image list.
        
                For a given pixel position any bad pixels (i.e. values) are handled as
                follows:
                Given n bad values on a given pixel position, n/2 of those values are assumed
                to be low outliers and n/2 of those values are assumed to be high outliers.
                Any low or high rejection will first reject up to n/2 bad values and if more
                values need to be rejected that rejection will take place on the good values.
                This rationale behind this is to allow the rejection of outliers to include
                bad pixels without introducing a bias.
                If for a given pixel all values in the input image list are rejected, the
                resulting pixel is set to zero and flagged as rejected.
        
                Parameters
                ----------
                nlow : int
                    Number of low rejected values
                nhigh : int
                    Number of high rejected values
        
                Returns
                -------
                cpl.core.Image
                    The average image
        
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the input image list is not valid or if the sum of the rejections is
                    not lower than the number of images or if nlow or nhigh is negative
                cpl.core.InvalidTypeError
                    if the passed image list type is not supported
        """
    def collapse_sigclip_create(self, kappalow: typing.SupportsFloat | typing.SupportsIndex, kappahigh: typing.SupportsFloat | typing.SupportsIndex, keepfrac: typing.SupportsFloat | typing.SupportsIndex, mode: ImageList.Collapse) -> tuple[Image, Image]:
        """
                Collapse an imagelist with kappa-sigma-clipping rejection
        
                The collapsing is an iterative process which will stop when it converges
                (i.e. an iteration did not reject any values for a given pixel) or
                when the next iteration would reduce the fraction of values to keep
                to less than or equal to keepfrac.
        
                A call with keepfrac == 1.0 will thus perform no clipping.
        
                Supported modes:
                cpl.core.ImageList.Collapse.MEAN:
                The center value of the acceptance range will be the mean.
                cpl.core.ImageList.Collapse.MEDIAN:
                The center value of the acceptance range will be the median.
                cpl.core.ImageList.Collapse.MEDIAN_MEAN:
                The center value of the acceptance range will be the median in
                the first iteration and in subsequent iterations it will be the
                mean.
        
                For each pixel position the pixels whose value is higher than
                center + kappahigh * stdev or lower than center - kappalow * stdev
                are discarded for the subsequent center and stdev computation, where center
                is defined according to the clipping mode, and stdev is the standard
                deviation of the values at that pixel position. Since the acceptance
                interval must be non-empty, the sum of kappalow and kappahigh must be
                positive. A typical call has both kappalow and kappahigh positive.
        
                The minimum number of values that the clipping can select is 2. This is
                because the clipping criterion is based on the sample standard deviation,
                which needs at least two values to be defined. This means that all calls
                with (positive) values of keepfrac less than 2/n will behave the same. To
                ensure that the values in (at least) i planes out of n are kept, keepfrac
                can be set to (i - 0.5) / n, e.g. to keep at least 50 out of 100 values,
                keepfrac can be set to 0.495.
        
                The output pixel is set to the mean of the non-clipped values, regardless
                of which clipping mode is used. Regardless of the input pixel type, the
                mean is computed in double precision. The result is then cast to the
                output-pixel type, which is identical to the input pixel type.
        
                Bad pixels are ignored from the start. This means that with a sufficient
                number of bad pixels, the fraction of good values will be less than keepfrac.
                In this case no iteration is performed at all. If there is at least one
                good value available, then the mean will be based on the good value(s). If
                for a given pixel position there are no good values, then that pixel is
                set to zero, rejected as bad and if available the value in the
                contribution map is set to zero.
        
                The input imagelist can be of type cpl.core.Type.INT, cpl.core.Type.FLOAT and
                cpl.core.Type.DOUBLE.
        
                Parameters
                ----------
                kappalow : float
                    kappa-factor for lower clipping threshold
                kappahigh : float
                    kappa-factor for upper clipping threshold
                keepfrac : float
                    The fraction of values to keep (0.0 < keepfrac <= 1.0)
                mode : cpl.core.ImageList.Collapse
                    Clipping mode, cpl.core.ImageList.Collapse.MEAN or cpl.core.ImageList.Collapse.MEDIAN
        
                Returns
                -------
                tuple(cpl.core.Image, cpl.core.Image)
                    The collapsed image and the contribution map as an integer image, i.e. the number
                    of kept (non-clipped) values after the iterative process on every pixel.
                    In the format (collapsed, contribution)
        
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if there are less than 2 images in the list
                cpl.core.IllegalInputError
                    if the sum of `kappalow` and `kappahigh` is non-positive,
                cpl.core.AccessOutOfRangeError
                    if keepfrac is outside the required interval which is 0.0 < keepfrac <= 1.0
                cpl.core.InvalidTypeError
                    if the type of the input imagelist is unsupported
                cpl.core.UnsupportedModeError
                    if the passed mode is none of the above listed
        """
    def divide(self, other: ImageList) -> None:
        """
                Divide this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is divides the image number n in this list.
        
                Parameters
                ----------
                other : cpl.core.ImageList
                    ImageList to divide with
        """
    def divide_image(self, img: Image) -> None:
        """
                Divide an image list by an image.
        
                Parameters
                ----------
                img : cpl.core.Image
                    image to divide
        """
    def divide_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise division of each image in the imlist with a scalar.
        
                Parameters
                ----------
                value : float
                    Non-zero number to divide with
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
                    -------
                    str 
                        Multiline string containing the dump of the image contents in the ImageList.
        """
    def empty(self) -> None:
        """
                Empty an imagelist and deallocate all its images
        
                After the call the image list can be populated again.
        """
    def exponential(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the elementwise exponential of each image in `self`. Modified in place.
        
                Parameters
                ----------
                base : float
                    Base of the exponential.
        """
    def insert(self, position: typing.SupportsInt | typing.SupportsIndex, item: Image) -> None:
        """
                Insert an image into the index `position`. This will increase the imagelist size by 1
        
                Parameters
                ----------
                position : int
                    index to insert Image
                item : cpl.core.Image
                    Image to insert
        """
    def is_uniform(self) -> bool:
        """
                Determine if an imagelist contains images of equal size and type
        
                The function raises an error if the imagelist is empty (see Raises)
        
                Returns
                -------
                bool
                  True if uniform, otherwise false
        
                Raises
                ------
                cpl.core.IllegalInputError
                  If the imagelist is empty
        """
    def multiply(self, other: ImageList) -> None:
        """
                Elementwise multiply this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is multiplyed with the image number n in this list.
        
                Parameters
                ----------
                other : cpl.core.ImageList
                    ImageList to multiply
        """
    def multiply_image(self, img: Image) -> None:
        """
                Multiply an image list by an image.
        
                Parameters
                ----------
                img : cpl.core.Image
                    image to multiply
        """
    def multiply_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise multiplication of a scalar to each image in the imlist.
        
                Parameters
                ----------
                value : float
                    Number to multiply with
        
                Returns
                -------
                None
        """
    def normalise(self, mode: Image.Normalise) -> None:
        """
                Normalize each image in the list. Modified in place.
        
                The list may be partly modified if an error occurs.
        
                Possible normalisations are:
                - cpl.core.Image.Normalise.SCALE sets the pixel interval to [0,1].
                - cpl.core.Image.Normalise.MEAN sets the mean value to 1.
                - cpl.core.Image.Normalise.FLUX sets the flux to 1.
                - cpl.core.Image.Normalise.ABSFLUX sets the absolute flux to 1.
        
                Parameters
                ----------
                mode : cpl.core.Image.Normalise
                    Normalization mode.
        """
    def pop(self, position: typing.SupportsInt | typing.SupportsIndex | None = None) -> Image:
        """
                Remove and return the image at `position`
        
                Parameters
                ----------
                position : int, optional
                    Index to pop image from the image list. Defaults to the last image.
        
                Raises
                ------
                IndexError
                    If `position` is out of range
        """
    def power(self, exponent: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the elementwise power of each image in the imlist.
        
                Parameters
                ----------
                exponent : float
                    Scalar exponent
        """
    def save(self, filename: str, pl: ..., mode: typing.SupportsInt | typing.SupportsIndex, dtype: Type = ...) -> None:
        """
                Save an imagelist to a FITS file
        
                This function saves an image list to a FITS file. If a property list is provided, it is written to the named file before the pixels are written.
        
                Image lists are saved as a 3 dimensional data cube.
        
                Supported image types are cpl.core.Type.DOUBLE, cpl.core.Type.FLOAT, cpl.core.Type.INT.
        
                The type used in the file can be one of: cpl.core.Type.UCHAR (8 bit unsigned), cpl.core.Type.SHORT (16 bit signed), cpl.core.Type.USHORT
                (16 bit unsigned), cpl.core.Type.INT (32 bit signed), cpl.core.Type.FLOAT (32 bit floating point), or cpl.core.Type.DOUBLE (64 bit floating point).
                By default the saved type is cpl.core.Type.UNSPECIFIED. This value means that the type used for saving is the pixel type
                of the input image. Using the image pixel type as saving type ensures that the saving incurs no loss of information.
        
                Supported output modes are cpl.core.io.CREATE (create a new file) and cpl.core.io.EXTEND (append a new extension to an existing file)
        
                Note that in append mode the file must be writable (and do not take for granted that a file is writable just because it was created by the same
                application, as this depends from the system umask).
        
                The output mode cpl.core.io.EXTEND can be combined (via bit-wise OR) with an option for tile-compression. This compression is lossless.
                The options are: cpl.core.io.COMPRESS_GZIP, cpl.core.io.COMPRESS_RICE, cpl.core.io.COMPRESS_HCOMPRESS, cpl.core.io.COMPRESS_PLIO. With compression
                the type must be cpl.core.Type.UNSPECIFIED or cpl.core.Type.INT.
        
                In extend and append mode, make sure that the file has write permissions. You may have problems if you create a file in your application and
                append something to it with the umask set to 222. In this case, the file created by your application would not be writable, and the append would fail.
        
                Parameters
                ----------
                filename : str
                    Name of the file to write
                pl : cpl.core.PropertyList, optional
                    Property list for the output header. None by default.
                mode : unsigned int
                    Desired output options, determined by bit-wise OR of cpl.core.io enums
                dtype : cpl.core.Type, optional
                    The type used to represent the data in the file. By default it saves using the image's current dtype
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the type or the mode is not supported
                cpl.core.InvalidTypeError
                    if the passed pixel type is not supported
                cpl.core.FileNotCreatedError
                    If the output file cannot be created
                cpl.core.FileIOError
                    if the data cannot be written to the file
        
                See Also
                --------
                cpl.core.Image.save : for saving individual images to a fits file
        """
    def subtract(self, other: ImageList) -> None:
        """
                Elementwise subtract this ImageList with another. Modified in place.
        
                The two input lists must have the same size, the image number n in the list other is subtracted from the image number n in this list.
        
                Parameters
                ----------
                other : cpl.core.ImageList
                    ImageList to subtract with
        """
    def subtract_image(self, img: Image) -> None:
        """
                Subtract an image list by an image.
        
                Parameters
                ----------
                img : cpl.core.Image
                    image to subtract
        """
    def subtract_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise subtraction of a scalar to each image in the imlist. Modified in place.
        
                Parameters
                ----------
                value : float
                    Number to subtract
        """
    def swap_axis_create(self, mode: ImageList.SwapAxis) -> ImageList:
        """
                This function is intended for users that want to use the ImageList object as a cube.
        
                Swapping the axis would give them access to the usual functions in the 3 dimensions. This has the cost that it duplicates the memory
                consumption, which can be a problem for big amounts of data.
        
                Image list can be cpl.core.Type.INT, cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE
        
                Parameters
                ----------
                mode : cpl.core.ImageList.SwapAxis
                    The swapping mode. The mode can be either cpl.core.ImageList.SwapAxis.XZ or cpl.core.ImageList.SwapAxis.YZ
        
                Returns
                -------
                New image list of the given axis
        """
    def threshold(self, lo_cut: typing.SupportsFloat | typing.SupportsIndex, hi_cut: typing.SupportsFloat | typing.SupportsIndex, assign_lo_cut: typing.SupportsFloat | typing.SupportsIndex, assign_hi_cut: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Threshold all pixel values to an interval.
        
                Threshold the images of the list using cpl_image_threshold()
                The input image list is modified.
        
                Pixels outside of the provided interval are assigned the given values.
        
                Parameters
                ----------
                lo_cut : float
                    Lower bound.
                hi_cut : float
                    Higher bound.
                assign_lo_cut : float
                    Value to assign to pixels below low bound.
                assign_hi_cut : float
                    Value to assign to pixels above high bound.
        """
class ImageRow:
    """
    
            Returned from a Image's __getitem__ method or iterator. Used to access specific rows of the image.
    
            Not instantiatable on its own.
        
    """
    def __array__(self, **kwargs) -> numpy.ndarray:
        ...
    def __contains__(self, arg0: typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | None) -> bool:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> float | int | float | complex | complex | None:
        ...
    def __len__(self) -> int:
        ...
    def __next__(self) -> float | int | float | complex | complex | None:
        ...
    def __reversed__(self) -> None:
        """
        Unsupported
        """
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | None) -> None:
        ...
    def __str__(self) -> typing.Any:
        ...
class IncompatibleInputError(Error, ValueError):
    """
    Data that had to be processed together did not match.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 13
class InvalidTypeError(Error, RuntimeError):
    """
    Data type was unsupported or invalid.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 20
class Kernel:
    """
    Members:
    
      DEFAULT : default kernel, currently cpl.core.Kernel.TANH
    
      TANH : Hyperbolic tangent
    
      SINC : Sinus cardinal
    
      SINC2 : Square sinus cardinal
    
      LANCZOS : Lanczos2 kernel
    
      HAMMING : Hamming kernel
    
      HANN : Hann kernel
    
      NEAREST : Nearest neighbor kernel (1 when dist < 0.5, else 0)
    """
    DEFAULT: typing.ClassVar[Kernel]  # value = <Kernel.DEFAULT: 0>
    HAMMING: typing.ClassVar[Kernel]  # value = <Kernel.HAMMING: 4>
    HANN: typing.ClassVar[Kernel]  # value = <Kernel.HANN: 5>
    LANCZOS: typing.ClassVar[Kernel]  # value = <Kernel.LANCZOS: 3>
    NEAREST: typing.ClassVar[Kernel]  # value = <Kernel.NEAREST: 6>
    SINC: typing.ClassVar[Kernel]  # value = <Kernel.SINC: 1>
    SINC2: typing.ClassVar[Kernel]  # value = <Kernel.SINC2: 2>
    TANH: typing.ClassVar[Kernel]  # value = <Kernel.DEFAULT: 0>
    __members__: typing.ClassVar[dict[str, Kernel]]  # value = {'DEFAULT': <Kernel.DEFAULT: 0>, 'TANH': <Kernel.DEFAULT: 0>, 'SINC': <Kernel.SINC: 1>, 'SINC2': <Kernel.SINC2: 2>, 'LANCZOS': <Kernel.LANCZOS: 3>, 'HAMMING': <Kernel.HAMMING: 4>, 'HANN': <Kernel.HANN: 5>, 'NEAREST': <Kernel.NEAREST: 6>}
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
class Mask(numpy.ndarray):
    """
    
    These masks are useful for object detection routines or bad pixel map handling. 
    
    Morphological routines (erosion, dilation, closing and opening) and logical operations are provided. 
    
    CPL masks are like a 2d binary array, with each pixel representing True or False, and can be set as such:
    
    .. code-block:: python
    
        m = cpl.core.Mask(3,3)
        m[0][0] = True
    
    PyCPL uses 0 indexing, in the sense that the lower left element in a CPL mask has index (0, 0).
    """
    __hash__: typing.ClassVar[None] = None
    @classmethod
    def __new__(cls, *args):
        """
        
        Generate a new Mask with the following formats:
        Mask(Collection) : Pass a non-empty list of homogeneous lists
        Mask(width, height, bytes) : Build a 2d mask from a bytestring with given dimensions
        
        Raises
        ------
        cpl.core.IllegalInputError
            if given width or height is negative
        """
    @classmethod
    def load(cls, fitsfile, extension = 0, plane = 0, window = None):
        """
        
        Loads an bitmask from an INTEGER FITS file
        
        Parameters
        ----------
        fitsfile : str
            filename of fits file
        extension : int
            Specifies the extension from which the image should be loaded
            (Default) 0 is for the main data section (Files without extension)
        plane : int
            Specifies the plane to request from the data section. Default 0.
        window : tuple(int, int, int, int)
            The rectangle in the format (x1,y1, x2, y2) specifying the subset of the image to load. 
            If None, load the entire window
        
        Raises
        ------
        cpl.core.FileIOError
            if the file cannot be opened or does not exist
        cpl.core.BadFileFormatError
            if the data cannot be loaded from the file
        cpl.core.IllegalInputError
            if the pased extension number is negative
        cpl.core.DataNotFoundError
            if the specified extension has no mask data
        """
    @classmethod
    def threshold_image(cls, image, lo_cut, hi_cut, inval):
        """
        
        Create a new Mask by applying the given thresholds to a `cpl.core.Image`.
        
        Parameters
        ----------
        image : cpl.core.Image
            Image to threshold
        lo_cut : float
            Lower bound for threshold
        hi_cut : float
            Uppper bound for threshold
        inval : bool
            This value (0 or 1, False or True) is assigned where
            the pixel value is not marked as rejected and is strictly
            inside the provided interval. The other positions are assigned
            the other value.
        
        Raises
        ------
        cpl.core.UnsupportedModeError
            if the image data type is unsupported
        cpl.core.IllegalInputError
            if inval is not binary
        
        Notes
        -----
        The input image type can be cpl.core.Type.DOUBLE, cpl.core.Type.FLOAT or cpl.core.Type.INT.
        
        If `lo_cut` is greater than or equal to `hi_cut`, then the mask is filled with
        outval (opposite of `inval`).
        """
    def _2d_to_bytes(self, lists):
        ...
    def __and__(self, other):
        ...
    def __deepcopy__(self):
        ...
    def __eq__(self, other):
        """
        
        Checks equality by xoring the two masks
        """
    def __invert__(self):
        ...
    def __or__(self, other):
        ...
    def __repr__(self):
        ...
    def __str__(self):
        ...
    def __xor__(self, other):
        ...
    def collapse_cols(self):
        """
        
        Create a 1-column mask, all elements are the logical AND of each cell in its 
        corresponding column. Height is kept the same.
        """
    def collapse_rows(self):
        """
        
        Create a 1-row mask, all elements are the logical AND of each cell in its 
        corresponding column. Width is kept the same
        """
    def copy(self):
        """
        
        Duplicate the mask
        """
    def count(self, window = None):
        """
        
        Determines number of occurrences of '1' bit in the given window of this bitmask
        
        Parameters
        ----------
        window : int
            Rectangle to count bits in the format (x1,y1,x2,y2)
        
        """
    def dump(self, filename = '', mode = 'w', window = None, show = True):
        """
        
        Dump the mask contents to a file, stdout or a string.
        
        This function is intended just for debugging. It prints the contents of a mask 
        to the file path specified by `filename`. 
        If a `filename` is not specified, output goes to stdout (unless `show` is False). 
        In both cases the contents are also returned as a string.
        
        Parameters
        ----------
        filename : str, optional
            File to dump mask contents to
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
          Defaults to entire image.
        show : bool, optional
            Send mask contents to stdout. Defaults to True.
        
        Returns
        -------
        str 
            Multiline string containing the dump of the mask contents.
        """
    def extract(self, window):
        """
        
        Copies out a window of this mask to a new mask.
        
        Parameters
        ----------
        window : tuple(int, int, int, int)
            rectangle to extract from this mask in the format (x1,y1, x2, y2)
        
        Raises
        ------
        cpl.core.IllegalInputError
            if the zone falls outside the mask
        """
    def filter(self, kernel, filter, border):
        """
        
        Filter the mask using a binary kernel. 
        
        The kernel must have an odd number of rows and an odd number of columns, with at least one pixel set to 1
        
        Parameters
        ----------
        kernel : cpl.core.Mask
            Mask of elements to use (for each pixel set to 1)
        filter : cpl.core.Filter
            cpl.core.Filter.EROSION, cpl.core.Filter.DILATION, cpl.core.Filter.OPENING, cpl.core.Filter.CLOSING 
        border : cpl.core.Border
            cpl.core.Border.NOP, cpl.core.Border.ZERO or cpl.core.Border.COPY 
        
        Raises
        ------
        cpl.core.DataNotFoundError 
            If the kernel is empty.
        cpl.core.AccessOutOfRangeError
            If the kernel has a side longer than the input mask.
        cpl.core.UnsupportedModeError 
            if the border/filter mode is unsupported.
        
        Returns
        ------
        Mask(self._mask.filter(kernel._mask, filter, border))
            filter_output mask same size as input
        
        Notes
        -----
        For erosion and dilation: In-place filtering is not supported, but the input buffer may overlap all but the 
        1+h first rows of the output buffer, where 1+2*h is the number of rows in the kernel.
        
        For opening and closing: Opening is implemented as an erosion followed by a dilation, and closing is implemented 
        as a dilation followed by an erosion. As such a temporary, internal buffer the size of self is allocated and used. 
        Consequently, in-place opening and closing is supported with no additional overhead, it is achieved by passing 
        the same mask as both self and other.
        
        Duality and idempotency: Erosion and Dilation have the duality relations: not(dil(A,B)) = er(not(A), B) and 
        not(er(A,B)) = dil(not(A), B).
        
        Opening and closing have similar duality relations: not(open(A,B)) = close(not(A), B) and not(close(A,B)) = open(not(A), B).
        
        Opening and closing are both idempotent, i.e. open(A,B) = open(open(A,B),B) and close(A,B) = close(close(A,B),B).
        
        The above duality and idempotency relations do not hold on the mask border (with the currently supported border modes).
        
        Unnecessary large kernels: Adding an empty border to a given kernel should not change the outcome of the filtering. However
        doing so widens the border of the mask to be filtered and therefore has an effect on the filtering of the mask border. Since 
        an unnecessary large kernel is also more costly to apply, such kernels should be avoided.
        
        1x3 erosion example:
        
        .. code-block:: python
        
            kernel = ~cpl.core.Mask(1,3)
            filtered.filter(kernel,cpl.core.Filter.EROSION,cpl.core.Border.NOP)
        """
    def flip(self, axis):
        """
        
        Flip a mask on a given mirror line. 
        
        Parameters
        ----------
        axis : int
            angle to mirror line in polar coord. is theta = (PI/4) * angle 
            - 0 (ϑ=0) to flip the image around the horizontal
            - 1 (ϑ=π∕4) to flip the image around y=x
            - 2 (ϑ=π∕2) to flip the image around the vertical
            - 3 (ϑ=3π∕4) to flip the image around y=-x
        
        Raises
        ------
        cpl.core.IllegalInputError
            if angle is not as specified
        """
    def insert(self, to_insert, ypos, xpos):
        """
        
        insert a mask into self
        
        Parameters
        ----------
        to_insert : cpl.core.Mask
            mask to insert into self
        ypos : int
            the y pixel position in self where the lower left pixel of to_insert should go 
            (from 0 to the y size of self)
        xpos : int
            the x pixel position in self where the lower left pixel of to_insert should go 
            (from 0 to the x size of in self)
        
        Raises
        ------
        cpl.core.IllegalInputError
            if xpos or ypos is outside self
        """
    def is_empty(self):
        ...
    def move(self, nb_cut, positions):
        """
        
        Reorganize the pixels in a mask. 
        
        nb_cut must be positive and divide the size of the input mask in x and y.
        
        Parameters
        ----------
        nb_cut : int
            the number of cut in x and y 
        new_pos : list of integers
            array with the nb_cut^2 new positions
        
        Raises
        ------
        cpl.core.IllegalInputError
            if nb_cut is not as requested.
        """
    def print(self):
        ...
    def rotate(self, turns):
        """
        
        Rotate this mask by a multiple of 90 degrees clockwise
        
        Parameters
        ----------
        turns : int
            Integral amount of 90 degree turns to execute.
        
        Notes
        -----
        `turns` can be any value, its modulo 4 determines rotation:
        
        - -3 to turn 270 degrees counterclockwise.
        - -2 to turn 180 degrees counterclockwise.
        - -1 to turn  90 degrees counterclockwise.
        -  0 to not turn
        - +1 to turn  90 degrees clockwise (same as -3)
        - +2 to turn 180 degrees clockwise (same as -2).
        - +3 to turn 270 degrees clockwise (same as -1).
        
        The lower left corner of the image is at (0,0), 
        x increasing from left to right, y increasing from bottom to top.
        """
    def save(self, filename, pl, mode):
        """
        
        Save a mask to a FITS file. 
        
        Parameters
        ----------
        filename : str
            Name of the file to write 
        pl : cpl.core.PropertyList
            Property list for the output header (Default None)
        mode : unsigned int 
            Desired output options, determined by bitwise or of cpl.core.io enums
        
        Raises
        ------
        cpl.core.IllegalInputError
            if the mode is unsupported
        cpl.core.NotCreatedError
            if the output file cannot be created
        cpl.core.FileIOError
            if the data cannot be written to the file
        
        Notes
        -----
        This function saves a mask to a FITS file. If a property list is provided, it is written to the header where the mask is written.
        
        The type used in the file is cpl.core.Type.UCHAR (8 bit unsigned).
        
        Supported output modes are cpl.core.io.CREATE (create a new file) and cpl.core.io.EXTEND (append a new extension to an existing file)
        
        The output mode cpl.core.io.EXTEND can be combined (via bit-wise or) with an option for tile-compression. This compression is lossless. 
        The options are: cpl.core.io.COMPRESS_GZIP, cpl.core.io.COMPRESS_RICE, cpl.core.io.COMPRESS_HCOMPRESS, cpl.core.io.COMPRESS_PLIO.
        
        Note that in append mode the file must be writable (and do not take for granted that a file is writable just because it was created 
        by the same application, as this depends from the system umask)
        """
    def shift(self, yshift, xshift):
        """
        
        shift a mask
        
        The 'empty zone' in the shifted mask is set to True. 
        
        The shift values have to be valid: -nx < dx < nx and -ny < dy < ny
        
        Parameters
        ----------
        yshift : int
            shift in y
        xshift : int
            shift in x
        
        Raises
        ------
        cpl.core.IllegalInputError
            if the offsets are too big
        """
    def subsample(self, ystep, xstep):
        """
        
        Subsample a mask.
        
        
        Parameters
        ----------
        ystep : int
            Take every ystep pixel in y
        xstep : int
            Take every xstep pixel in x  
        
        Raises
        ------
        cpl.core.IllegalInputError
            if xstep and ystep are not greater than zero
        """
    @property
    def height(self):
        """
        Number of rows high this mask is
        """
    @property
    def width(self):
        """
        Number of columns wide this mask is
        """
class Matrix:
    """
    
            This class provides the ability to create and interface with cpl_matrix.
            The elements of a cpl_matrix with M rows and N columns are counted from
            0,0 to M-1,N-1. The matrix element 0,0 is the one at the upper left
            corner of a matrix.
    
            The CPL matrix functions work properly only in the case the matrices
            elements do not contain garbage (such as NaN or infinity).
    
            Parameters
            ----------
            data : iterable of floats
              A 1d or 2d iterable containing matrix data to copy from. Any iterable should be compatible
              as long as it implements python's buffer protocol and only contains values of type float
              If a 1d iterable is given, `rows` must also be given to properly split the data into matrix rows.
            rows : int, optional
              Width of the new matrix. This will split `data` into `rows` number of rows to initialise the
              the new matrix. Should only be given if `data` is 1d, otherwise a ValueError exception is thrown.
        
    """
    class ThresholdMode:
        """
        Members:
        
          EPSILON : use machine DBL_EPSILON as the cutoff factor
        
          SIZE : compute the cutoff factor as 10*DBL_EPSILON*max(N, M)
        
          USER : use user-defined value as the cutoff factor
        """
        EPSILON: typing.ClassVar[Matrix.ThresholdMode]  # value = <ThresholdMode.EPSILON: 0>
        SIZE: typing.ClassVar[Matrix.ThresholdMode]  # value = <ThresholdMode.SIZE: 1>
        USER: typing.ClassVar[Matrix.ThresholdMode]  # value = <ThresholdMode.USER: 2>
        __members__: typing.ClassVar[dict[str, Matrix.ThresholdMode]]  # value = {'EPSILON': <ThresholdMode.EPSILON: 0>, 'SIZE': <ThresholdMode.SIZE: 1>, 'USER': <ThresholdMode.USER: 2>}
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
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def solve(coefficients: Matrix, rhs: Matrix) -> Matrix:
        """
                Solution of a linear system.
        
                Compute the solution of a system of N equations with N unknowns:
        
                coefficients * X = rhs
        
                coefficients must be an NxN matrix, and rhs a NxM matrix. M greater than 1 means that multiple independent
                right-hand-sides are solved for.
        
                rhs must have N rows and may contain more than one column, which each represent an independent
                right-hand-side.
        
                Parameters
                ----------
                coefficients : cpl.core.Matrix
                    The N x N matrix of coefficients
                rhs : cpl.core.Matrix
                    An N by M matrix containing one or more right-hand sides
        
                Returns
                -------
                cpl.core.Matrix
                    New solution cpl.core.Matrix with the same size as rhs
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if coefficients is not a square matrix
                cpl.core.IncompatibleInputError
                    if coefficients and rhs do not have the same number of rows
                cpl.core.SingularMatrixError
                    if coefficients is singular (to working precision)
        """
    @staticmethod
    def solve_normal(coefficients: Matrix, rhs: Matrix) -> Matrix:
        """
                Solution of overdetermined linear equations in a least squares sense.
        
                rhs may contain more than one column, which each represent an independent right-hand-side.
        
                Parameters
                ----------
                coefficients : cpl.core.Matrix
                    The N by M matrix of coefficients, where N >= M.
                rhs : cpl.core.Matrix
                    An N by K matrix containing K right-hand-sides.
        
                Return
                ------
                cpl.core.Matrix
                    A newly allocated M by K solution matrix
        
                Raises
                ----------
                cpl.core.IllegalInputError
                    if coefficients is not a square matrix
                cpl.core.IncompatibleInputError
                    if coefficients and rhs do not have the same number of rows
                cpl.core.SingularMatrixError
                    if coeff is singular (to working precision)
        
                Notes
                -----
                The following linear system of N equations and M unknowns is given:
        
                coeff * X = rhs
        
                where coeff is the NxM matrix of the coefficients, X is the MxK matrix of the unknowns, and rhs the NxK matrix
                containing the K right hand side(s).
        
                The solution to the normal equations is known to be a least-squares solution, i.e. the 2-norm of coeff * X - rhs
                is minimized by the solution to transpose(coeff) * coeff * X = transpose(coeff) * rhs.
        
                In the case that coeff is square (N is equal to M) it gives a faster and more accurate result to use
                cpl.core.Matrix.solve().
        """
    @staticmethod
    def solve_svd(coefficients: Matrix, rhs: Matrix, threshold_mode: typing.SupportsInt | typing.SupportsIndex | None = None, threshold_tol: typing.SupportsFloat | typing.SupportsIndex = 0) -> Matrix:
        """
                Solve a linear system in a least square sense using an SVD factorization, optionally discarding
                singular values below a given threshold.
        
                The function solves a linear system of the form Ax = b for the solution vector x, where A is
                represented by the argument `coefficients` and b by the argument `rhs`. 
                
                If `threshold_mode` and `threshold_tol` are passed, singular values which are less or equal than a
                given cutoff value are treated as zero. Otherwise all singular values are taken into account,
                regardless of their magnitude. This latter case is equivalent to setting `threshold_mode` to
                cpl.core.Matrix.ThresholdMode.USER and `threshold_tol` to 0.
        
                The argument `threshold_mode` is used to select the computation of the cutoff value for small
                singular values. If `threshold_mode` is set to cpl.core.Matrix.ThresholdMode.EPSILON the machine
                precision DBL_EPSILON is used as the cutoff factor. If `threshold_mode` is cpl.core.Matrix.ThresholdMode.SIZE,
                the cutoff factor is computed as 10*DBL_EPSILON*max(N, M), and if `threshold_mode`
                is cpl.core.Matrix.ThresholdMode.USER the argument `threshold_tol`, a value in the range [0,1]
                is used as the cutoff factor. The actual cutoff value, is then given by the cutoff factor times
                the biggest singular value obtained from the SVD of the matrix coefficients of`self`.
        
                Parameters
                ----------
                coefficients : cpl.core.Matrix
                    An N by M matrix of linear system coefficients, where N >= M
                rhs : cpl.core.Matrix
                    An N by 1 matrix with the right hand side of the system
                threshold_mode : cpl.core.Matrix.ThresholdMode, optional
                    Optional cutoff mode selector. used to select the computation of the cutoff value for small singular values.
                    Options:
                    - cpl.core.Matrix.ThresholdMode.EPSILON to use machine DBL_EPSILON as the cutoff factor
                    - cpl.core.Matrix.ThresholdMode.SIZE, where the cutoff factor is computed as 10*DBL_EPSILON*max(N, M)
                    - cpl.core.Matrix.ThresholdMode.USER, where the cutoff factor is set as the value passed to `threshold_tol`
                    For consistency with CPL, the integer values 0, 1 or 2 can also be passed instead of the symbolic
                    contants.
                threshold_tol : float, optional
                    Factor used to compute the cutoff value if `threshold_mode` is set to cpl.core.Matrix.ThresholdMode.USER.
                    Must be a value between 0. and 1. Defaults to 0. if not set, but is not used unless `threshold_mode`
                    is set to cpl.core.Matrix.ThresholdMode.USER. See Notes for more details.
        
                Return
                ------
                cpl.core.Matrix
                    A newly allocated M by 1 solution matrix
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `coefficients` and `rhs` do not have the same number of rows
                cpl.core.IllegalInputError
                    if matrix `rhs` has more than one column, an illegal mode
                    (not one of cpl.core.Matrix.ThresholdMode.EPSILON, cpl.core.Matrix.ThresholdMode.SIZE
                    or cpl.core.Matrix.ThresholdMode.USER, or their integer equivalents
                    0, 1, or 2), or an illegal tolerance (not between 0. and 1.) was given.
                
                Notes
                -----
                The linear system is solved using the singular value decomposition (SVD) of the coefficient matrix,
                based on a one-sided Jacobi orthogonalization.
        """
    @staticmethod
    def zeros(rows: typing.SupportsInt | typing.SupportsIndex, columns: typing.SupportsInt | typing.SupportsIndex) -> Matrix:
        """
              Create an matrix of columns x rows dimensions, all 0’s
        
              Parameters
              ----------
              rows : int
                  number of rows in the new matrix
              columns : int
                  number of columns in the new matrix
        
              Returns
              -------
              cpl.core.Matrix
                  New columns x rows matrix initialised with all 0’s
        """
    @typing.overload
    def __deepcopy__(self, arg0: dict) -> Matrix:
        ...
    @typing.overload
    def __deepcopy__(self, arg0: typing.Any) -> Matrix:
        ...
    @typing.overload
    def __eq__(self, arg0: Matrix) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> MatrixRow:
        """
        Get matrix row via index
        """
    @typing.overload
    def __getitem__(self, arg0: tuple[typing.Any, typing.Any]) -> Matrix:
        """
        Extract matrix values with slice
        """
    @typing.overload
    def __init__(self, data: collections.abc.Iterable, rows: typing.SupportsInt | typing.SupportsIndex | None = None) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: Matrix) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterate through the matrix rows
        """
    def __len__(self) -> int:
        ...
    def __matmul__(self, arg0: Matrix) -> Matrix:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def add(self, other: Matrix) -> None:
        """
                Perform matrix addition with `other` and `self`
        
                Add matrices `self` and `other` element by element. The two matrices must have
                identical sizes. The result is written to the `self`.
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    matrix to add with
        
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `other` does not have the same size as `self`
        """
    def add_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Add a scalar to `self`.
        
                Add the same value to each matrix element.
        
                Parameters
                ----------
                value : float
                    Value to add.
        """
    def append(self, other: Matrix, mode: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Append a matrix to another.
        
                If mode is set to 0, the matrices must have the same number of rows, and are
                connected horizontally with the `self` on the left. If mode is set to 1,
                the matrices must have the same number of columns, and are connected vertically
                with `self` on top. The `self` is expanded to include the values from the
                `other`, while `other` is left untouched.
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    matrix to append to `self`
                mode : int
                    Matrices connected horizontally (0) or vertically (1).
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `mode` is neither 0 nor 1
                cpl.core.IncompatibleInputError
                    	Matrices cannot be joined as indicated by `mode`.
        """
    def copy_values_from(self, submatrix: Matrix, row: typing.SupportsInt | typing.SupportsIndex, col: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Copy the values from another matrix into `self`
        
                The values of `submatrix` are written to `self` starting at the
                indicated row and column. There are no restrictions on the sizes of
                `submatrix`: just the parts of submatrix overlapping matrix are copied.
                There are no restrictions on row and col either, that can also be
                negative. If the two matrices do not overlap, nothing is done, but an
                error is raised.
        
                Parameters
                ----------
                submatrix : cpl.core.Matrix
                    Pointer to matrix to get the values from.
                row : int
                    Position of row 0 of `submatrix` in `self`.
                col : int
                    Position of column 0 of `submatrix` in `self`.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    No overlap exists between the two matrices.
        """
    def decomp_chol(self) -> None:
        """
                Replace a matrix by its Cholesky-decomposition, L * transpose(L) = A
        
                Notes
                -----
                Only the upper triangle of self is read, L is written in the lower triangle
                If the matrix is singular the elements of self become undefined
        """
    def decomp_lu(self) -> tuple[list[int], bool]:
        """
                Replace a matrix by its LU-decomposition
        
                `self` must be a n X n non-singular matrix to decompose. `self` will be modified
                inplace in which its values will be replaced with it's LU-decomposed values.
        
                The resulting LU decomposition can be solved with `cpl.core.Matrix.solve_lu`.
        
                Returns
                -------
                tuple(List[int], bool)
                    The pair of n-integer list filled with row permutations (perm) and True/False for even number of
                    permutations (psig). In the format (perm, psig).
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `self` is not an n by n matrix.
                cpl.core.SingularMatrixError
                    `self` is singular.
                cpl.core.IncompatibleInputError
                    `self` and `perm` have incompatible sizes.
                cpl.core.TypeMismatchError
                    `perm` is not a list of ints
        
                Notes
                -----
                Algorithm reference: Golub & Van Loan, Matrix Computations, Algorithms 3.2.1 (Outer Product
                Gaussian Elimination) and 3.4.1 (Gauss Elimination with Partial Pivoting).
        
                See Also
                --------
                cpl.core.Matrix.solve_lu : Used to solve the LU-decomposition
        """
    def determinant(self) -> float:
        """
                Compute the determinant of a matrix.
        
                `self` must be a square matrix. In case of a 1x1 matrix, the matrix
                single element value is returned.
        
                Returns
                -------
                float
                    Matrix determinant
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `self` is not square.
                cpl.core.UnspecifiedError
                    `self` is near-singular with a determinant so close to zero that it cannot be
                    represented by a double.
        """
    def divide(self, other: Matrix) -> None:
        """
                Divide `self` by `other`, element by element.
        
                Divide each element of `self` by the corresponding
                element of the second one. The two matrices must have the same
                number of rows and columns. The result is written to the first
                matrix. No check is made against a division by zero.
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    matrix to divide with
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `other` does not have the same size as `self`
        """
    def divide_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Divide `self` by a scalar.
        
                Divide each matrix element by the same value.
        
                Parameters
                ----------
                value : float
                    Divisor.
        
                Raises
                ------
                cpl.core.DivisionByZeroError
                    `value` is 0.0
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                    Dump the matrix contents to a file, stdout or a string.
        
                    This function is intended just for debugging. It just prints the elements of a matrix, ordered in rows and columns
                    to the file path specified by `filename`. 
                    If a `filename` is not specified, output goes to stdout (unless `show` is False).
        
                    Parameters
                    ----------
                    filename : str, optional
                        File to dump matrix contents to
                    mode : str, optional
                        Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                        but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                        it if it does).
                    show : bool, optional
                        Send matrix contents to stdout. Defaults to True.
        
                    Returns
                    -------
                    str 
                        Multiline string containing the dump of the matrix contents.
        """
    def erase_columns(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Delete columns from a matrix.
        
                A portion of the matrix data is removed. The specified
                segment can extend beyond the end of the matrix, but an attempt to
                remove all matrix columns will raise an exception because zero length
                matrices are illegal. Columns are counted starting from 0.
        
                Parameters
                ----------
                start : int
                    First column to delete.
                count : int
                    Number of columns to delete.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified start is outside the matrix boundaries.
                cpl.core.IllegalInputError
                    count is not positive.
                cpl.core.IllegalOutputError
                    Attempt to delete all the columns of matrix.
        """
    def erase_rows(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Delete rows from a matrix.
        
                A select number of rows will be completely removed from the object,
                reducing the total number of rows by 1. The specified segment can
                extend beyond the end of the matrix, but an attempt to remove all
                matrix rows will raise an exception because zero length matrices
                are illegal. Rows are counted starting from 0.
        
                Parameters
                ----------
                start : int
                    First row to delete.
                count : int
                    Number of rows to delete.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified start is outside the matrix boundaries.
                cpl.core.IllegalInputError
                    count is not positive.
                cpl.core.IllegalOutputError
                    Attempt to delete all the rows of matrix.
        """
    def exponential(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the exponential of matrix elements.
        
                Each matrix element is replaced by its exponential in the specified base.
                The base must be positive.
        
                Parameters
                ----------
                base : float
                    Exponential base.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `base` is not positive
        """
    def extract_column(self, column: typing.SupportsInt | typing.SupportsIndex) -> Matrix:
        """
                Copy a matrix column.
        
                If a MxN matrix is given in input, the extracted row is a new Mx1 matrix.
                The column number is counted from 0.
        
                Parameters
                ----------
                column : int
                    Sequence number of column to copy.
        
                Returns
                -------
                cpl.core.Matrix
                    Mx1 Matrix containing the extracted column values.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The column is outside the matrix boundaries.
        """
    def extract_diagonal(self, diagonal: typing.SupportsInt | typing.SupportsIndex) -> Matrix:
        """
                Extract a matrix diagonal.
        
                If a MxN matrix is given in input, the extracted diagonal is a Mx1
                matrix if :math:`N >= M`, or a 1xN matrix if :math:`N < M`. The diagonal number is
                counted from 0, corresponding to the matrix diagonal starting at
                element (0,0). A square matrix has just one diagonal; if M != N,
                the number of diagonals in the matrix is :math:`|M - N|` + 1. To specify
                a  diagonal sequence number outside this range raises a
                `cpl.core.AccessOutOfRangeError`
        
                Parameters
                ----------
                diagonal : int
                    Sequence number of the diagonal to copy.
        
                Returns
                -------
                cpl.core.Matrix
                    matrix with either 1xN or Mx1 dimensions containing the extracted diagonal.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError:
                    If the `diagonal` is outside the matrix boundaries
        """
    def extract_row(self, row: typing.SupportsInt | typing.SupportsIndex) -> Matrix:
        """
                Extract a matrix row.
        
                If a MxN matrix is given in input, the extracted row is a new 1xN matrix.
                The row number is counted from 0.
        
                Parameters
                ----------
                row : int
                    Sequence number of row to copy.
        
                Returns
                -------
                cpl.core.Matrix
                    New matrix representing the row.
        
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The row is outside the matrix boundaries.
        """
    def fill(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Write the same value to all matrix elements.
        
                Parameters
                ----------
                value : float
                    Value to write
        """
    def fill_column(self, value: typing.SupportsFloat | typing.SupportsIndex, column: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Write the same value to a matrix column.
        
                Write the same value to a matrix column. Columns are counted starting
                from 0.
        
                Parameters
                ----------
                value : float
                    Value to write
                column : int
                    Sequence number of column to overwrite
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified column is outside the matrix boundaries.
        """
    def fill_diagonal(self, value: typing.SupportsFloat | typing.SupportsIndex, diagonal: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Write a given value to all elements of a given matrix diagonal.
        
                Parameters
                ----------
                value : float
                    Value to write to diagonal
                diagonal : int
                    Number of diagonal to overwrite, 0 for main, positive for
                    above main, negative for below main.
                    main is the diagonal starting from (0, 0) on the matrix.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified diagonal is outside the matrix boundaries.
        """
    def fill_row(self, value: typing.SupportsFloat | typing.SupportsIndex, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Write the same value to a matrix row.
        
                Write the same value to a matrix row. Rows are counted starting from 0.
        
                Parameters
                ----------
                value : float
                    Value to write
                row : int
                    Sequence number of row to overwrite.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified row is outside the matrix boundaries.
        """
    def fill_window(self, value: typing.SupportsFloat | typing.SupportsIndex, row: typing.SupportsInt | typing.SupportsIndex, col: typing.SupportsInt | typing.SupportsIndex, nrow: typing.SupportsInt | typing.SupportsIndex, ncol: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Write the same value into a submatrix of a matrix.
        
                The specified value is written to `self` starting at the indicated
                row and column; `nrow` and `ncol` can exceed `self`
                boundaries, just the range overlapping the `self` is used in that
                case.
        
                Parameters
                ----------
                value : float
                    Value to write.
                row : int
                    Start row of matrix submatrix.
                col : int
                    Start column of matrix submatrix.
                nrow : int
                    Number of rows of matrix submatrix.
                ncol : int
                    Number of columns of matrix submatrix.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified start position is outside the matrix boundaries.
                cpl.core.IllegalInputError
                    nrow or ncol are not positive.
        """
    def flip_columns(self) -> None:
        """
                Reverse order of columns in matrix.
        
                The order of the columns in the matrix is reversed in place.
        """
    def flip_rows(self) -> None:
        """
                Reverse order of rows in matrix.
        
                The order of the rows in the matrix is reversed in place.
        """
    def invert_create(self) -> Matrix:
        """
                Find a matrix inverse of `self`
        
                `self` must be a square matrix.
        
                Returns
                -------
                cpl.core.Matrix
                    Inverse matrix.
        
        
                Notes
                -----
                When calling  invert_create() with a nearly singular
                matrix, it is possible to get a result containin NaN values without
                any error code being set.
        """
    def is_diagonal(self, tolerance: typing.SupportsFloat | typing.SupportsIndex | None = None) -> bool:
        """
                Check if a matrix is diagonal.
        
                A threshold may be specified to consider zero any number that
                is close enough to zero. If the specified `tolerance` is
                negative (default), the default value is used, equal to the
                machine double epsilon. A zero tolerance may also be specified.
        
                No error is raised if the `self` is not square.
        
                Parameters
                ----------
                tolerance : float
                    Max tolerated rounding to zero. If not given the machine
                    double epsilon is used.
        
                Returns
                -------
                bool
                    True if `self` is a diagonal matrix. False otherwise
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the matrix is not square
        
                Notes
                -----
                If tolerance is given a negative value, the default value for tolerance
                will be used (machine double epsilon).
        """
    def is_identity(self, tolerance: typing.SupportsFloat | typing.SupportsIndex | None = None) -> bool:
        """
                Check for identity matrix.
        
                A threshold may be specified to consider zero any number that is
                close enough to zero, and 1 any number that is close enough to 1.
                If `tolerance` is not given, the default
                value is used, equal to the machine double epsilon. A zero
                tolerance may also be specified.
        
                No error is raised if the `self` is not square.
        
                Parameters
                ----------
                tolerance : float, optional
                    Max tolerated rounding to zero, or to one. If not given the machine
                    double epsilon is used.
        
                Returns
                -------
                bool
                    True if `self` is a identity matrix. False otherwise
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the matrix is not square
        
                Notes
                -----
                If tolerance is given a negative value, the default value for tolerance
                will be used (machine double epsilon).
        """
    def is_zero(self, tolerance: typing.SupportsFloat | typing.SupportsIndex | None = None) -> bool:
        """
                Check for zero matrix.
        
                After specific manipulations of a matrix some of its elements
                may theoretically be expected to be zero. However, because of
                numerical noise, such elements may turn out not to be exactly
                zero. In this specific case, if any of the matrix element is
                not exactly zero, the matrix would not be classified as a null
                matrix. A threshold may be specified to consider zero any number
                that is close enough to zero.
        
                If no `tolerance` is given then the default value is used, equal to the
                machine double epsilon.
        
                Parameters
                ----------
                tolerance : float
                    Max tolerated rounding to zero. If not given the machine
                    double epsilon is used.
        
                Returns
                -------
                bool
                    True if `self` is a zero matrix. False otherwise
        
                Notes
                -----
                If tolerance is given a negative value, the default value for tolerance
                will be used (machine double epsilon).
        """
    def logarithm(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the logarithm of matrix elements.
        
                Each matrix element is replaced by its logarithm in the specified base.
                The base and all matrix elements must be positive.
        
                Parameters
                ----------
                base : float
                    Logarithm base.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `base` or any element of `self` is not positive
        """
    def max(self) -> float:
        """
                Find the maximum value of all matrix elements.
        
                The maximum value of matrix elements is found.
        
                Returns
                -------
                float
                    Maximum value in the matrix
        """
    def maxpos(self) -> tuple[int, int]:
        """
                Find position of maximum value of matrix elements.
        
                The position of the maximum value of all matrix elements is found.
                If more than one matrix element have a value corresponding to
                the maximum, the lowest element row number is returned in  row.
                If more than one maximum matrix elements have the same row number,
                the lowest element column number is returned in column.
        
                Returns
                -------
                tuple(int, int)
                    tuple in the format (row, column), where:
                    - row is the returned row position of maximum.
                    - column is the returned column position of maximum
        """
    def mean(self) -> float:
        """
                Find the mean of all matrix elements.
        
                The mean of all matrix elements is calculated
        
                Returns
                -------
                float
                    Mean of all matrix elements
        
                Notes
                -----
                This function works properly only if all elements of the matrix have
                finite values (not NaN or Infinity).
        """
    def median(self) -> float:
        """
                Find the median of all matrix elements.
        
                The median of all matrix elements is calculated
        
                Returns
                -------
                float
                    Median of all matrix elements
        """
    def min(self) -> float:
        """
                Find the minimum value of all matrix elements.
        
                The minimum value of matrix elements is found.
        
                Returns
                -------
                float
                    Minimum value in the matrix
        """
    def minpos(self) -> tuple[int, int]:
        """
                Find position of minimum value of matrix elements.
        
                The position of the minimum value of all matrix elements is found.
                If more than one matrix element have a value corresponding to
                the minimum, the lowest element row number is returned in  row.
                If more than one minimum matrix elements have the same row number,
                the lowest element column number is returned in column.
        
                Returns
                -------
                tuple(int, int)
                    tuple in the format (row, column), where:
                    - row is the returned row position of minimum.
                    - column is the returned column position of minimum
        """
    def multiply(self, other: Matrix) -> None:
        """
                Multiply `self` by `other`, element by element. The two matrices must
                have identical sizes. The result is written to `self`.
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    matrix to multiply with
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `other` does not have the same size as `self`
        
                Notes
                -----
                To obtain the rows-by-columns product between two matrices, use product_create()
        
                See Also
                --------
                cpl.core.matrix.product_create : Rows-by-columns product of two matrices
                cpl.core.matrix.multiply_scalar : Multiply `self` by a scalar.
        """
    def multiply_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Multiply `self` by a scalar.
        
                Multiply each matrix element by the same factor.
        
                Parameters
                ----------
                value : float
                    Multiplication factor.
        
                See Also
                ----------
                cpl.core.matrix.multiply : Multiply `self` by `other`, element by element.
                cpl.core.matrix.product_create : Rows-by-columns product of two matrices.
        """
    def power(self, exponent: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute a power of matrix elements.
        
                Each matrix element is replaced by its power to the specified
                exponent. If the specified exponent is not negative, all matrix
                elements must be not negative; if the specified exponent is
                negative, all matrix elements must be positive; otherwise, an
                error condition is set and the matrix will be left unchanged.
                If the exponent is exactly 0.5 the (faster)  sqrt() will be
                applied instead of  pow(). If the exponent is zero, then any
                (non negative) matrix element would be assigned the value 1.0.
        
                Parameters
                ----------
                exponent : float
                    Constant exponent.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    Any element of `self` is not compatible with `exponent` (see extended summary)
        """
    def product_create(self, rhs: Matrix) -> Matrix:
        """
                Rows-by-columns product of two matrices.
        
                The number of columns of the first matrix must be equal to the
                number of rows of the second matrix.
        
                Can also use the ``@`` operator to call this function for example with
                ``lhs`` as the calling object:
        
                .. code-block:: python
                
                  product = lhs @ rhs
        
                Parameters
                ----------
                rhs : cpl.core.Matrix
                    Right side matrix to get the product with the calling object
        
                Returns
                -------
                cpl.core.Matrix
                    The rows-by-columns product of calling matrix and rhs matrix
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    The number of columns of the calling matrix is not equal to
                    the number of rows of the rhs matrix.
        
                See Also
                --------
                cpl.core.matrix.multiply : Multiply `self` by `other`, element by element.
                cpl.core.matrix.multiply_scalar : Multiply `self` by a scalar.
                cpl.core.matrix.product_normal : Compute A = B * transpose(B)
        """
    def product_normal(self) -> Matrix:
        """
                Compute A = B * transpose(B)
        
                self * transpose(self)
                Matrix multiplication results in a matrix of the size
                [rows of left] * [columns of right]
                Here, left = self, right = transpose(self)
                and the rows/columns of a transpose(self) are flipped from a self
                so the result of the multiplication is [rows of self] * [columns of
                transpose(self)], simplifies into  [rows of self] * [rows of self]
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    M x N Matrix to multiply with its transpose
        
                Returns
                -------
                cpl.core.Matrix
                    Resulting matrix
        
                Notes
                -----
                Only the upper triangle of A is computed, while the elements below the main diagonal have undefined values.
        
                See Also
                --------
                cpl.core.product_create : Rows-by-columns product of two matrices.
                cpl.core.multiply : Multiply `self` by `other`, element by element.
                cpl.core.matrix.multiply_scalar : Multiply `self` by a scalar.
        """
    def product_transpose(self, ma: Matrix, mb: Matrix) -> None:
        """
                Fill a matrix with the product of A * B'
        
                Parameters
                ----------
                ma : cpl.core.Matrix
                    The matrix A, of size M x K
                mb : cpl.core.Matrix
                    The matrix B, of size N x K
        
                Notes
                -----
                The use of the transpose of B causes a more efficient memory access
                Changing the order of A and B is allowed, it transposes the result
        """
    def resize(self, top: typing.SupportsInt | typing.SupportsIndex, bottom: typing.SupportsInt | typing.SupportsIndex, left: typing.SupportsInt | typing.SupportsIndex, right: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Resize a matrix by adding or removing rows and/or columns from the edges.
        
                `self` is reframed according to specifications. Extra rows
                and column on the sides might also be negative, as long as they are
                compatible with the matrix sizes: `self` would be reduced
                in size accordingly, but an attempt to remove all matrix columns
                and/or rows raises an exception because zero length matrices are
                illegal. The old matrix elements contained in the new shape are left
                unchanged, and new matrix elements added by the reshaping are initialised
                to zero. No reshaping (i.e., all the extra rows set to zero) would
                not raise an exception.
        
                Parameters
                ----------
                top : int
                    Extra rows on top.
                bottom : int
                    Extra rows on bottom.
                left : int
                    Extra columns on left.
                right : int
                    Extra columns on right.
        
                Raises
                ------
                cpl.core.IllegalOutputError
                    Attempt to shrink `self` to zero size (or less).
        """
    def set_size(self, rows: typing.SupportsInt | typing.SupportsIndex, columns: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Resize a matrix.
        
                `self` is resized according to specifications. The old
                matrix elements contained in the resized matrix are left unchanged,
                and new matrix elements will be added by an increase of the matrix number of
                rows and/or columns are initialised to zero. New rows and/or columns
                will be added to the right/bottom of `self`. Likewise when
                shrinking the matrix by one of the dimensions, the rows/columns will be
                removed from the right/bottom of `self`.
        
                Parameters
                ----------
                rows : int
                    New number of rows.
                columns : int
                    New number of columns.
        
                Raises
                ------
                cpl.core.IllegalOutputError
                    Attempt to shrink matrix to zero size (or less).
        """
    def shift(self, rshift: typing.SupportsInt | typing.SupportsIndex, cshift: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Shift matrix elements.
        
                The performed shift operation is cyclical (toroidal), i.e., matrix
                elements shifted out of one side of the matrix get shifted in from
                its opposite side. There are no restrictions on the values of the
                shift. Positive shifts are always in the direction of increasing
                row/column indexes.
        
                Parameters
                ----------
                rshift : int
                    Shift in the vertical direction.
                cshift : int
                    Shift in the horizontal direction.
        """
    def solve_chol(self, rhs: Matrix) -> None:
        """
                Solve a L*transpose(L)-system
        
                Parameters
                ----------
                rhs : cpl.core.Matrix
                    M right-hand-sides to be replaced by their solution
        
                Notes
                -----
                Only the lower triangle of self is accessed
        """
    def solve_lu(self, rhs: Matrix, perm: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex] | None = None) -> Matrix:
        """
                Solve a LU-system
        
                `self` should be a n x n LU-matrix that has been decomposed using
                `self.decomp_lu()`
        
                Parameters
                ----------
                rhs : cpl.core.Matrix
                    m right-hand-sides. This is duplicated and replaced by the solution of `self` to generate the
                    return matrix.
                perm : list of ints
                    n-integer array filled with the row permutations
        
                Returns
                -------
                cpl.core.Matrix
                    The solution of `self` as applied to `rhs`
        
                Raises
                ------
                cpl.core.IllegalInputError
                    `self` is not an n by n matrix
                cpl.core.IncompatibleInputError
                    The array or matrices not have the same number of rows.
                cpl.core.DivisionByZeroError
                    The main diagonal of U contains a zero. This error can only occur
                    if the LU-matrix does not come from a successful call to
                    `cpl.core.Matrix.decomp_lu`.
        
                See Also
                --------
                cpl.core.Matrix.decomp_lu : Used to generate an LU-system which can then be solved using this method.
        """
    def sort_columns(self, by_absolute: bool = False) -> None:
        """
                Sort matrix by columns.
        
                The matrix elements of the top row are used as reference for the column sorting,
                if there are identical the values of the second row are considered, etc.
                Columns with the greater values go to left.
        
                Parameters
                ----------
                by_absolute : bool, optional
                    True to sort by absolute value. Default False.
        """
    def sort_rows(self, by_absolute: bool = False) -> None:
        """
                Sort matrix by rows.
        
                The matrix elements of the leftmost column are used as reference for the row sorting,
                if there are identical the values of the second column are considered, etc.
                Rows with the greater values go on top.
        
                Parameters
                ----------
                by_absolute : bool, optional
                    True to sort by absolute value. Default False.
        """
    def stdev(self) -> float:
        """
                Find the standard deviation of all matrix elements.
        
                The standard deviation of all matrix elements is calculated
        
                Returns
                -------
                float
                    Standard deviation of all matrix elements
        
                Notes
                -----
                This function works properly only if all elements of the matrix have
                finite values (not NaN or Infinity).
        """
    def subtract(self, other: Matrix) -> None:
        """
                Subtract matrix `other` from `self`
        
                Subtract `other` from `self` element by element.
                The two matrices must have identical sizes. The result is written
                to `self`.
        
                Parameters
                ----------
                other : cpl.core.Matrix
                    matrix to subtract with
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `other` does not have the same size as `self`
        """
    def subtract_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Subtract a scalar to `self`.
        
                Subtract the same value to each matrix element.
        
                Parameters
                ----------
                value : float
                    Value to subtract.
        """
    def swap_columns(self, column1: typing.SupportsInt | typing.SupportsIndex, column2: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Swap two matrix columns.
        
                The values of two given matrix columns are exchanged. Columns are
                counted starting from 0. If the same column number is given twice,
                nothing is done and no exception is raised.
        
                Parameters
                ----------
                column1 : int
                    One matrix column.
                column2 : int
                    Another matrix column.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    Either of the specified columns is outside the matrix boundaries.
        """
    def swap_rowcolumn(self, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Swap a matrix column with a matrix row.
        
                The values of the indicated row are exchanged with the column having
                the same sequence number. Rows and columns are counted starting from 0.
        
                Parameters
                ----------
                row : int
                    Matrix row.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The specified row is outside the matrix boundaries.
                cpl.core.IllegalInputError
                    `self` is not square.
        """
    def swap_rows(self, row1: typing.SupportsInt | typing.SupportsIndex, row2: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Swap two matrix rows.
        
                The values of two given matrix rows are exchanged. Rows are counted
                starting from 0. If the same row number is given twice, nothing is
                done and no exception is raised.
        
                Parameters
                ----------
                row1 : int
                    One matrix row.
                row2 : int
                    Another matrix row.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    Either of the specified rows is outside the matrix boundaries.
        """
    def threshold_small(self, tolerance: typing.SupportsFloat | typing.SupportsIndex | None = None) -> None:
        """
                Rounding to zero very small numbers in matrix.
        
                After specific manipulations of a matrix some of its elements
                may theoretically be expected to be zero (for instance, as a result
                of multiplying a matrix by its inverse). However, because of
                numerical noise, such elements may turn out not to be exactly
                zero. With this function any very small number in the matrix is
                turned to exactly zero.
        
                If no `tolerance` is given then the default value is used, equal to the
                machine double epsilon.
        
                Parameters
                ----------
                tolerance : float, optional
                    Max tolerated rounding to zero. If not given the machine
                    double epsilon is used.
        
                Notes
                -----
                If tolerance is given a negative value, the default value for tolerance
                will be used (machine double epsilon).
        """
    def transpose_create(self) -> Matrix:
        """
                Returns the transpose of the matrix in a new matrix.
        
                Returns
                -------
                cpl.core.Matrix
                    New transposed matrix.
        """
    @property
    def height(self) -> int:
        ...
    @property
    def shape(self) -> tuple[int, int]:
        ...
    @property
    def width(self) -> int:
        ...
class MatrixRow:
    """
    
            Returned from a Matrix's __getitem__ method or iterator. Used to access specific rows of the matrix.
    
            Not instantiatable on its own.
        
    """
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> float:
        ...
    def __init__(self, arg0: Matrix, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        ...
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def index(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> int:
        ...
class Msg:
    """
    
            This module provides functions to display and log messages. The following operations are supported:
            
            - Enable messages output to terminal or to log file.
            - Optionally adding informative tags to messages.
            - Setting width for message line wrapping.
            - Control the message indentation level.
            - Filtering messages according to their severity level.
    
            This module is configured via the `set_config` method and controls how messages are output.
            
    """
    class SeverityLevel:
        """
        Members:
        
          DEBUG
        
          INFO
        
          WARNING
        
          ERROR
        
          OFF
        """
        DEBUG: typing.ClassVar[Msg.SeverityLevel]  # value = <SeverityLevel.DEBUG: 0>
        ERROR: typing.ClassVar[Msg.SeverityLevel]  # value = <SeverityLevel.ERROR: 3>
        INFO: typing.ClassVar[Msg.SeverityLevel]  # value = <SeverityLevel.INFO: 1>
        OFF: typing.ClassVar[Msg.SeverityLevel]  # value = <SeverityLevel.OFF: 4>
        WARNING: typing.ClassVar[Msg.SeverityLevel]  # value = <SeverityLevel.WARNING: 2>
        __members__: typing.ClassVar[dict[str, Msg.SeverityLevel]]  # value = {'DEBUG': <SeverityLevel.DEBUG: 0>, 'INFO': <SeverityLevel.INFO: 1>, 'WARNING': <SeverityLevel.WARNING: 2>, 'ERROR': <SeverityLevel.ERROR: 3>, 'OFF': <SeverityLevel.OFF: 4>}
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
    def debug(component: str, message: str) -> None:
        """
                Display a debug message. 
        
                Parameters
                ----------
                component : str
                        Name of the function generating the message. 
                message : str
                        Message to output
        
                Notes
                -----
                The `show_component` option in the config must be set to True for the component to be visible. 
        """
    @staticmethod
    def error(component: str, message: str) -> None:
        """
                Display an error message. 
        
                Parameters
                ----------
                component : str
                        Name of the function generating the message. 
                message : str
                        Message to output
        
                Notes
                -----
                The `show_component` option in the config must be set to True for the component to be visible.
        """
    @staticmethod
    def get_config() -> dict:
        """
        Gets current CPL Messaging configuration
        """
    @staticmethod
    def info(component: str, message: str) -> None:
        """
                Display an information message. 
        
                Parameters
                ----------
                component : str
                        Name of the function generating the message. 
                message : str
                        Message to output
        
                Notes
                -----
                The `show_component` option in the config must be set to True for the component to be visible.
        """
    @staticmethod
    def set_config(**kwargs) -> None:
        """
                Set CPL Messaging configuration via kwargs as seen in the parameters below
        
                Parameters
                ----------
                level : cpl.core.Msg.SeverityLevel, optional
                        Verbosity level, message below said verbosity level are not printed
                domain : str, optional
                        The domain name, also known as a task identifier, typically a pipeline recipe name.
                width : int, optional
                        The maximum width of the displayed text. 
                indent : int, optional
                        The indentation level. Messages are indented by a number of characters equal to the level. 
                        Specifying a negative indentation level would set the indentation level to zero. 
                show_threadid : bool, optional
                        True to attach the threadid tag with the messages
                show_domain : bool, optional
                        True to attach the domain name tag with the messages
                show_time : bool, optional
                        True to attach the time tag with the messages
                show_component : bool, optional
                        True to attach the component tag with the messages
        """
    @staticmethod
    def start_file(verbosity: Msg.SeverityLevel, name: str = '.logfile') -> None:
        """
                Begin log to file.
        
                Typically called at the start of a script.
        
                If this has already been called previously, the previous file log will stop and restart with the new logger.
        
                Parameters
                ----------
                Verbosity : cpl.core.Msg.SeverityLevel 
                    Verbosity level
                name : str
                    Filename to begin logging to
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If name is longer than 72 characters
        """
    @staticmethod
    def stop_file() -> None:
        """
        Close the current log file if running. Will not throw an error if logging is not currently active. This routine may be called in case the logging should be terminated before the end of a program.
        """
    @staticmethod
    def warning(component: str, message: str) -> None:
        """
                Display a warning message. 
        
                Parameters
                ----------
                component : str
                        Name of the function generating the message. 
                message : str
                        Message to output
        
                Notes
                -----
                The `show_component` option in the config must be set to True for the component to be visible.
        """
class NoWCSError(Error, RuntimeError):
    """
    The WCS functionalities are missing.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 12
class Polynomial:
    """
    
            This module provides functions to handle uni- and multivariate polynomials.
    
            Comparing the two polynomials
    
                P1(x) = p0 + p1.x + p4.x^2
    
            and
            
                P2(x,y) = p0 + p1.x + p2.y + p3.x.y + p4.x^2 + p5.y^2
    
            P1(x) may evaluate to more accurate results than P2(x,0), especially around the roots.
            The base constructor creates a new polynomial with degree 0 and evaluates as 0.
    
            Parameters
            ----------
            dim : int
                The positive polynomial dimensions (number of variables)
    
            Notes
            -----
            Note that a polynomial like P3(z) = p0 + p1.z + p2.z^2 + p3.z^3, z=x^4 is preferable to p4(x) = p0 + p1.x^4 + p2.x^8 + p3.x^12.
            Polynomials are evaluated using Horner's method. For multivariate polynomials the evaluation is performed one dimension at a time,
            starting with the lowest dimension and proceeding upwards through the higher dimensions.
            
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def from_numpy(data: typing.Any) -> Polynomial:
        """
                Construct a CPL polynomial from a numpy polynomial
        
                Parameters
                ----------
                data : np.polynomial.polynomial.Polynomial
                    Input polynomial
        """
    def __deepcopy__(self, arg0: dict) -> Polynomial:
        ...
    @typing.overload
    def __eq__(self, arg0: Polynomial) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __init__(self, dim: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __ne__(self, arg0: Polynomial) -> bool:
        ...
    @typing.overload
    def __ne__(self, arg0: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def add(self, other: Polynomial) -> None:
        """
                Add a polynomial with the same dimension as `self`
        
                Parameters
                ----------
                other : cpl.core.Polynomial
                    polynomial to add
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `self` and `other` do not have identical dimensions
        """
    def compare(self, other: Polynomial, tol: typing.SupportsFloat | typing.SupportsIndex) -> int:
        """
                Compare the coefficients of two polynomials
        
                The two polynomials are considered equal if they have identical
                dimensions and the absolute difference between their coefficients
                does not exceed the given tolerance.
        
                This means that the following two polynomials per definition are
                considered different:
        
                .. code-block:: none
        
                    P1(x1) = 3*x1 different from P2(x1,x2) = 3*x1.
        
                If all input parameters are valid and `self` and `other` point to the same
                polynomial the function returns 0.
        
                If two polynomials have different dimensions, the return value is this
                (positive) difference.
        
                If two 1D-polynomials differ, the return value is 1 plus the degree of the
                lowest order differing coefficient.
        
                If for a higher dimension they differ, it is 1 plus the degree of a
                differing coefficient.
        
                Parameters
                ----------
                other : cpl.core.Polynomial
                    The 2nd polynomial
                tol : float
                    The absolute (non-negative) tolerance
        
                Returns
                -------
                int
                    0 when equal, positive when different, negative on error.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    If `tol` is negative
        """
    def copy(self) -> Polynomial:
        """
                Return a copy of the Polynomial.
        
                Returns
                -------
                cpl.core.Polynomial
                    A new Polynomial containing a copy of the coefficients of the original.
        """
    def derivative(self, dim: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Compute a first order partial derivative
        
                The dimension of the polynomial is preserved, even if the operation may cause
                the polynomial to become independent of the dimension `dim` variable.
        
                `self` is modified to store the result
        
                Parameters
                ----------
                dim : int
                    The dimension to differentiate (zero for first dimension)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if `dim` is negative.
                cpl.core.AccessOutOfRangeError
                    if `dim` exceeds the dimension of `self`.
        
                Notes
                -----
                The call requires n FLOPs, where n is the number of (non-zero) polynomial
                coefficients whose power in dimension dim is at least 1.
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                    Dump the Polynomial contents to a file, stdout or a string.
        
                    This function is intended just for debugging. It just prints the elements of a polynomial, ordered in rows and columns
                    to the file path specified by `filename`. 
                    If a `filename` is not specified, output goes to stdout (unless `show` is False).
        
                    Parameters
                    ----------
                    filename : str, optional
                        File to dump polynomial contents to
                    mode : str, optional
                        Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                        but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                        it if it does).
                    show : bool, optional
                        Send polynomial contents to stdout. Defaults to True.
        
                    Returns
                    -------
                    str 
                        Multiline string containing the dump of the polynomial contents.
        """
    @typing.overload
    def eval(self, x: Vector) -> float:
        """
                Evaluate the polynomial at the given point using Horner's rule.
        
                A polynomial with no non-zero coefficients evaluates as 0.
        
                The length of `x` must match the polynomial dimension.
        
                :Parameters:
                    **x** (*cpl.core.Vector*) -- Points of evaluation
        
                :Returns: The computed value or undefined on error. 
                :Return type: float
        
                :Raises:
                    **cpl.core.IncompatibleInputError** -- if the length of `x` differs from the dimension
        
                **Notes**
                
                With n coefficients the complexity is about 2n FLOPs.
        """
    @typing.overload
    def eval(self, x: collections.abc.Iterable) -> float:
        """
                Evaluate the polynomial at the given point using Horner's rule.
        
                A polynomial with no non-zero coefficients evaluates as 0.
        
                The length of `x` must match the polynomial dimension.
        
                :Parameters:
                    **x** (*iterable of float*) -- Points of evaluation
        
                :Returns: The computed value or undefined on error. 
                :Return type: float
        
                :Raises:
                    **cpl.core.IncompatibleInputError** -- if the length of `x` differs from the dimension
        
                **Notes**
                
                With n coefficients the complexity is about 2n FLOPs.
        """
    def eval_1d(self, x: typing.SupportsFloat | typing.SupportsIndex) -> tuple[float, float]:
        """
                Evaluate a univariate (1D) polynomial using Horner's rule.
        
                A polynomial with no non-zero coefficents evaluates to 0 with a derivative that does likewise.
        
                Parameters
                ----------
                x : float
                    Point of evaluation
        
                Returns
                -------
                tuple(float, float)
                    in the format (result, pd), where pd is the derivative evaluated at x
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial is not (1D) univariate
        
                Notes
                -----
                The result is computed as :math:`p_0 + x * ( p_1 + x * ( p_2 + ... x * p_n ))`
                and requires 2n FLOPs where n+1 is the number of coefficients.
        
                The derivative is computed using a nested Horner rule.
        """
    def eval_1d_diff(self, a: typing.SupportsFloat | typing.SupportsIndex, b: typing.SupportsFloat | typing.SupportsIndex) -> tuple[float, float]:
        """
                Evaluate p(a) - p(b) using Horner's rule.
        
                Parameters
                ----------
                a : float
                  The evaluation point of the minuend
                b : float
                  The evaluation point of the subtrahend
        
                Returns
                -------
                tuple(float, float)
                    in the format (result, ppa), where ppa is the result of p(a)
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial has the wrong dimension
        
                Notes
                -----
                The call requires about 4n FLOPs where n is the number of coefficients in
                self, which is the same as that required for two separate polynomial
                evaluations. cpl_polynomial_eval_1d_diff() is however more accurate.
        
                The underlying algorithm is the same as that used in eval_1d()
                when the derivative is also requested.
        """
    def eval_2d(self, x: typing.SupportsFloat | typing.SupportsIndex, y: typing.SupportsFloat | typing.SupportsIndex) -> tuple[float, Vector]:
        """
                Evaluate a bivariate (2D) polynomial using Horner's rule and compute the derivatives.
        
                A polynomial with no non-zero coefficents evaluates to 0 with a
                derivative that does likewise.
        
                Parameters
                ----------
                x : float
                  x component of the evaluation point
                y : float
                  y component of the evaluation point
        
                Returns
                -------
                tuple(float, tuple(float, float))
                    in the format (result, gradient), where gradient is a tuple of 2 floats containing
                    the X and Y components of the gradient vector at the evaluated point
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial is not 2D
        
                Notes
                -----
                The result is computed as :math:`p_0 + x * ( p_1 + x * ( p_2 + ... x * p_n ))`
        
                The derivative is computed using a nested Horner rule.
        """
    def eval_3d(self, x: typing.SupportsFloat | typing.SupportsIndex, y: typing.SupportsFloat | typing.SupportsIndex, z: typing.SupportsFloat | typing.SupportsIndex) -> tuple[float, Vector]:
        """
                Evaluate a trivariate (3D) polynomial using Horner's rule and compute
                the derivatives.
        
                A polynomial with no non-zero coefficents evaluates to 0 with a
                derivative that does likewise.
        
                Parameters
                ----------
                x : float
                  x component of the evaluation point
                y : float
                  y component of the evaluation point
                z : float
                  z component of the evaluation point
        
                Returns
                -------
                tuple(float, tuple(float, float, float))
                    in the format (result, gradient), where gradient is a tuple of 3 floats containing
                    the X, Y and Z components of the gradient vector at the evaluated point
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial is not 3D
        
                Notes
                -----
                The result is computed as :math:`p_0 + x * ( p_1 + x * ( p_2 + ... x  * p_n ))`
        
                If the derivative is requested it is computed using a nested Horner rule.
        """
    def extract(self, dim: typing.SupportsInt | typing.SupportsIndex, other: Polynomial) -> Polynomial:
        """
                Collapse one dimension of a multi-variate polynomial by composition
        
                The dimension of the polynomial `self` must be one greater than that of the
                other polynomial. Given these two polynomials the dimension `dim` of `self` is
                collapsed by creating a new polynomial from::
        
                    self(x0, x1, ..., x{dim-1}, other(x0, x1, ..., x{dim-1}, x{dim+1}, x{dim+2}, ..., x{n-1}), x{dim+1}, x{dim+2}, ..., x{n-1}).
        
                The created polynomial thus has a dimension which is one less than the
                polynomial self and which is equal to that of the other polynomial.
                Collapsing one dimension of a 1D-polynomial is equivalent to evaluating it,
                which can be done with eval_1d().
        
                `other` polynomial must currently have a degree of zero, i.e. it must
                be a constant.
        
                Parameters
                ----------
                dim : int
                    The dimension to collapse (zero for first dimension)
                other : cpl.core.Polynomial
                    The polynomial to replace dimension dim of `self`
        
                Returns
                -------
                cpl.core.Polynomial
                    The collapsed polynomial or NULL on error
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial is uni-variate.
                cpl.core.IllegalInputError
                    if dim is negative.
                cpl.core.AccessOutOfRangeError
                    if dim exceeds the dimension of self.
                cpl.core.IncompatibleInputError
                    if other has the wrong dimension.
        
                Notes
                -----
                `other` only allowed to be a constant is to be fixed by CPL
        
                The collapse uses Horner's rule and requires for n coefficients requires
                about 2n FLOPs.
        """
    def fill_polynomial(self, p: typing.SupportsInt | typing.SupportsIndex, x0: typing.SupportsFloat | typing.SupportsIndex, d: typing.SupportsFloat | typing.SupportsIndex) -> Vector:
        """
                Evaluate a 1D-polynomial on equidistant points using Horner's rule
        
                The evaluation points are x_i = x0 + i * d, i=0, 1, ..., n-1,
                where n is the length of the vector.
        
                Parameters
                ----------
                out_size : int
                    How many points to evaluate (size of returned vector)
                x0 : float
                    The first point of evaluation
                d : float
                    The increment between points of evaluation
        
                Returns
                -------
                cpl.core.Vector
                    The evaluated points
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    If the polynomial `self` is not 1D
        
                Notes
                -----
                The call requires about 2nm FLOPs, where m+1 is the number of coefficients in
                p.
        """
    def fit(self, samppos: Matrix, fitvals: Vector, dimdeg: bool, maxdeg: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], sampsym: collections.abc.Sequence[bool] | None = None, fitsigm: cpl.core.Vector | None = None, mindeg: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex] | None = None) -> None:
        """
                Fit a polynomial to a set of samples in a least squares sense
        
                Any pre-set non-zero coefficients in self are overwritten or reset by the fit.
        
                For 1D-polynomials N = 1 + maxdeg - mindeg coefficients are fitted. A non-zero
                mindeg ensures that the fitted polynomial has a fix-point at zero.
        
                The number of distinct samples should exceed the number of coefficients to
                fit. The number of distinct samples may also equal the number of coefficients
                to fit, but in this case the fit has another meaning (any non-zero residual
                is due to rounding errors, not a fitting error).
                It is an error to try to fit more coefficients than there are distinct
                samples.
        
                In 1D the sampling points as pairs average u_0 (with an odd
                number of samples one sample must equal u_0).
        
                In 2D the sampling points are symmetric in the 2D-plane.
                For the first dimension sampling symmetry means that the sampling is line-
                symmetric around y = u_1, while for the second dimension, sampling symmetry
                implies line-symmetry around x = u_2. Point symmetry around
                (x,y) = (u_1, u_2) means that both sampsym[0] and sampsym[1] may be set to
                true. For Chebyshev nodes sampsym can be set to True.
        
                Parameters
                ----------
                samppos : cpl.core.Matrix
                    Matrix of p sample positions, with d rows and p columns
                fitvals : cpl.core.Vector
                    Vector of the p values to fit
                dimdeg : cpl_boolean
                    True iff there is a fitting degree per dimension. 
                    If dimdeg is false, an n-degree coefficient is fitted iff
                    mindeg <= n <= maxdeg.
                    If dimdeg is true, nci = 1 + maxdeg[i] - mindeg[i] 
                    coefficients are fitted for dimension i
                maxdeg : list of ints
                    Pointer to 1 or d maximum fitting degree(s), at least mindeg
                sampsym : list of bools, optional
                    d booleans, true iff the sampling is symmetric.
                    sampsym is ignored if mindeg is nonzero, otherwise the caller 
                    may use sampsym to indicate an a priori knowledge that the sampling positions are symmetric
                fitsigm : cpl.core.Vector, optional
                    Uncertainties of the sampled values, or None for all ones.
                    If relative uncertainties of the sampled values are known, they may be
                    passed via fitsigm. Not passing means that all uncertaintiesequals one. 
                mindeg : cpl.core.Size, optional
                    Pointer to 1 or d minimum fitting degree(s)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if a mindeg value is negative, or if a maxdeg value is less than the corresponding value
                cpl.core.DataNotFoundError
                    if the number of columns in samppos is less than the number of coefficients to be determined
                cpl.core.IncompatibleInputError
                    if samppos, fitvals or fitsigm have incompatible sizes
                cpl.core.SingularMatrixError
                    if samppos contains too few distinct values
                cpl.core.DivisionByZeroError
                    if an element in fitsigm is zero
                cpl.core.UnsupportedModeError
                    if the polynomial dimension exceeds two
        
                Examples
                ------
                fit1d = cpl.core.Polynomial(1)
                samppos1d = my_sampling_points_1d() # 1-row matrix
                fitvals   = my_sampling_values()
                sampsym = [True]
                maxdeg1d = 4 # Fit 5 coefficients
                fit1d.fit(sappos1d,fitsvals, False, [maxdeg1d], sampsym=sampsym)
        
                fit2d = cpl.core.Polynomial(2)
                samppos2d = my_sampling_points_2d() # 2-row matrix
                fitvals   = my_sampling_values()
                maxdeg2d = [2, 1] # Fit 6 coefficients
                fit2d.fit(sappos2d,fitsvals, False, [maxdeg2d])
        
                fit3d = cpl.core.Polynomial(3)
                samppos3d = my_sampling_points_3d() # 2-row matrix
                fitvals   = my_sampling_values()
                maxdeg3d = [2, 1, 2] # Fit 6 coefficients
                fit3d.fit(sappos3d,fitsvals, False, [maxdeg3d])
        
                Notes
                -----
                Currently only 1D (uni-variate), 2D (bi-variate) and 3D (tri-variate)
                polynomials are supported. For all but uni-variate polynomials mindeg must be zero.
                
                For a univariate (1D) fit the call requires 6MN + N^3/3 + 7/2N^2 + O(M) FLOPs
                where M is the number of data points and where N is the number of polynomial
                coefficients to fit, N = 1 + maxdeg - mindeg.
        
                For a bivariate fit the call requires MN^2 + N^3/3 + O(MN) FLOPs where M
                is the number of data points and where N is the number of polynomial
                coefficients to fit.
        
                The fit is done in the following steps:
                1) If fitsigm is not None. The factors are applied to the values.
                2) If mindeg is zero, the sampling positions are first transformed into
                Xhat_i = X_i - mean(X_i), i=1, .., dimension.
                3) The Vandermonde matrix is formed from Xhat. If fitsigm is not None,
                the weights are also taken into account.
                4) The normal equations of the Vandermonde matrix is solved.
                5) If mindeg is zero, the resulting polynomial in Xhat is transformed
                back to X.
                Warning: An increase in the polynomial degree will normally reduce the
                fitting error. However, due to rounding errors and the limited accuracy
                of the solver of the normal equations, an increase in the polynomial degree
                may at some point cause the fitting error to _increase_. In some cases this
                happens with an increase of the polynomial degree from 8 to 9.
        """
    def fit_residual(self, fitvals: Vector, samppos: Matrix, fitsigm: Vector = None) -> tuple[Vector, float]:
        """
                Compute the residual of a polynomial fit using `self` as the fitted polynomial
        
                If the relative uncertainties of the sampled values are known, they may be
                passed via fitsigm. Passing None means that all uncertainties equal one. The
                uncertainties are taken into account when computing the reduced
                chi square value.
        
                Parameters
                ----------
                fitvals : cpl.core.Vector
                    Vector of the p fitted values
                samppos : cpl.core.Matrix
                    Matrix of p sample positions, with d rows and p columns
        
                fitsigm : cpl.core.Vector, optional
                    Uncertainties of the sampled values or passed None for a uniform
                    uncertainty
                Returns
                -------
                tuple(cpl.core.Vector, double)
                    tuple in the format (result, rechisq) where:
                    - result is the vector of the fitting residuals, same size as fitvals
                    - redchisq is the reduced chi square of the fit
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if samppos, fitvals, fitsigm have incompatible sizes
                cpl.core.DivisionByZeroError
                    if an element in fitsigm is zero
                cpl.core.DataNotFoundError
                    if the number of columns in samppos is less than than the number of coefficients in the fitted polynomial.
        """
    def get_coeff(self, pows: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> float:
        """
                Get a coefficient of the polynomial. 
        
                Requesting the value of a coefficient that has not been set is allowed, in this case zero is returned.
        
                Parameters
                ----------
                pows : list of ints
                    The non-negative power(s) of the variable(s)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if pows contains negative values
        
                Notes
                -----
                For an N-dimensional polynomial the complexity is O(N) 
        
                Examples
                -------
                coeff = poly1d.get_coeff([3])
                    Requesting the value of a coefficient that has not been set is allowed, in this case zero is returned.
        """
    def multiply(self, other: Polynomial) -> None:
        """
                Multiply with a polynomial with the same dimension as `self`
        
                Parameters
                ----------
                other : cpl.core.Polynomial
                    polynomial to multiply with
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `self` and `other` do not have identical dimensions
        """
    def multiply_scalar(self, factor: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Multiply polynomial `self` with a scalar
        
                Parameters
                ----------
                factor : float
                    factor to multiply with
        
                Notes
                -----
                If factor is zero, all coefficients are reset, if it is 1 all are copied
        """
    def set_coeff(self, pows: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Set a coefficient of the polynomial
                
                `pows` is assumed to have the size of the polynomial dimension.
        
                If the coefficient is already there, it is overwritten, if not, a new coefficient is added to the polynomial. This may cause the degree of the polynomial to be increased, or if the new coefficient is zero, to decrease.
        
                Parameters
                ----------
                pows : list of ints
                    The non-negative power(s) of the variable(s)
                value : float
                    the coefficient
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if pows contains negative values
        
                Notes
                -----
                For an N-dimensional polynomial the complexity is O(N)
        """
    def shift_1d(self, i: typing.SupportsInt | typing.SupportsIndex, u: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Modify p, p(x0, x1, ..., xi, ...) := (x0, x1, ..., xi+u, ...)
        
                Shifting the polynomial p(x) = x^n with u = 1 will generate the binomial
                coefficients for n.
        
                Shifting the coordinate system to (x,y) for the 2D-polynomium poly2d::
        
                    poly2d.shift_1d(0,x)
                    poly2d.shift_1d(1,y)
        
                Parameters
                ----------
                i : int
                    The dimension to shift (0 for first)
                u : float
                    The shift
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if i is negative
                cpl.core.AccessOutOfRangeError if i exceeds the dimension of p
        """
    def solve_1d(self, x0: typing.SupportsFloat | typing.SupportsIndex, mul: typing.SupportsInt | typing.SupportsIndex) -> float:
        """
                A real solution to p(x) = 0 using Newton-Raphsons method
        
                Even if a real solution exists, it may not be found if the first guess is
                too far from the solution. But a solution is guaranteed to be found if all
                roots of p are real (except in the case where the derivative at the first
                guess happens to be zero, see below - for an n-degree polynomial there are up
                to n-1 such guesses). If the constant term is zero, the solution 0 will be
                returned regardless of the first guess.
        
                Parameters
                ----------
                x0 : float
                    First guess of the solution
                mul : int
                    The root multiplicity (or 1 if unknown)
        
                Returns
                -------
                float
                    The solution
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    if the polynomial has the wrong dimension
                cpl.core.IllegalInputError
                    if the multiplicity is non-positive
                cpl.core.ContinueError
                    If the algorithm does not converge
                cpl.core.DivisionByZeroError
                    if a division by zero occurs
        
                Notes
                ------
                No solution is found when the iterative process stops because:
                1) It can not proceed because p`(x) = 0 (cpl.core.DivisionByZeroError).
                2) Only a finite number of iterations are allowed (cpl.core.ContinueError).
                Either can happen due to an an actual lack of a real solution or due to an
                insufficiently good first guess.
        
                The accuracy and robustness deteriorates with increasing multiplicity
                of the solution. This is also the case with numerical multiplicity,
                i.e. when multiple solutions are located close together.
        
                `mul` is assumed to be the multiplicity of the solution. Knowledge of the
                root multiplicity often improves the robustness and accuracy. If there
                is no knowledge of the root multiplicity mul should be 1.
                Setting mul to a too high value should be avoided.
        """
    def subtract(self, other: Polynomial) -> None:
        """
                Subtract a polynomial with the same dimension as `self`
        
                Parameters
                ----------
                other : cpl.core.Polynomial
                    polynomial to subtract
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `self` and `other` do not have identical dimensions
        """
    @property
    def degree(self) -> int:
        """
        The degree is the highest sum of exponents (with a non-zero coefficient). If there are no non-zero coefficients the degree is zero.
        """
    @property
    def dimension(self) -> int:
        """
        The dimension of the polynomial.
        """
class Property:
    """
    
        Properties are basically a variable container which consists of a name, a type identifier and a specific value of that type. 
        
        The type identifier always determines the type of the associated value. A property is similar to an ordinary variable and its 
        current value can be set or retrieved through its name. In addition a property may have a descriptive comment associated.
        
        The following types are supported:
            - cpl.core.Type.BOOL
            - cpl.core.Type.FLOAT
            - cpl.core.Type.INT
            - cpl.core.Type.CHAR
            - cpl.core.Type.STRING
            - cpl.core.Type.DOUBLE
            - cpl.core.Type.LONG
            - cpl.core.Type.LONG_LONG
            - cpl.core.Type.FLOAT_COMPLEX
            - cpl.core.Type.DOUBLE_COMPLEX
        
        Support for arrays in general is currently not available.
        
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getstate__(self) -> tuple:
        ...
    @typing.overload
    def __init__(self, name: str, type: Type) -> None:
        """
        Initialise a property without setting value
        """
    @typing.overload
    def __init__(self, name: str, type: Type, initial_value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | str | typing.SupportsFloat | typing.SupportsIndex, comment: str | None = None) -> None:
        """
                    Create a new property, manually provide type. Comment is optional. The following types are supported:
                    - cpl.core.Type.BOOL
                    - cpl.core.Type.FLOAT
                    - cpl.core.Type.INT
                    - cpl.core.Type.CHAR
                    - cpl.core.Type.STRING
                    - cpl.core.Type.DOUBLE
                    - cpl.core.Type.LONG
                    - cpl.core.Type.LONG_LONG
                    - cpl.core.Type.FLOAT_COMPLEX
                    - cpl.core.Type.DOUBLE_COMPLEX
        """
    @typing.overload
    def __init__(self, name: str, initial_value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | str | typing.SupportsFloat | typing.SupportsIndex, comment: str | None = None) -> None:
        """
                    Create a new property with automatic type inference. Comment is optional. The following types are supported:
        
                    If the value provided fails to be inferred to one of the supported types, an InvalidTypeError is thrown. 
        """
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def __str__(self) -> str:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a property contents to a file, stdout or a string.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump property contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send property contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the property contents.
        """
    @property
    def __len__(self) -> int:
        ...
    @property
    def comment(self) -> str | None:
        """
        property description
        """
    @comment.setter
    def comment(self, arg1: str) -> None:
        ...
    @property
    def name(self) -> str:
        """
        name of property
        """
    @name.setter
    def name(self, arg1: str) -> None:
        ...
    @property
    def type(self) -> Type:
        """
        CPL type of property. See 
        """
    @property
    def value(self) -> complex | float | bool | str | int | complex | int | int | str | float:
        ...
    @value.setter
    def value(self, arg1: typing.Any) -> None:
        ...
class PropertyList:
    """
    
        The opaque property list data type. 
    
        Was designed for supporting the FITS header information. Indeed, it is possible, using a
        single function, to load a header file into a property list, given the filename and the 
        number of the extension using the `load()` function. 
        
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def load(name: str, position: typing.SupportsInt | typing.SupportsIndex) -> PropertyList:
        """
                    Create a filtered property list from a file.
                    
                    Parameters
                    ----------
                    name : str 
                        Name of the input file.
                    position : int 
                        Index of the data set to read.
        
                    Returns
                    -------
                    cpl.core.PropertyList
                        The loaded propertylist from the input file at index `position`
        
                    Notes
                    -----
                    The function reads the properties of the data set with index position from the file name.
        
                    Currently only the FITS file format is supported. The property list is created by reading 
                    the FITS keywords from extension position. The numbering of the data sections starts from 
                    0. When creating the property list from a FITS header, any keyword without a value such 
                    as undefined keywords, are not transformed into a property. In the case of float or double 
                    (complex) keywords, there is no way to identify the type returned by CFITSIO, therefore 
                    this function will always load them as double (complex).
        """
    @staticmethod
    def load_regexp(name: str, position: typing.SupportsInt | typing.SupportsIndex, regexp: str, invert: bool) -> PropertyList:
        """
                    Create a filtered property list from a file.
                    
                    Parameters
                    ----------
                    name : str 
                        Name of the input file.
                    position : int 
                        Index of the data set to read.
                    regexp : str
                        Regular expression used to filter properties.
                    invert : bool
                        Flag inverting the sense of matching property names.
        
                    Returns
                    -------
                    cpl.core.PropertyList
                        The loaded propertylist from the input file at index `position`, with properties matching the `regexp` filter
        
                    Notes
                    -----
                    The function reads all properties of the data set with index `position`
                    with matching names from the file `name`. If the flag `invert` is False,
                    all properties whose names match the regular expression `regexp` are
                    read. If `invert` is set to True, all properties with
                    names not matching `regexp` are read rather. The function expects
                    POSIX 1003.2 compliant extended regular expressions.
                    
                    Currently only the FITS file format is supported. The property list is
                    created by reading the FITS keywords from extension `position`.
                    
                    The numbering of the data sections starts from 0.
                    
                    When creating the property list from a FITS header, any keyword without
                    a value such as undefined keywords, are not transformed into
                    a property. In the case of float or double (complex) keywords, there is no
                    way to identify the type returned by CFITSIO, therefore this function will
                    always load them as double (complex).
                    
                    FITS format specific keyword prefixes (e.g. ``HIERARCH``) must
                    not be part of the given pattern string `regexp`, but only the actual
                    FITS keyword name may be given.
        """
    @typing.overload
    def __contains__(self, arg0: str) -> bool:
        ...
    @typing.overload
    def __contains__(self, arg0: Property) -> bool:
        ...
    @typing.overload
    def __delitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Remove a Property from the PropertyList from position `index`
        """
    @typing.overload
    def __delitem__(self, name: str) -> None:
        """
        Remove a Property from the PropertyList with a specific name
        """
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> Property:
        """
        Retrieve property from list using it's index position
        """
    @typing.overload
    def __getitem__(self, prop_name: str) -> Property:
        """
        Retrieve property from list using it's name
        """
    @typing.overload
    def __getitem__(self, arg0: slice) -> PropertyList:
        ...
    def __getstate__(self) -> list[Property]:
        ...
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty property list
        """
    @typing.overload
    def __init__(self, from_: collections.abc.Iterable) -> None:
        """
        Build a PropertyList from an iterable of properties. If iterable contains any non-properties a cast error will be thrown
        """
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    @typing.overload
    def __setitem__(self, index: typing.SupportsInt | typing.SupportsIndex, property: Property) -> None:
        """
        Insert a property at the given index position
        """
    @typing.overload
    def __setitem__(self, arg0: str, arg1: Property) -> None:
        ...
    @typing.overload
    def __setitem__(self, slice: slice, properties: collections.abc.Sequence[Property]) -> None:
        """
        Insert an iterable of Properties into `this`, within the slice range. Slice must be the same size as the iterable
        """
    def __setstate__(self, arg0: collections.abc.Sequence[Property]) -> None:
        ...
    def __str__(self) -> str:
        ...
    @typing.overload
    def append(self, property: Property) -> None:
        """
                  Append a property value to a property list.
        
                  This function creates a new property and appends it to the end of a property list. It will not check if the property already exists.
        
                  :Parameters:
                    **property** (*cpl.core.Property*) -- Property to append
        """
    @typing.overload
    def append(self, other: PropertyList) -> None:
        """
                  Append a propertylist
        
                  This function appends the properties from the property list `other` to `self`.
        
                  :Parameters:
                    **other** (*cpl.core.PropertyList*) -- Propertylist to append
        """
    @typing.overload
    def append(self, name: str, value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex | typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | str | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                  Append a new property using a name and value
        
                  This function appends a new property with `name` and inital value `value`. The type will be infered by `value`' s type
        
                  :Parameters:
                    - **name** (*str*) -- Name for the new property
                    - **value** (*str, char, float, complex, bool, int*) -- Initial value of the new property
        """
    def del_regexp(self, regexp: str, invert: bool) -> int:
        """
                    Erase all properties with name matching a given regular expression.
        
                    The function searches for all the properties matching in the list.
        
                    The function expects POSIX 1003.2 compliant extended regular expressions.
        
                    Parameters
                    ----------
                    regexp : str
                        Regular expression.
                    invert : bool
                        Flag inverting the sense of matching.
        
                    Returns
                    -------
                    int
                        The number of erased entries
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a property list contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump property list contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send property list contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the property list contents.
        """
    @typing.overload
    def insert(self, index: typing.SupportsInt | typing.SupportsIndex, property: Property) -> None:
        """
        Insert a property at index. PropertyList will increase in size by 1.
        """
    @typing.overload
    def insert(self, arg0: str, arg1: Property) -> None:
        ...
    def save(self, name: str, mode: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                    Save a property list to a FITS file. 
                    
                    Parameters
                    ----------
                    name : str 
                        Name of the input file.
                    position : int 
                        Index of the data set to read.
                    mode : unsigned
                        The desired output options (combined with bitwise or of cpl.core.io enums)
        
                    Notes
                    -----
                    This function saves a property list to a FITS file, using cfitsio. The data unit is empty.
        
                    Supported output modes are cpl.core.io.CREATE (create a new file) and cpl.core.io.EXTEND 
                    (append to an existing file) 
        """
    def setup_product_header(self, product_frame: cpl.ui.Frame, framelist: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, recid: str, pipeline_id: str, dictionary_id: str, inherit_frame: typing.Any = None) -> None:
        """
            Add product keywords to a pipeline product property list.
        
            Parameters
            ----------
            product_frame : cpl.ui.Frame
              Frame describing the product
            framelist : cpl.ui.FrameSet
              List of frames including all input frames
            parlist : cpl.ui.ParameterList
              Recipe parameter list
            recid : str
              Recipe name
            pipeline_id : str
              Pipeline package (unique) identifier
            dictionary_id : str
              PRO dictionary identifier
            inherit_frame : cpl.ui.Frame, optional
              Frame from which header information is inherited
        
            Returns
            -------
            None
        
            Raises
            ------
            cpl.core.DataNotFoundError
              If the input framelist contains no input frames or
              a frame in the input framelist does not specify a file.
              In the former case the string "Empty set-of-frames" is appended
              to the error message.
            cpl.core.IllegalInputError
              If the product frame is not tagged or not grouped
              as cpl.ui.Frame.FrameGroup.PRODUCT. A specified `inherit_frame`
              doesn't belong to the input frame list, or it is not in FITS format.
            cpl.core.FileNotFoundError
              If a frame in the input framelist specifies a non-existing file.
            cpl.core.BadFileFormatError
              If a frame in the input framelist specifies an invalid file.
        
            Notes
            -----
            This function updates and validates that the property list `self` is DICB
            compliant. In particular, this function does the following:
        
            1. Selects a reference frame from which the primary and secondary
               keyword information is inherited. The primary information is
               contained in the FITS keywords ``ORIGIN``, ``TELESCOPE``, ``INSTRUME``,
               ``OBJECT``, ``RA``, ``DEC``, ``EPOCH``, ``EQUINOX``, ``RADESYS``,
               ``DATE-OBS``, ``MJD-OBS``, ``UTC``, ``LST``, ``PI-COI``, ``OBSERVER``,
               while the secondary information is contained in all the other keywords.
               If the `inherit_frame` is None, both primary and secondary information
               is inherited from the first frame in the input framelist with
               group cpl.ui.Frame.FrameGroup.RAW, or if no such frames are present
               the first frame with group cpl.ui.Frame.FrameGroup.CALIB.
               If `inherit_frame` is not None, the secondary information
               is inherited from `inherit_frame` instead.
        
            2. Copy to `self`, if they are present, the following primary
               FITS keywords from the first input frame in the `framelist`:
               ``ORIGIN``, ``TELESCOPE``, ``INSTRUME``, ``OBJECT``, ``RA``,
               ``DEC``, ``EPOCH``, ``EQUINOX``, ``RADESYS``, ``DATE-OBS``,
               ``MJD-OBS``, ``UTC``, ``LST``, ``PI-COI``, ``OBSERVER``. If those
               keywords are already present in the `self` property list, they
               are overwritten only in case they have the same type. If any of
               these keywords are present with an unexpected type, a warning is
               issued, but the keywords are copied anyway (provided that the
               above conditions are fulfilled), and no error is set.
        
            3. Copy all the ``HIERARCH ESO *`` keywords from the primary FITS header
               of the `inherit_frame` in `framelist`, with the exception of
               the ``HIERARCH ESO DPR *``, and of the ``HIERARCH ESO PRO *`` and
               ``HIERARCH ESO DRS *`` keywords if the `inherit_frame` is a calibration.
               If those keywords are already present in `self`, they are overwritten.
        
            4. If found, remove the ``HIERARCH ESO DPR *`` keywords from `self`.
        
            5. If found, remove the ``ARCFILE`` and ``ORIGFILE`` keywords from `self`.
        
            6. Add to `self` the following mandatory keywords from the PRO
               dictionary: ``PIPEFILE``, ``ESO PRO DID``, ``ESO PRO REC1 ID``,
               ``ESO PRO REC1 DRS ID``, ``ESO PRO REC1 PIPE ID``, and
               ``ESO PRO CATG``. If those keywords are already present in
               `self`, they are overwritten. The keyword ``ESO PRO CATG`` is
               always set identical to the tag in `product_frame`.
        
            7. Only if missing, add to `self` the following mandatory keywords
               from the PRO dictionary: ``ESO PRO TYPE``, ``ESO PRO TECH``, and
               ``ESO PRO SCIENCE``. The keyword ``ESO PRO TYPE`` will be set to
               ``REDUCED``. If the keyword ``ESO DPR TECH`` is found in the header
               of the first frame, ``ESO PRO TECH`` is given its value, alternatively
               if the keyword ``ESO PRO TECH`` is found it is copied instead, and
               if all fails the value ``UNDEFINED`` is set. Finally, if the keyword
               ``ESO DPR CATG`` is found in the header of the first frame and is set
               to ``SCIENCE``, the boolean keyword ``ESO PRO SCIENCE`` will be set to
               `true`, otherwise it will be copied from an existing ``ESO PRO SCIENCE``
               keyword, while it will be set to `false` in all other cases.
        
            8. Check the existence of the keyword ``ESO PRO DATANCOM`` in `self`. If
               this keyword is missing, one is added, with the value of the total
               number of raw input frames.
        
            9. Add to `self` the keywords ``ESO PRO REC1 RAW1 NAME``,
               ``ESO PRO REC1 RAW1 CATG``, ``ESO PRO REC1 CAL1 NAME``, ``ESO PRO REC1 CAL1 CATG``,
               to describe the content of the input set-of-frames.
        
            See the DICB PRO dictionary for details on the mentioned PRO keywords.
        
            Non-FITS files are handled as files with an empty FITS header.
            
            The pipeline identifier string `pipe_id` is composed of the pipeline package
            name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
        """
    def sort(self, compare: collections.abc.Callable) -> None:
        """
                    Sort a property list using a passed function.
        
                    Sort is done in place
        
                    Parameters
                    ----------
                    compare : function(cpl.core.Property, cpl.core.Property) -> int
                        The function used to compare two properties.  This function compares to determine whether a property is less, 
                        equal or greater than another one.
        
                    Returns
                    -------
                    None
        """
class SingularMatrixError(Error, RuntimeError):
    """
    Could not invert a matrix.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 17
class Sort:
    """
    Members:
    
      DESCENDING : For use with cpl.core.Vector.sort() for descending order sort
    
      ASCENDING : For use with cpl.core.Vector.sort() for ascending order sort
    """
    ASCENDING: typing.ClassVar[Sort]  # value = <Sort.ASCENDING: 1>
    DESCENDING: typing.ClassVar[Sort]  # value = <Sort.DESCENDING: -1>
    __members__: typing.ClassVar[dict[str, Sort]]  # value = {'DESCENDING': <Sort.DESCENDING: -1>, 'ASCENDING': <Sort.ASCENDING: 1>}
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
class SortMode:
    """
    Members:
    
      BY_X
    
      BY_Y
    """
    BY_X: typing.ClassVar[SortMode]  # value = <SortMode.BY_X: 0>
    BY_Y: typing.ClassVar[SortMode]  # value = <SortMode.BY_Y: 1>
    __members__: typing.ClassVar[dict[str, SortMode]]  # value = {'BY_X': <SortMode.BY_X: 0>, 'BY_Y': <SortMode.BY_Y: 1>}
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
class Table:
    """
    
            This module provides functions to create and user PyCPL tables. 
    
            A CPL table is made of columns, and a column consists of an array of elements of a given 
            type. 
            
            The following types are supported, 
            - cpl.core.Type.INT 
            - cpl.core.Type.LONG_LONG 
            - cpl.core.Type.FLOAT 
            - cpl.core.Type.DOUBLE
            - cpl.core.Type.DOUBLE_COMPLEX
            - cpl.core.Type.FLOAT_COMPLEX
            - cpl.core.Type.STRING. 
            
            Moreover, it is possible to define columns of arrays, i.e. columns whose elements are arrays 
            of all the basic types listed above. Within the same column all arrays must have the same 
            type and the same length.
            
            A table column is accessed by specifying its name. The ordering of the columns within a table 
            is undefined: a CPL table is not an n-tuple of columns, but just a set of columns. The N elements 
            of a column are counted from 0 to N-1, with element 0 on top. The set of all the table columns 
            elements with the same index constitutes a table row, and table rows are counted according to the 
            same convention. 
            
            It is possible to flag each table row as "selected" or "unselected", and each column element as 
            "valid" or "invalid". Selecting table rows is mainly a way to extract just those table parts 
            fulfilling any given condition, 
            while invalidating column elements is a way to exclude such elements from any computation. A CPL table 
            is created with all rows selected, and a column is created with all elements invalidated.
    
            New columns can be allocated either by calling the appropriate function (:py:meth:`new_column` for
            regular columns, :py:meth:`new_column_array` for array columns) or setting directly via index
            (however the given array must be of the number of table rows). 
            See __setitem__ docs for more info. 
    
            Array column elements must all be of the same length.
    
            New CPL tables can be built from an existing table-like object, or via the `empty` static method.
            See the Parameters section for building from existing data
    
            Parameters
            ----------
            input : object
                Data used to build the new CPL table object. This data source must only contain objects of
                types compatible with CPL tables
    
                `input` can be the following types:
                - astropy.table.QTable
                - pandas.Dataframe
                - numpy.recarray
    
            Raises
            ------
            cpl.core.InvalidTypeError
                If one of the columns in `input` is not a CPL compatible type
    
            
    """
    class Operator:
        """
        Members:
        
          NOT_EQUAL_TO
        
          EQUAL_TO
        
          GREATER_THAN
        
          NOT_GREATER_THAN
        
          LESS_THAN
        
          NOT_LESS_THAN
        """
        EQUAL_TO: typing.ClassVar[Table.Operator]  # value = <Operator.EQUAL_TO: 0>
        GREATER_THAN: typing.ClassVar[Table.Operator]  # value = <Operator.GREATER_THAN: 2>
        LESS_THAN: typing.ClassVar[Table.Operator]  # value = <Operator.LESS_THAN: 4>
        NOT_EQUAL_TO: typing.ClassVar[Table.Operator]  # value = <Operator.NOT_EQUAL_TO: 1>
        NOT_GREATER_THAN: typing.ClassVar[Table.Operator]  # value = <Operator.NOT_GREATER_THAN: 3>
        NOT_LESS_THAN: typing.ClassVar[Table.Operator]  # value = <Operator.NOT_LESS_THAN: 5>
        __members__: typing.ClassVar[dict[str, Table.Operator]]  # value = {'NOT_EQUAL_TO': <Operator.NOT_EQUAL_TO: 1>, 'EQUAL_TO': <Operator.EQUAL_TO: 0>, 'GREATER_THAN': <Operator.GREATER_THAN: 2>, 'NOT_GREATER_THAN': <Operator.NOT_GREATER_THAN: 3>, 'LESS_THAN': <Operator.LESS_THAN: 4>, 'NOT_LESS_THAN': <Operator.NOT_LESS_THAN: 5>}
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
    def empty(rows: typing.SupportsInt | typing.SupportsIndex) -> Table:
        """
              Construct empty table with a set number of rows
        
              Parameters
              ----------
              rows : int
                  number of rows in the new table
        
              Returns
              -------
              cpl.core.Table
                  New empty Table with `rows` number of rows
        
              Raises
              ------
              cpl.core.IllegalInputError
                  `rows` is negative
        """
    @staticmethod
    def load(filename: str, xtnum: typing.SupportsInt | typing.SupportsIndex, check_nulls: bool = True) -> Table:
        """
                Load a FITS table extension to generate a new cpl.core.Table
        
                The selected FITS file table extension is just read and converted into the cpl.core.Table object.
        
                Parameters
                ----------
                filename : str
                    Name of FITS file with at least one table extension.
                xtnum : int
                    Number of extension to read, starting from 1.
                check_nulls : bool, optional
                    If set to False, identified invalid values are not marked.
        
                Returns
                -------
                cpl.core.Table
                    New cpl.core.Table from loaded data.
        
                Raises
                ------
                cpl.core.FileNotFoundError
                    A file named as specified in `filename` is not found.
                cpl.core.BadFileFormatError
                    The input file is not in FITS format.
                cpl.core.IllegalInputError
                    The specified FITS file extension is not a table, or, if it is a table, it has more than 9999 columns.
                cpl.core.AccessOutOfRangeError
                    `xtnum` is greater than the number of FITS extensions in the FITS file, or is less than 1.
                cpl.core.DataNotFoundError
                    The FITS table has no rows or no columns.
                cpl.core.UnspecifiedError
                    Generic error condition, that should be reported to the CPL Team.
        
                See Also
                --------
                cpl.core.Table.load_window : Load part of the FITS table extension
        """
    @staticmethod
    def load_window(filename: str, xtnum: typing.SupportsInt | typing.SupportsIndex, start: typing.SupportsInt | typing.SupportsIndex, nrow: typing.SupportsInt | typing.SupportsIndex, check_nulls: bool = True, cols: collections.abc.Sequence[str] = []) -> Table:
        """
                Load part of a FITS table extension to generate a new cpl.core.Table
        
                The selected FITS file table extension is just read and converted into the cpl.core.Table object.
        
                Parameters
                ----------
                filename : str
                    Name of FITS file with at least one table extension.
                xtnum : int
                    Number of extension to read, starting from 1.
                start : int
                    First table row to extract.
                nrow : int
                    Number of rows to extract.
                check_nulls : bool, optional
                    If set to False, identified invalid values are not marked.
                cols : list of str, optional
                    List of the names of the columns to extract. If not given all columns are selected.
        
                Returns
                -------
                cpl.core.Table
                    New cpl.core.Table from loaded data.
        
                Raises
                ------
                cpl.core.FileNotFoundError
                    A file named as specified in `filename` is not found.
                cpl.core.BadFileFormatError
                    The input file is not in FITS format.
                cpl.core.IllegalInputError
                    The specified FITS file extension is not a table, or, if it is a table, it has more than 9999 columns.
                cpl.core.AccessOutOfRangeError
                    `xtnum` is greater than the number of FITS extensions in the FITS file, or is less than 1. Or `start` is either less than zero, or greater than the number of rows in the table.
                cpl.core.DataNotFoundError
                    The FITS table has no rows or no columns.
                cpl.core.UnspecifiedError
                    Generic error condition, that should be reported to the CPL Team.
        
                See Also
                --------
                cpl.core.Table.load_window : Load the entire FITS table extension
        """
    def __contains__(self, arg0: str) -> int:
        ...
    def __deepcopy__(self, arg0: dict) -> Table:
        ...
    @typing.overload
    def __getitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex]) -> typing.Any:
        """
        Get value from location [column_name, row]
        """
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> dict:
        """
        Get table row
        """
    @typing.overload
    def __getitem__(self, arg0: str) -> typing.Any:
        """
        Get table column values
        """
    def __init__(self, input: typing.Any) -> None:
        ...
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    @typing.overload
    def __setitem__(self, location: tuple[str, slice], setting: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Fill column with double value in slice range. Location in format [column_name, start_index:stop_index]. Column type must be numerical.
                Example:
        
                t["double_col", 1:4]= 4.5
        
                Slice must be within Table range (0<=start_index<stop_index<= len(t))
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, slice], setting: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Fill column with long long value in slice range. Location in format [column_name, start_index:stop_index]. Column type must be numerical.
                Example:
        
                t["long_long_col", 1:4]= 4555
        
                Slice must be within Table range (0<=start_index<stop_index<= len(t))
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, slice], setting: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Fill column with double complex value in slice range. Location in format [column_name, start_index:stop_index]. Column must a complex type.
                Example:
        
                t["double_complex_col", 1:4]= 4.0+52.0j
        
                Slice must be within Table range (0<=start_index<stop_index<= len(t))
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, slice], setting: str) -> None:
        """
                Fill column with string value in slice range. Location in format [column_name, start_index:stop_index]. Column must be of type cpl.core.Type.STRING
                Example:
        
                t["string_col", 1:4]= "value"
        
                Slice must be within Table range (0<=start_index<stop_index<= len(t))
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, slice], setting: typing.Any) -> None:
        """
        Fill column with array value in slice range. Location in format [column_name, start_index:stop_index]. Must be an array column
                Example:
        
                t["array_col", 1:4]= [1,2,3]
        
                Slice must be within Table range (0<=start_index<stop_index<= len(t))
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex], setting: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
        Set double value at location [column_name,row]. Column must be of a numerical type.
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex], setting: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Set long long value at location [column_name,row]. Column must be of a numerical type.
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex], setting: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
        Set double complex value at location [column_name,row]. Column must be of a complex type.
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex], setting: str) -> None:
        """
        Set string value at location [column_name,row]. Column must be of cpl.core.Type.STRING.
        """
    @typing.overload
    def __setitem__(self, location: tuple[str, typing.SupportsInt | typing.SupportsIndex], setting: typing.Any) -> None:
        """
        Set array at location [column_name,row]. Column must be an array type.
        """
    @typing.overload
    def __setitem__(self, column_name: str, setting: typing.Any) -> None:
        """
                Set column values with numpy array compatible object. Must be of same size as the number of table rows, for example:
        
                .. code-block:: python
        
                  t = table(3)
                  t["new_column"]=[1,2,3]
        
                Will create new column "new_column" of inferred type long long with values 1,2,3. This also works with array columns e.g.
        
                .. code-block:: python
                
                  t = table(3)
                  t["new_column"]=[[1,2,3], [4,5,6], [7,8,9]]
        """
    def __str__(self) -> str:
        ...
    def abs(self, name: str) -> None:
        """
                Compute the absolute value of column values.
        
                Each column element is replaced by its absolute value.
                Invalid elements are not modified by this operation.
                If the column is complex, its type will be turned to
                real (cpl.core.Type.FLOAT_COMPLEX will be changed into cpl.core.Type.FLOAT,
                and cpl.core.Type.DOUBLE_COMPLEX will be changed into cpl.core.Type.DOUBLE).
        
                Parameters
                ----------
                name : str
                    Table column name.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def add_columns(self, to_name: str, from_name: str) -> None:
        """
                Add the values of two numeric or complex table columns.
        
                The columns are summed element by element, and the result of the sum is
                stored in the target column. The columns' types may differ, and in that
                case the operation would be performed using the standard C upcasting
                rules, with a final cast of the result to the target column type.
                Invalid elements are propagated consistently: if either or both members
                of the sum are invalid, the result will be invalid too. Underflows and
                overflows are ignored.
        
                Parameters
                ----------
                to_name : str
                    Name of target column.
                from_name : str
                    Name of source column.
        """
    def add_scalar(self, name: str, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Add a constant value to a numerical or complex column.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                are not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : float
                    Value to add.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def add_scalar_complex(self, name: str, value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Add a constant complex value to a numerical or complex column.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                are not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : complex
                    Value to add.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def and_selected(self, name1: str, operator: Table.Operator, name2: str) -> int:
        """
                Select from selected table rows, by comparing the values of two columns.
        
                Either both columns must be numerical, or they both must be strings.
                The comparison between strings is lexicographical. Neither can be a
                complex or array type.
        
                For all the already selected table rows, the values of the specified
                column are compared. The table rows not fulfilling the comparison
                are unselected. Invalid elements from either columns never fulfill
                any comparison by definition.
        
                For this function, the column is of a numerical type if its type is:
                * cpl.core.Type.INT
                * cpl.core.Type.FLOAT
                * cpl.core.Type.DOUBLE
                * cpl.core.Type.LONG_LONG
        
                All table rows not fulfilling the comparison are unselected. An invalid element never
                fulfills any comparison by definition.
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name1 : str
                    Name of the first table column
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                name2 : str
                    Name of second table column.
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with any of the specified names is not found in table.
                cpl.core.InvalidTypeError
                    Invalid types for comparison.
        
                See Also
                --------
                cpl.core.Table.or_selected : To select from unselected rows using column comparison
        """
    def and_selected_invalid(self, name: str) -> int:
        """
                Select from selected table rows all rows with an invalid value in a specified column.
        
                For all the already selected table rows, all the rows containing valid
                values at the specified column are unselected. See also the function
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    New number of selected rows
        """
    def and_selected_numerical(self, name: str, operator: Table.Operator, value: typing.Any) -> int:
        """
                Select from already selected table rows, by comparing a column of numercal values to the reference `value`
        
                For all the already selected table rows, the values of the specified
                column are compared with the reference value.
        
                The column is of a numerical type if its type is:
                * cpl.core.Type.INT
                * cpl.core.Type.FLOAT
                * cpl.core.Type.DOUBLE
                * cpl.core.Type.DOUBLE_COMPLEX
                * cpl.core.Type.FLOAT_COMPLEX
                * cpl.core.Type.LONG_LONG
        
                All table rows not fulfilling the comparison are unselected. An invalid element never
                fulfills any comparison by definition.
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name : str
                    Column name.
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                value : int, float or complex
                    Reference value
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if the column of the given `name` wasn't found.
                cpl.core.TypeMismatchError
                    Column of the given `name` is not numerical
        
                See Also
                --------
                cpl.core.Table.or_selected_numerical : To select from unselected rows using numerical operator comparison
        """
    def and_selected_string(self, name: str, operator: Table.Operator, string: str) -> int:
        """
                Select from selected table rows, by comparing a column of string values to the given `string`.
        
                For all the already selected table rows, the values of the specified
                column are compared with the reference string.
        
                If `operator` is equal to cpl.core.Operator.EQUAL_TO or cpl.core.Operator.NOT_EQUAL_TO
                then the comparison string is treated as a regular expression.
        
                All table rows not fulfilling the comparison are unselected. An invalid element never
                fulfills any comparison by definition.
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name : str
                    Column name.
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                string : str
                    Reference character string
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if the column of the given `name` wasn't found.
                cpl.core.TypeMismatchError
                    Column of the given `name` is not of type cpl.core.Type.STRING
                cpl.core.IllegalInputError
                    Invalid regular expression
        
                See Also
                --------
                cpl.core.Table.or_selected_string : To select from unselected rows using string comparison
        """
    def and_selected_window(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
                Select from selected rows only those within a table segment.
        
                All the selected table rows that are outside the specified interval are
                unselected. If the sum of `start` and `count` goes beyond the end
                of the input table, rows are checked up to the end of the table.
        
                Parameters
                ----------
                start : int
                    First row of table segment.
                count : int
                    Length of segment
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.AccessOutOfRange
                    `self` has zero length, or `start` is outside `self`'s boundaries
                cpl.core.IllegalInputError
                    `count` is negative
        
                See Also
                --------
                cpl.core.Table.or_selected_window : To select from unselected rows using a specified segment
        """
    def arg_column(self, name: str) -> None:
        """
                Compute the phase angle value of table column elements.
        
                Each column element is replaced by its phase angle value.
                The phase angle will be in the range of [-pi,pi].
                Invalid elements are not modified by this operation.
                If the column is complex, its type will be turned to
                real (cpl.core.Type.FLOAT_COMPLEX will be changed into cpl.core.Type.FLOAT,
                and cpl.core.Type.DOUBLE_COMPLEX will be changed into cpl.core.Type.DOUBLE).
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical or complex
        """
    def cast_column(self, from_name: str, to_name: str, type: Type) -> None:
        """
                Cast a numeric or complex column to a new numeric or complex type column.
        
                A new column of the specified type is created, and the content of the
                given numeric column is cast to the new type. If the input column type
                is identical to the specified type the column is duplicated as is done
                by the function  :py:meth:`duplicate_column`. Note that a column of
                arrays is always cast to another column of arrays of the specified type,
                unless it has depth 1. Consistently, a column of numbers can be cast
                to a column of arrays of depth 1.
                Here is a complete summary of how any (legal)  type specification
                would be interpreted, depending on the type of the input column:
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER
                specified type = cpl.core.Type.XXX | cpl.core.Type.POINTER
                to_name   type = cpl.core.Type.XXX | cpl.core.Type.POINTER
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth > 1)
                specified type = cpl.core.Type.XXX
                to_name   type = cpl.core.Type.XXX | cpl.core.Type.POINTER
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth = 1)
                specified type = cpl.core.Type.XXX
                to_name   type = cpl.core.Type.XXX
        
                from_name type = cpl.core.Type.XXX
                specified type = cpl.core.Type.XXX | cpl.core.Type.POINTER
                to_name   type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth = 1)
        
                from_name type = cpl.core.Type.XXX
                specified type = cpl.core.Type.POINTER
                to_name   type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth = 1)
        
                from_name type = cpl.core.Type.XXX
                specified type = cpl.core.Type.YYY
                to_name   type = cpl.core.Type.YYY
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER
                specified type = cpl.core.Type.YYY | cpl.core.Type.POINTER
                to_name   type = cpl.core.Type.YYY | cpl.core.Type.POINTER
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth > 1)
                specified type = cpl.core.Type.YYY
                to_name   type = cpl.core.Type.YYY | cpl.core.Type.POINTER
        
                from_name type = cpl.core.Type.XXX | cpl.core.Type.POINTER (depth = 1)
                specified type = cpl.core.Type.YYY
                to_name   type = cpl.core.Type.YYY
        
                from_name type = cpl.core.Type.XXX
                specified type = cpl.core.Type.YYY | cpl.core.Type.POINTER
                to_name   type = cpl.core.Type.YYY | cpl.core.Type.POINTER (depth = 1)
        
                Parameters
                ----------
                from_name : str
                    Name of table column to cast.
                to_name : str
                    Name of new table column.
                type : cpl.core.Type
                    Type of new table column.
        """
    def column_array(self, name: str) -> typing.Any:
        """
                Get column data in the form of a `numpy.ndarray`
        
                The array will be returned with the corresponding type as the column, containing in order all the values contained within column `name`.
        
                If the column is an array type, then the array returned will be 2d, with each nested array represents a value.
        
                Parameters
                ----------
                `name` : str
                    column to extract values
        
                Returns
                -------
                numpy.ndarray
                    array of values contained within column `name` in `self`.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If column `name` does not exist in `self`
                cpl.core.InvalidTypeError
                    If the column type cannot be cast to a numpy array
        """
    def compare_structure(self, other: Table) -> bool:
        """
                Compare the structure of two tables.
        
                Two tables have the same structure if they have the same number
                of columns, with the same names, the same types, and the same units.
                The order of the columns is not relevant.
        
                Parameters
                ----------
                other : cpl.core.Table
                    Other table to compare with.
        
                Returns
                -------
                bool
                    True if the tables have the same structure, otherwise False.
        """
    def conjugate_column(self, name: str) -> None:
        """
                Compute the complex conjugate of column values.
        
                Each column element is replaced by its complex conjugate.
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def copy_structure(self, toCopy: Table) -> None:
        """
                Copy the structure (column names, types and units) from another table
        
                This function assignes to a columnless table the same column structure
                (names, types, units) of a given model table. All columns are physically
                created in the new table, and they are initialised to contain just
                invalid elements.
        
                Parameters
                ----------
                toCopy : cpl.core.Table
                    table from which the structure is to be copied from.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if `self` contains columns
        """
    def count_invalid(self, name: str) -> int:
        """
                Count number of invalid values in a table column.
        
                Count number of invalid elements in a table column.
        
                Parameters
                ----------
                name : str
                    Name of table column to examine.
        
                Returns
                -------
                int
                    Number of invalid elements in a table column.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
        """
    def divide_columns(self, to_name: str, from_name: str) -> None:
        """
                Divide two numeric or complex table columns.
        
                The columns are divided element by element, and the result of the
                division is stored in the target column. The columns' types may
                differ, and in that case the operation would be performed using
                the standard C upcasting rules, with a final cast of the result
                to the target column type. Invalid elements are propagated consistently:
                if either or both members of the division are invalid, the result
                will be invalid too. Underflows and overflows are ignored, but a
                division by exactly zero will set an invalid column element.
        
                Parameters
                ----------
                to_name : str
                    Name of target column.
                from_name : str
                    Name of column dividing the target column.
        """
    def divide_scalar(self, name: str, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Divide a numerical or complex column by a constant.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : float
                    Divisor value.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
                cpl.core.DivisionByZeroError
                    `value` is equal to 0.0
        """
    def divide_scalar_complex(self, name: str, value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Divide a numerical or complex column by a complex constant.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : complex
                    Divisor value.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
                cpl.core.DivisionByZeroError
                    `value` is equal to 0.0
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', start: typing.SupportsInt | typing.SupportsIndex = 0, count: typing.Any = None, show: bool | None = True) -> str:
        """
                Dump the Table contents to a file, stdout or a string.
                  
                This function is mainly intended for debug purposes.
                All column elements are printed according to the column formats,
                that may be specified for each table column with the function.
        
                Parameters
                ----------
                filename : str, optional
                    file path to dump table contents to
                mode : str, optional
                    File mode to save the file, default 'w' overwrites contents.
                start : int
                    First row to print
                count : int
                    Number of rows to print
                show : bool, optional
                    Send table contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the table contents.
        """
    def duplicate_column(self, to_name: str, from_table: Table, from_name: str) -> None:
        """
                Copy a column from a table to `self`.
        
                Copy a column from a table to `self`. The column is duplicated. A column
                may be duplicated also within the same table.
        
                Parameters
                ----------
                to_name : str
                    New name of copied column.
                from_table : cpl.core.Table
                    Source table.
                from_name : str
                    Name of column to copy.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `from_name` is not found in `from_table`.
                cpl.core.IncompatibleInputError
                    `self` and `from_table` do not have the same number of rows
                cpl.core.IllegalOutputError
                    `to_name` already exists as a column in `self`
        """
    def erase_column(self, name: str) -> None:
        """
                Delete a column from a table.
        
                Delete a column from a table. If the table is left without columns,
                also the selection flags are lost.
        
                Parameters
                ----------
                name : str
                    Name of table column to delete.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def erase_invalid(self) -> None:
        """
                Remove from a table all columns just containing invalid elements,
                and then all rows containing at least one invalid element.
        
                Firstly, all columns consisting just of invalid elements are deleted
                from the table. Next, the remaining table rows containing at least
                one invalid element are also deleted from the table.
        
                The function is similar to the function :py:meth:`erase_invalid_rows`,
                except for the criteria to remove rows containing invalid elements after
                all invalid columns have been removed. While :py:meth:`erase_invalid_rows`
                requires all elements to be invalid in order to remove a row from the
                table, this function requires only one (or more) elements to be invalid.
        
                Notes
                -----
                If the input table just contains invalid elements, all columns are deleted.
        """
    def erase_invalid_rows(self) -> None:
        """
                Remove from a table columns and rows just containing invalid elements.
        
                Table columns and table rows just containing invalid elements are deleted
                from the table, i.e. a column or a row is deleted only if all of its
                elements are invalid. The selection flags are set back to "all selected"
                even if no rows or columns are removed.
        
                Notes
                -----
                If the input table just contains invalid elements, all columns are deleted.
        """
    def erase_selected(self) -> None:
        """
                Delete the selected rows of a table.
        
                A portion of the table data is physically removed, and the table
                selection flags are set back to "all selected".
        """
    def erase_window(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Delete a table segment.
        
                A portion of the table data is physically removed.
        
                Parameters
                ----------
                start : int
                    First row to delete.
                count : int
                    Number of rows to delete.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    The table has length of zero, or `start` is outside the table range.
                cpl.core.IllegalInputError
                    `count` is negative
        """
    def exponential_column(self, name: str, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the exponential of column values.
        
                Each column element is replaced by its exponential in the specified base.
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                base : float
                    Exponential base.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
                cpl.core.IllegalInputError
                    `base` is not positive
        """
    def extract(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> Table:
        """
                Create a table from a section of another table.
        
                A number of consecutive rows are copied from an input table to a
                newly created table. The new table will have the same structure of
                the original table (see function  :py:meth:`compare_structure`).
                If the sum of  start and  count goes beyond the end of the
                input table, rows are copied up to the end. All the rows of the
                new table are selected, i.e., existing selection flags are not
                transferred from the old table to the new one.
        
                Parameters
                ----------
                start : int
                    First row to be copied to new table.
                count : int
                    Number of rows to be copied.
        
                Returns
                -------
                cpl.core.Table
                    The new table.
        """
    def extract_selected(self) -> Table:
        """
                Create a new table from the selected rows of another table.
        
                A new table is created, containing a copy of all the selected
                rows of the input table. In the output table all rows are selected.
        
                Returns
                -------
                cpl.core.Table
                    New cpl.core.Table of selected rows
        """
    def fill_invalid(self, name: str, value: typing.Any) -> None:
        """
                Write a numerical value to invalid elements.
        
                The value will adapt to the column type
        
                Parameters
                ----------
                name : str
                    Column name.
                value : float, int, complex, or array
                    Value to write to invalid column elements.
        
                Notes
                -----
                Assigning a value to an invalid numerical element will not make it valid,
                but assigning a value to an element consisting of an array of numbers
                will make the array element valid.
        """
    def get(self, name: str, row: typing.SupportsInt | typing.SupportsIndex) -> tuple[float, int]:
        """
                Read a value from a numerical column.
        
                Rows are counted starting from 0.
        
                Parameters
                ----------
                name : str
                    Name of table column to be accessed.
                row : int
                    Position of element to be read.
        
                Returns
                -------
                float
                    Value read. In case of invalid table element 0.0 is returned.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `self` has zero length or `row` is outside table `self`'s boundaries
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
                cpl.core.InvalidTypeError
                    The specified column is not numerical, or is a column of arrays.
        """
    def get_column_depth(self, name: str) -> int:
        """
                Get the depth of a table column.
        
                Get the depth of a column. Columns of type array always have positive
                depth, while columns listing numbers or character strings have depth 0.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    Column depth
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def get_column_dimension(self, name: str, indx: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
                Get size of one dimension of a table column of arrays.
        
                Get the size of one dimension of a column. If a column is not an array
                column, or if it has no dimensions, 1 is returned.
        
                Parameters
                ----------
                name : str
                    Column name.
                indx : int
                    Indicate dimension to query (0 = x, 1 = y, 2 = z, etc.).
        
                Returns
                -------
                int
                    Size of queried dimension of the column, or zero in case of error.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
                cpl.core.UnsupportedModeError
                    A column with the given `name` is not of type cpl.core.Type.ARRAY
                cpl.core.AccessOutOfRangeError
                    The specified `indx` array is not compatible with the column dimensions
                cpl.core.IncompatibleInputError
                    The specified dimensions are incompatible with the total number of elements in the column arrays.
        """
    def get_column_dimensions(self, name: str) -> int:
        """
                Get the number of dimensions of a table column of arrays.
        
                Get the number of dimensions of a column. If a column is not an array
                column, or if it has no dimensions, 1 is returned.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    Column number of dimensions, or 0 in case of failure.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def get_column_format(self, name: str) -> str:
        """
                Get the format of a table column.
        
                Return the format of a column.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                str
                    Format of column.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def get_column_max(self, name: str) -> float:
        """
                Get maximum value in a numerical column.
        
                Invalid column values are excluded from the computation. The table
                selection flags have no influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                float
                    Maximum value. See documentation of  :py:meth:`get_column_mean`.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_maxpos(self, name: str) -> int:
        """
                Get position of maximum in a numerical column.
        
                Invalid column values are excluded from the search. The return value is the
                position of the maximum value where rows are counted starting from 0.
        
                If more than one column element correspond to the max value, the position
                with the lowest row number is returned.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    Returned row position of maximum value in column `name`
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_mean(self, name: str) -> float:
        """
                Compute the mean value of a numerical column.
        
                Invalid column values are excluded from the computation. The table
                selection flags have no influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                float
                    Mean value
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_mean_complex(self, name: str) -> complex:
        """
                Compute the mean value of a numerical or complex column.
        
                Invalid column values are excluded from the computation. The table
                selection flags have no influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                complex
                    Mean value. In case of error 0.0 is returned.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical or complex
        """
    def get_column_median(self, name: str) -> float:
        """
                Compute the median value of a numerical column.
        
                Invalid column values are excluded from the computation. The table
                selection flags have no influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                float
                    Median value
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_min(self, name: str) -> float:
        """
                Get minimum value in a numerical column.
        
                Invalid column values are excluded from the computation. The table
                selection flags have no influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                float
                    Minimum value. See documentation of  :py:meth:`get_column_mean`.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_minpos(self, name: str) -> int:
        """
                Get position of minimum in a numerical column.
        
                Invalid column values are excluded from the search. The return value is the
                position of the minimum value where rows are counted starting from 0.
        
                If more than one column element correspond to the minimum value, the position
                with the lowest row number is returned.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    Returned row position of minimum value in column `name`
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_stdev(self, name: str) -> float:
        """
                Find the standard deviation of a table column.
        
                Invalid column values are excluded from the computation of the
                standard deviation. If just one valid element is found, 0.0 is
                returned but no error is set. The table selection flags have no
                influence on the result.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                float
                    Standard deviation
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`, or it just contains invalid elements, or table length is zero
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def get_column_type(self, name: str) -> Type:
        """
                Get the type of a table column.
        
                Get the type of a column.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                cpl.core.Type
                    The column type of `name`
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def get_column_unit(self, name: str) -> str | None:
        """
                Get the unit of a table column.
        
                Return the unit of a column if present, otherwise `None` is
                returned.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                str
                    Unit of column.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def has_invalid(self, name: str) -> bool:
        """
                Check if a column contains at least one invalid value.
        
                Check if there are invalid elements in a column. In case of columns
                of arrays, invalid values within an array are not considered.
        
                Parameters
                ----------
                name : str
                    Name of table column to access.
        
                Returns
                -------
                bool
                    True if the column contains at least one invalid element
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
        """
    def has_valid(self, name: str) -> bool:
        """
                Check if a column contains at least one valid value.
        
                Check if there are valid elements in a column. In case of columns
                of arrays, invalid values within an array are not considered.
        
                Parameters
                ----------
                name : str
                    Name of table column to access.
        
                Returns
                -------
                bool
                    True if the column contains at least one valid element
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
        """
    def imag_column(self, name: str) -> None:
        """
                Compute the imaginary part value of table column elements.
        
                Each column element is replaced by its imaginary party value only.
                Invalid elements are not modified by this operation.
                If the column is complex, its type will be turned to
                real (cpl.core.Type.FLOAT_COMPLEX will be changed into cpl.core.Type.FLOAT,
                and cpl.core.Type.DOUBLE_COMPLEX will be changed into cpl.core.Type.DOUBLE).
                Existing references to the column data should be considered as invalid after
                calling this method.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical or complex
        """
    def insert(self, insert_table: Table, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Insert a table into `self`
        
                The input tables must have the same structure, as defined by the function
        
                Parameters
                ----------
                insert_table : cpl.core.Table
                    Table to be inserted in the target table.
                row : int
                    Row where to insert the insert table.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `row` is negative
                cpl.core.IncompatibleInputError
                    `self` and `insert_table` do not have the same structure.
        """
    def insert_window(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Insert a segment of rows into table data.
        
                Insert a segment of empty rows, just containing invalid elements.
                Setting `start` to a number greater than the column length is legal,
                and has the effect of appending extra rows at the end of the table:
                this is equivalent to expanding the table using set_size().
                The input column may also have zero length.
        
                The table selection flags are set back to "all selected".
        
                Parameters
                ----------
                start : int
                    Row where to insert the segment.
                count : int
                    Length of the segment.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `start` is negative
                cpl.core.IllegalInputError
                    `count` is negative
        """
    def is_selected(self, row: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
                Determine whether a table row is selected or not.
        
                Parameters
                ----------
                row : int
                    Table row to check.
        
                Returns
                -------
                bool
                    True if row is selected. False if its not.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with any of the specified names is not found in table.
                cpl.core.InvalidTypeError
                    Invalid types for comparison.
        
                See Also
                --------
                cpl.core.Table.and_selected : To select from already selected rows using column comparison
        """
    def is_valid(self, name: str, row: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
                Check if a column element is valid.
        
                Check if a column element is valid.
        
                Parameters
                ----------
                name : str
                    Name of table column to access.
                row : int
                    Column element to examine.
        
                Returns
                -------
                bool
                    True if the column element is valid, False if invalid
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
        """
    def logarithm_column(self, name: str, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the logarithm of column values.
        
                Each column element is replaced by its logarithm in the specified base.
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                not modified by this operation, but zero or negative elements are
                invalidated by this operation. In case of complex numbers, values
                very close to the origin may cause an overflow. The imaginary part
                of the result is chosen in the interval [-pi/ln(base),pi/ln(base)],
                so it should be kept in mind that doing the logarithm of exponential
                of a complex number will not always express the phase angle with the
                same number. For instance, the exponential in base 2 of (5.00, 5.00)
                is (-30.33, -10.19), and the logarithm in base 2 of the latter will
                be expressed as (5.00, -4.06).
        
                Parameters
                ----------
                name : str
                    Table column name.
                base : float
                    Logarithm base.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
                cpl.core.IllegalInputError
                    `base` is not positive
        """
    def move_column(self, name: str, from_table: Table) -> None:
        """
                Move a column from a table to `self`.
        
                Move a column from a table to `self`.
        
                Parameters
                ----------
                name : str
                    Name of column to move.
                from_table : cpl.core.Table
                    Source table.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `from_table`.
                cpl.core.IncompatibleInputError
                    `self` and `from_table` do not have the same number of rows
                cpl.core.IllegalInputError
                    `self` and `from_table` are the same table (object)
                cpl.core.IllegalOutputError
                    `name` already exists as a column in `self`
        """
    def multiply_columns(self, to_name: str, from_name: str) -> None:
        """
                Multiply two numeric or complex table columns.
        
                The columns are multiplied element by element, and the result of the
                multiplication is stored in the target column. See the documentation of
                the function  :py:meth:`add_columns` for further details.
        
                Parameters
                ----------
                to_name : str
                    Name of target column.
                from_name : str
                    Name of column to be multiplied with target column.
        """
    def multiply_scalar(self, name: str, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Multiply a numerical or complex column by a constant.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                are not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : float
                    Multiplication factor.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def multiply_scalar_complex(self, name: str, value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Multiply a numerical or complex column by a complex constant.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                are not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : complex
                    Multiplication factor.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def name_column(self, from_name: str, to_name: str) -> None:
        """
                Rename a table column.
        
                This function is used to change the name of a column.
        
                Parameters
                ----------
                from_name : str
                    Name of table column to rename.
                to_name : str
                    New name of column.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `from_name` is not found in `self`.
                cpl.core.IllegalOutputError
                    `name` already exists as a column in `self`
        """
    def new_column(self, name: str, type: Type) -> None:
        """
                Create an empty column in a table.
        
                Creates a new column of specified `type`, excluding array types
                (for creating a column of arrays use the function :py:meth:`new_column_array`,
                where the column depth must also be specified).
        
                The new column name must be different from  any other column name
                in the table. All the elements of the new column are marked as invalid.
        
                Parameters
                ----------
                name : str
                    Name of the new column.
                type : cpl.core.Type
                    Type of the new column.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    `type` is not supported by cpl.core.Table
                cpl.core.IllegalOutputError
                    column with the same `name` already exists in the table
        """
    def new_column_array(self, name: str, type: Type, depth: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Create an empty column of arrays in a table.
        
                This creates a new column of specified array length.
        
                Parameters
                ----------
                name : str
                    Name of the new column.
                type : cpl.core.Type
                    Type of the new column.
                depth : int
                    Depth of the new column.
        
                Raises
                ------
                cpl.core.InvalidTypeError
                    `type` is not supported by cpl.core.Table
                cpl.core.IllegalInputError
                    The specified `depth` is negative
                cpl.core.IllegalOutputError
                    column with the same `name` already exists in the table
        """
    def not_selected(self) -> int:
        """
                Select unselected table rows, and unselect selected ones.
        
                Returns
                -------
                int
                    New number of selected rows
        """
    def or_selected(self, name1: str, operator: Table.Operator, name2: str) -> int:
        """
                Select from unselected table rows, by comparing the values of two columns.
        
                Either both columns must be numerical, or they both must be strings.
                The comparison between strings is lexicographical. Neither can be a
                complex or array type.
        
                For all unselected table rows, the values of the specified
                column are compared. All the table rows fulfilling the comparison are selected.
                Invalid elements from either columns never fulfill any comparison by definition.
        
                For this function, the column is of a numerical type if its type is:
                * cpl.core.Type.INT
                * cpl.core.Type.FLOAT
                * cpl.core.Type.DOUBLE
                * cpl.core.Type.LONG_LONG
        
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name1 : str
                    Name of the first table column
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                name2 : str
                    Name of second table column.
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with any of the specified names is not found in table.
                cpl.core.InvalidTypeError
                    Invalid types for comparison.
        
                See Also
                --------
                cpl.core.Table.and_selected : To select from already selected rows using column comparison
        """
    def or_selected_invalid(self, name: str) -> int:
        """
                Select from unselected table rows all rows with an invalid value in a specified column.
        
                For all the unselected table rows, all the rows containing invalid
                values at the specified column are selected.
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Returns
                -------
                int
                    New number of selected rows
        """
    def or_selected_numerical(self, name: str, operator: Table.Operator, value: typing.Any) -> int:
        """
                Select from unselected table rows, by comparing a column of numerical values to the reference `value`
        
                For all the unselected table rows, the values of the specified
                column are compared with the reference value.
        
                The column is of a numerical type if its type is:
                * cpl.core.Type.INT
                * cpl.core.Type.FLOAT
                * cpl.core.Type.DOUBLE
                * cpl.core.Type.DOUBLE_COMPLEX
                * cpl.core.Type.FLOAT_COMPLEX
                * cpl.core.Type.LONG_LONG
        
                All table rows fulfilling the comparison are selected. An invalid element never
                fulfills any comparison by definition.
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name : str
                    Column name.
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                value : int, float or complex
                    Reference value
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if the column of the given `name` wasn't found.
                cpl.core.TypeMismatchError
                    Column of the given `name` is not numerical
        
                See Also
                --------
                cpl.core.Table.and_selected_numerical : To select from already selected rows using numerical operator comparison
        """
    def or_selected_string(self, name: str, operator: Table.Operator, string: str) -> int:
        """
                Select from unselected table rows, by comparing a column of string values to the given `string`.
        
                For all the unselected table rows, the values of the specified column are compared with the
                reference string. The table rows fulfilling the comparison are selected. An invalid element never
                fulfills any comparison by definition.
        
                If `operator` is equal to cpl.core.Operator.EQUAL_TO or cpl.core.Operator.NOT_EQUAL_TO
                then the comparison string is treated as a regular expression.
        
                Allowed relational operators are
                * cpl.core.Operator.EQUAL_TO
                * cpl.core.Operator.NOT_EQUAL_TO
                * cpl.core.Operator.GREATER_THAN
                * cpl.core.Operator.NOT_GREATER_THAN
                * cpl.core.Operator.LESS_THAN
                * cpl.core.Operator.NOT_LESS_THAN
        
                Parameters
                ----------
                name : str
                    Column name.
                operator : cpl.core.Operator
                    Relational Operator. See extended summary for allowed operators.
                string : str
                    Reference character string
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    if the column of the given `name` wasn't found.
                cpl.core.TypeMismatchError
                    Column of the given `name` is not of type cpl.core.Type.STRING
                cpl.core.IllegalInputError
                    Invalid regular expression
        
                See Also
                --------
                cpl.core.Table.and_selected_string : To select from already selected rows using string comparison
        """
    def or_selected_window(self, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> int:
        """
                Select from unselected rows only those within a table segment.
        
                All the unselected table rows that are within the specified interval are
                selected. If the sum of `start` and `count` goes beyond the end
                of the input table, rows are checked up to the end of the table.
        
                Parameters
                ----------
                start : int
                    First row of table segment.
                count : int
                    Length of segment
        
                Returns
                -------
                int
                    New number of selected rows
        
                Raises
                ------
                cpl.core.AccessOutOfRange
                    `self` has zero length, or `start` is outside `self`'s boundaries
                cpl.core.IllegalInputError
                    `count` is negative
        
                See Also
                --------
                cpl.core.Table.and_selected_window : To select from already selected rows using a specified segment
        """
    def power_column(self, name: str, exponent: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the power of numerical column values.
        
                Each column element is replaced by its power to the specified exponent.
                For float and float complex the operation is performed in single precision,
                otherwise it is performed in double precision and then rounded if the column
                is of an integer type. Results that would or do cause domain errors or
                overflow are marked as invalid.
        
                Parameters
                ----------
                name : str
                    Name of column of numerical type
                exponent : float
                    Constant exponent.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical
        """
    def real_column(self, name: str) -> None:
        """
                Compute the real part value of table column elements.
        
                Each column element is replaced by its real party value only.
                Invalid elements are not modified by this operation.
                If the column is complex, its type will be turned to
                real (cpl.core.Type.FLOAT_COMPLEX will be changed into cpl.core.Type.FLOAT,
                and cpl.core.Type.DOUBLE_COMPLEX will be changed into cpl.core.Type.DOUBLE).
        
                Parameters
                ----------
                name : str
                    Column name.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical or complex
        """
    def save(self, pheader: cpl.core.PropertyList | None, header: PropertyList, filename: str, mode: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Save a CPL table to a FITS file.
        
                This function can be used to convert a CPL table into a binary FITS
                table extension. If the  mode is set to  cpl.core.IO.CREATE, a new
                FITS file will be created containing an empty primary array, with
                just one FITS table extension. An existing (and writable) FITS file
                with the same name would be overwritten. If the  mode flag is set
                to  cpl.core.IO.EXTEND, a new table extension would be appended to an
                existing FITS file. If  mode is set to  cpl.core.IO.APPEND it is possible
                to add rows to the last FITS table extension of the output FITS file.
        
                Note that the modes  cpl.core.IO.EXTEND and  cpl.core.IO.APPEND require that
                the target file must be writable (and do not take for granted that a file
                is writable just because it was created by the same application,
                as this depends on the system  umask).
        
                When using the mode  cpl.core.IO.APPEND additional requirements must be
                fulfilled, which are that the column properties like type, format, units,
                etc. must match as the properties of the FITS table extension to which the
                rows should be added exactly. In particular this means that both tables use
                the same null value representation for integral type columns!
        
                Two property lists may be passed to this function, both
                optionally. The first property list,  pheader, is just used if
                the  mode is set to  cpl.core.IO.CREATE, and it is assumed to
                contain entries for the FITS file primary header. In  pheader any
                property name related to the FITS convention, as ``SIMPLE``, ``BITPIX``,
                ``NAXIS``, ``EXTEND``, ``BLOCKED``, and ``END``, are ignored: such
                entries would be written anyway to the primary header and set to some
                standard values.
        
                If a no pheader is passed, the primary array would be created
                with just such entries, that are mandatory in any regular FITS file.
                The second property list,  header, is assumed to contain entries
                for the FITS table extension header. In this property list any
                property name related to the FITS convention, as ``XTENSION``,
                ``BITPIX``, ``NAXIS``, ``PCOUNT``, ``GCOUNT``, and ``END``, and to
                the table structure, as ``TFIELDS``, ``TTYPEi``, ``TUNITi``,
                ``TDISPi``, ``TNULLi``, ``TFORMi``, would be ignored: such
                entries are always computed internally, to guarantee their
                consistency with the actual table structure. A ``DATE`` keyword
                containing the date of table creation in ISO8601 format is also
                added automatically.
        
                Using the mode  cpl.core.IO.APPEND requires that the column properties of
                the table to be appended are compared to the column properties of the
                target FITS extension for each call, which introduces a certain overhead.
                This means that appending a single table row at a time may not be
                efficient and is not recommended. Rather than writing one row at a
                time one should write table chunks containing a suitable number or rows.
        
                Parameters
                ----------
                pheader : cpl.core.Propertylist
                    Primary header entries.
                header : cpl.core.Propertylist
                    Table header entries.
                filename : str
                    Name of output FITS file.
                mode : unsigned
                    Output mode.
        
                Notes
                -----
                Invalid strings in columns of type  cpl.core.Type.STRING are
                written to FITS as blanks.
        """
    def select_all(self) -> None:
        """
                Select all table rows.
        
                The table selection flags are reset, meaning that they are
                all marked as selected. This is the initial state of any
                table.
        """
    def select_row(self, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Flag a table row as selected.
        
                Flag a table row as selected. Any previous selection is kept.
        
                Parameters
                ----------
                row : int
                    Row to select.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `self` has a length of zero, or `row` is outside the table boundaries.
        """
    def set_column_depth(self, name: str, depth: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Modify depth of a column of arrays
        
                This function is applicable just to columns of arrays. The contents
                of the arrays in the specified column will be unchanged up to the
                lesser of the new and old depths. If the depth is increased, the
                extra array elements would be flagged as invalid. Existing references
                to the array data should be considered invalid after calling this method.
        
                Parameters
                ----------
                name : str
                    Column name.
                depth : int
                    New column depth.
        """
    def set_column_dimensions(self, name: str, dimensions: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        """
                Set the dimensions of a table column of arrays.
        
                Set the number of dimensions of a column. If the  dimensions array
                has size less than 2, nothing is done and no error is returned.
        
                Parameters
                ----------
                name : str
                    Column name.
                dimensions : list of int
                    the sizes of the column dimensions
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
                cpl.core.IllegalInputError
                    A column with the given `name` is not of type cpl.core.Type.ARRAY, or `dimensions` contains invalid elements
                cpl.core.IncompatibleInputError
                    The specified dimensions are incompatible with the total number of elements in the column arrrays.
        """
    def set_column_format(self, name: str, format: str) -> None:
        """
                Give a new format to a table column.
        
                The input format string is duplicated before being used as the column
                format. If no format is set, "%s" will be used if
                the column is of type cpl.core.Type.STRING, "%1.5e" if the column is
                of type  cpl.core.Type.FLOAT or cpl.core.Type.DOUBLE, and "%7d" if it is
                of type  cpl.core.Type.INT. The format associated to a column has no
                effect on any operation performed on columns, and it is used just
                while printing a table using the function :py:meth:`dump`.
        
                This information is lost after saving the table in FITS format using  save().
        
                Parameters
                ----------
                name : str
                    Column name.
                format : str
                    New format.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def set_column_invalid(self, name: str, start: typing.SupportsInt | typing.SupportsIndex, count: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Invalidate a column segment.
        
                All the column elements in the specified interval are invalidated.
                In the case of either a string or an  rray column, the
                corresponding strings or arrays are set free. If the sum of start
                and count exceeds the number of rows in the table, the column is
                invalidated up to its end.
        
                Parameters
                ----------
                name : str
                    Name of table column to access.
                start : int
                    Position where to begin invalidation.
                count : int
                    Number of column elements to invalidate.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
                cpl.core.AccessOutOfRangeError
                    `self` has zero length, or `start` is outside the table boundaries.
                cpl.core.IllegalInputError
                    `count` is negative
        """
    def set_column_unit(self, name: str, unit: str) -> None:
        """
                Give a new unit to a table column.
        
                The input unit string is duplicated before being used as the column
                unit.
        
                The unit associated to a column has no effect on any operation performed
                on columns, and it must be considered just an optional description of
                the content of a column. It is however saved to a FITS file when using
                save().
        
                Parameters
                ----------
                name : str
                    Column name.
                unit : str
                    New unit.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    A column with the given `name` not found in table.
        """
    def set_invalid(self, name: str, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Flag a column element as invalid.
                
                The column element given by the column name `name` and the row number `row` is flagged as invalid.
                This also means that the data which was stored in this table cell becomes inaccessible. To reset
                an invalid column cell it must be updated with a new value.
        
                Parameters
                ----------
                name : str
                    Name of table column to access.
                row : int
                    Table row to set to invalid.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `self` has zero length or `row` is outside table `self`'s boundaries
                cpl.core.DataNotFoundError
                    A column with the given `name` is not found in `self`.
        """
    def set_size(self, new_length: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Resize a table to a new number of rows.
        
                The contents of the columns will be unchanged up to the lesser of the
                new and old sizes. If the table is expanded, the extra table rows would
                just contain invalid elements. The table selection flags are set back
                to "all selected". Existing eferences to the column data should be considered
                invalid after calling this method.
        
                Parameters
                ----------
                new_length : int
                    New number of rows in table.
        """
    def shift_column(self, name: str, shift: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Shift the position of numeric or complex column values.
        
                The position of all column values is shifted by the specified amount.
                If  shift is positive, all values will be moved toward the bottom
                of the column, otherwise toward its top. In either case as many column
                elements as the amount of the  shift will be left undefined, either
                at the top or at the bottom of the column according to the direction
                of the shift. These column elements will be marked as invalid. This
                function is applicable just to numeric and complex columns, and not
                to strings and or array types. The selection flags are always set back
                to "all selected" after this operation.
        
                Parameters
                ----------
                name : str
                    Name of table column to shift.
                shift : int
                    Shift column values by so many rows.
        """
    def sort(self, reflist: PropertyList) -> None:
        """
                Sort table rows according to columns values.
        
                The table rows are sorted according to the values of the specified
                reference columns. The reference column names are listed in the input
        
                Parameters
                ----------
                reflist : cpl.core.Propertylist
                    Names of reference columns with corresponding sorting mode.
        """
    def subtract_columns(self, to_name: str, from_name: str) -> None:
        """
                Subtract two numeric or complex table columns.
        
                The columns are subtracted element by element, and the result of the
                subtraction is stored in the target column. See the documentation of
                the function  :py:meth:`add_columns` for further details.
        
                Parameters
                ----------
                to_name : str
                    Name of target column.
                from_name : str
                    Name of column to be subtracted from target column.
        """
    def subtract_scalar(self, name: str, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Subtract a constant value from a numerical or complex column.
        
                See the description of the function  :py:meth:`add_scalar`.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : float
                    Value to subtract.
        """
    def subtract_scalar_complex(self, name: str, value: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Subtract a constant complex value from a numerical or complex column.
        
                The operation is always performed in double precision, with a final
                cast of the result to the target column type. Invalid elements are
                are not modified by this operation.
        
                Parameters
                ----------
                name : str
                    Column name.
                value : complex
                    Value to subtract.
        
                Raises
                ------
                cpl.core.DataNotFoundError
                    If a column with the specified `name` is not found in `self`
                cpl.core.InvalidTypeError
                    If the requested column is not numerical, or is an array column
        """
    def to_records(self) -> typing.Any:
        """
                Convert the cpl.core.Table to a numpy recarray
        
                Returns
                -------
                numpy.recarray
                    numpy recarray containing entries and values from the cpl.core.Table
        """
    def unselect_all(self) -> None:
        """
                Unselect all table rows.
        
                The table selection flags are all unset, meaning that no table
                rows are selected.
        """
    def unselect_row(self, row: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Flag a table row as unselected.
        
                Flag a table row as unselected. Any previous selection is kept.
        
                Parameters
                ----------
                row : int
                    Row to unselect.
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    `self` has a length of zero, or `row` is outside the table boundaries.
        """
    def where_selected(self) -> numpy.ndarray:
        """
                Get array of indexes to selected table rows
        
                Get array of indexes to selected table rows. If no rows are selected,
                an array of size zero is returned.
        
                Returns
                -------
                list of int
                    Indexes to selected table rows
        """
    @property
    def column_names(self) -> list[str]:
        """
        list of all column names
        """
    @property
    def selected(self) -> int:
        """
        int : number of selected rows in given table.
        """
    @property
    def shape(self) -> tuple:
        """
        Shape of the table in the format (rows, columns)
        """
class Type:
    """
    Members:
    
      UNSPECIFIED
    
      CHAR
    
      UCHAR
    
      BOOL
    
      SHORT
    
      USHORT
    
      INT
    
      UINT
    
      LONG
    
      ULONG
    
      LONG_LONG
    
      SIZE
    
      FLOAT
    
      DOUBLE
    
      FLOAT_COMPLEX
    
      DOUBLE_COMPLEX
    
      STRING
    
      ARRAY
    """
    ARRAY: typing.ClassVar[Type]  # value = <Type.ARRAY: 262144>
    BOOL: typing.ClassVar[Type]  # value = <Type.BOOL: 128>
    CHAR: typing.ClassVar[Type]  # value = <Type.CHAR: 32>
    DOUBLE: typing.ClassVar[Type]  # value = <Type.DOUBLE: 131072>
    DOUBLE_COMPLEX: typing.ClassVar[Type]  # value = <Type.DOUBLE_COMPLEX: 655360>
    FLOAT: typing.ClassVar[Type]  # value = <Type.FLOAT: 65536>
    FLOAT_COMPLEX: typing.ClassVar[Type]  # value = <Type.FLOAT_COMPLEX: 589824>
    INT: typing.ClassVar[Type]  # value = <Type.INT: 1024>
    LONG: typing.ClassVar[Type]  # value = <Type.LONG: 4096>
    LONG_LONG: typing.ClassVar[Type]  # value = <Type.LONG_LONG: 16384>
    SHORT: typing.ClassVar[Type]  # value = <Type.SHORT: 256>
    SIZE: typing.ClassVar[Type]  # value = <Type.SIZE: 32768>
    STRING: typing.ClassVar[Type]  # value = <Type.STRING: 33>
    UCHAR: typing.ClassVar[Type]  # value = <Type.UCHAR: 64>
    UINT: typing.ClassVar[Type]  # value = <Type.UINT: 2048>
    ULONG: typing.ClassVar[Type]  # value = <Type.ULONG: 8192>
    UNSPECIFIED: typing.ClassVar[Type]  # value = <Type.UNSPECIFIED: 1048576>
    USHORT: typing.ClassVar[Type]  # value = <Type.USHORT: 512>
    __members__: typing.ClassVar[dict[str, Type]]  # value = {'UNSPECIFIED': <Type.UNSPECIFIED: 1048576>, 'CHAR': <Type.CHAR: 32>, 'UCHAR': <Type.UCHAR: 64>, 'BOOL': <Type.BOOL: 128>, 'SHORT': <Type.SHORT: 256>, 'USHORT': <Type.USHORT: 512>, 'INT': <Type.INT: 1024>, 'UINT': <Type.UINT: 2048>, 'LONG': <Type.LONG: 4096>, 'ULONG': <Type.ULONG: 8192>, 'LONG_LONG': <Type.LONG_LONG: 16384>, 'SIZE': <Type.SIZE: 32768>, 'FLOAT': <Type.FLOAT: 65536>, 'DOUBLE': <Type.DOUBLE: 131072>, 'FLOAT_COMPLEX': <Type.FLOAT_COMPLEX: 589824>, 'DOUBLE_COMPLEX': <Type.DOUBLE_COMPLEX: 655360>, 'STRING': <Type.STRING: 33>, 'ARRAY': <Type.ARRAY: 262144>}
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
class TypeMismatchError(Error, RuntimeError):
    """
    Data were not of the expected type.
    
    A CPL Error subclass. This is a CPL Error that is thrown from C/C++
    and has C/C++ stacktrace available, with line numbers, file names, function
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
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
    names, and CPL Error codes. See cpl.core.Error help documentation for more
    help on members and methods (scroll down to inherited methods)
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    code: typing.ClassVar[int] = 16
class Vector:
    """
    
            Class for ordered sequences of numbers.
    
            A `cpl.core.Vector` contains an ordered list of double precision floating point numbers.
            It has methods for sorting, statistics, and other simple operations. Two Vectors may
            be combined into a `cpl.core.Bivector` to represent sequences of x and y values.
    
            A Vector can also be created using the zeros class method.
    
            Parameters
            ----------
            data : iterable of floats
                An iterable object which yields floating point values.
    
            See Also
            --------
            cpl.core.Bivector: Class for pairs of ordered sequences of numbers.
            cpl.core.Vector.zeros: Create a Vector of given length, initialised with 0's.
    
            Examples
            --------
            >>> vector_list = cpl.core.Vector([1, 2, 3])
            ... vector_tuple = cpl.core.Vector((4, 5, 6))
            ... vector_copy = cpl.core.Vector(vector_list)
            ... vector_zeros = cpl.core.Vector.zeros(5)
        
    """
    class LowPass:
        """
        Filter type for cpl.core.Vector.filter_lowpass_create
        
        Members:
        
          LINEAR
        
          GAUSSIAN
        """
        GAUSSIAN: typing.ClassVar[Vector.LowPass]  # value = <LowPass.GAUSSIAN: 1>
        LINEAR: typing.ClassVar[Vector.LowPass]  # value = <LowPass.LINEAR: 0>
        __members__: typing.ClassVar[dict[str, Vector.LowPass]]  # value = {'LINEAR': <LowPass.LINEAR: 0>, 'GAUSSIAN': <LowPass.GAUSSIAN: 1>}
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
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def correlate(v1: Vector, v2: Vector, max_shift: typing.SupportsInt | typing.SupportsIndex) -> tuple[Vector, int]:
        """
                Return cross-correlation of two vectors.
        
                The length of `v2` may not exceed that of `v1`. If the difference in length
                between `v1` and `v2` is less than `max_shift` then this difference must be
                even (if the difference is odd resampling of `v2` may be useful).
        
                The cross-correlation is in fact the dot product of two unit vectors and
                therefore ranges from -1 to 1.
        
                The cross-correlation is computed with shifts ranging from `-max_shift`
                to `+max_shift`.
        
                On success, element i (starting with 0) of the returned `cpl.core.Vector` contains
                the cross-correlation at offset `i - max_shift`.
        
                If `v1` is longer than `v2`, the first element in `v1` used for the resulting
                cross-correlation is `max(0, shift + (len(v1) - len(v2)) / 2)`.
        
                Parameters
                ----------
                v1 : cpl.core.Vector
                    1st vector to correlate
                v2 : cpl.core.Vector
                    2nd vector to correlate
                max_shift : int
                    Maximum size of shift to be used when calculating cross correlation.
        
                Returns
                -------
                cpl.core.Vector
                    New Vector of size `2 * max_shift + 1` containing the cross correlation of
                    `v1` and `v2` for shifts ranging from `-max_shift` to `+max_shift`. 
                int
                    Index of output Vector at which the maximum cross correlation value occurs.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if `v1` and `v2` have incompatible sizes.
        
                Notes
                -----
                The cross-correlation is, in absence of rounding errors, commutative only for
                equal-sized vectors, i.e. changing the order of `v1` and `v2` will move element `j`
                in the returned Vector to `2 * max_shift - j` and thus change the index of maximum
                cross correlation from `i` to `2 * max_shift - i`.
        
                If, in absence of rounding errors, more than one shift would give the maximum
                cross-correlation, rounding errors may cause any one of those shifts to be
                returned. If rounding errors have no effect the index corresponding to the
                shift with the smallest absolute value is returned (with preference given to
                the smaller of two indices that correspond to the same absolute shift).
        
                Cross-correlation with `max_shift == 0` requires about 8n FLOPs, where
                `n` is the number of elements of `v2`.
        
                Each increase of `max_shift` by 1 requires about 4n FLOPs more, when all 
                elements of `v2` can be cross-correlated, otherwise the extra cost is about 4m,
                where `m` is the number of elements in `v2` that can be cross-correlated,
                `n - max_shift <= m < n`.
        
                Example of 1D-wavelength calibration (error handling omitted for brevity):
        
                .. code-block:: python
        
                    # Dispersion is of type cpl.core.Polynomial
                    # The return type of mymodel() and myobservation() is cpl.core.Vector
                    model = mymodel(dispersion)
                    observed = myobservation()
                    vxc, max_index = cpl.core.Vector.correlate(model, observed, max_shift)
                    dispersion.shift_1d(0, max_index - max_shift)
        """
    @staticmethod
    def fit_gaussian(x: typing.Any, y: typing.Any, y_sigma: typing.Any, fit_pars: typing.SupportsInt | typing.SupportsIndex, x0: typing.SupportsFloat | typing.SupportsIndex | None = None, sigma: typing.SupportsFloat | typing.SupportsIndex | None = None, area: typing.SupportsFloat | typing.SupportsIndex | None = None, offset: typing.SupportsFloat | typing.SupportsIndex | None = None) -> typing.Any:
        """
                    Apply a 1d gaussian fit.
        
                    This function fits to the input vectors a 1d gaussian function of the form
        
                    .. math::
        
                      f(x) =  \\mathrm{area} / \\sqrt{2 \\pi \\sigma^2} * \\exp(-(x - x0)^2 / (2 \\sigma^2)) + \\mathrm{offset}
        
                    where `area` > 0, by minimizing chi^2 using a Levenberg-Marquardt algorithm.
        
                    The values to fit are read from the input vector `x`.
        
                    The diagonal elements (the variances) are guaranteed to be positive.
        
                    Occasionally, the Levenberg-Marquardt algorithm fails to converge to a set of
                    sensible parameters. In this case (and only in this case), a
                    cpl.core.ContinueError is set. To allow the caller to recover from this
                    particular error.
        
                    Parameters
                    ----------
                    x : cpl.core.Vector
                        Positions to fit
                    y : cpl.core.Vector
                        The N values to fit.
                    y_sigma : cpl.core.Vector
                        Uncertainty (one sigma, gaussian errors assumed) associated with y
                    fit_pars : cpl.core.FitMode
                        Specifies which parameters participate in the fit (any other parameters will be held constant).
                        Possible values are cpl.core.FitMode.CENTROID, cpl.core.FitMode.STDEV, cpl.core.FitMode.AREA,
                        cpl.core.FitMode.OFFSET and cpl.core.FitMode.ALL, and any bitwise combination of these (using
                        bitwise OR).
                    x0 : double, optional
                        Preset center of best fit gaussian if cpl.core.FitMode.CENTROID is not used in fit_pars.
                        Value is unused otherwise.
                    sigma : doublee, optional
                        Width of best fit gaussian if cpl.core.FitMode.STDEV is not used in fit_pars.
                        Value is unused otherwise.
                    area : doublee, optional
                        Area of gaussian if cpl.core.FitMode.AREA is not used in fit_pars.
                        Value is unused otherwise.
                    offset : doublee, optional
                        Fitted background level if cpl.core.FitMode.OFFSET is not used in fit_pars.
                        Value is unused otherwise.
                    Returns
                    -------
                    NamedTuple(float, float, float, float, float, float, cpl.core.Matrix)
                        A FitGaussianResult NamedTuple with the following elements:
                          x0 : float
                              Center of best fit gaussian.
                          sigma : float
                              Width of best fit gaussian. A positive number on success.
                          area : float
                              Area of gaussian. A positive number on succes.
                          offset : float
                              Fitted background level.
                          mse : float
                              the mean squared error of the best fit
                          red_chisq : float
                              the reduced chi-squared of the best fit. None if `y_sigma` is not passed
                          covariance : cpl.core.Matrix
                              The formal covariance matrix of the best fit, On success the diagonal
                              terms of the covariance matrix are guaranteed to be positive.
                              However, terms that involve a constant parameter (as defined by the input
                              array `evaluate_derivatives`) are always set to zero. None if `y_sigma`
                              is not passed
        
                    Raises
                    ------
                    cpl.core.InvalidTypeError
                        if the specified fit_pars is not a bitwise combination of the allowed values (e.g. 0 or 1).
                    cpl.core.IncompatibleInputError
                        if the sizes of any input vectors are different, or if the computation of reduced chi square or covariance is requested, but sigma_y is not provided.
                    cpl.core.IllegalInputError
                        if any input noise values, sigma or area is non-positive, or if chi square computation is requested and there are less than 5 data points to fit, or if an
                        initial value is required for x0, sigma, area or offset when a fit_pars mode is not present.
                    cpl.core.IllegalOutputError
                        if memory allocation failed.
                    cpl.core.ContinueError
                        if the fitting algorithm failed.
                    cpl.core.SingularMatrixError
                        if the covariance matrix could not be calculated.
        """
    @staticmethod
    def kernel_profile(type: Kernel, radius: typing.SupportsFloat | typing.SupportsIndex, size: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        """
                Return a Vector containing a kernel profile.
        
                A number of predefined kernel profiles are available:
                - cpl.core.Kernel.DEFAULT: default kernel, currently cpl.core.Kernel.TANH
                - cpl.core.Kernel.TANH: Hyperbolic tangent
                - cpl.core.Kernel.SINC: Sinus cardinal
                - cpl.core.Kernel.SINC2: Square sinus cardinal
                - cpl.core.Kernel.LANCZOS: Lanczos2 kernel
                - cpl.core.Kernel.HAMMING: Hamming kernel
                - cpl.core.Kernel.HANN: Hann kernel
                - cpl.core.Kernel.NEAREST: Nearest neighbor kernel (1 when dist < 0.5, else 0)
        
                Parameters
                ----------
                type : cpl.core.Kernel
                    Type of kernel profile.
                radius : float
                    Radius of the profile in pixels
                size : int
                    Size of the kernel profile in pixels.
        
                Returns
                -------
                cpl.core.Vector
                    Vector of length `size` containing the calculated kernel values.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if `radius` is non-positive, or in case of the `cpl.core.Kernel.TANH` profile if `size` exceeds 32768
        """
    @staticmethod
    def load(filename: str, xtnum: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        """
                Load a list of values from a FITS file
        
                This function loads a vector from a FITS file (``NAXIS`` = 1).
        
                `xtnum` specifies from which extension the vector should be loaded.
                This could be 0 for the main data section or any number between 1 and N,
                where N is the number of extensions present in the file.
        
                Parameters
                ----------
                filename : str
                    Name of the input file
                xtnum : int
                    Extension number in the file (0 for primary HDU)
        
                Returns
                -------
                cpl.core.Vector
                    The loaded vector from the file, at extension xtnum
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the extension is not valid
                cpl.core.FileIOError
                    if the file cannot be read
                cpl.core.UnsupportedModeError
                     if the file is too large to be read
        """
    @staticmethod
    def read(filename: str) -> Vector:
        """
                Read a list of values from an ASCII file and create a Vector
        
                Parse an input ASCII file values and create a Vector from it
                Lines beginning with a hash are ignored, blank lines also.
                In valid lines the value is preceeded by an integer, which is ignored.
        
                In addition to normal files, FIFO (see man mknod) are also supported.
        
                Parameters
                ----------
                filename : cpl.core.std::string
                    Name of the input ASCII file
        
                Returns
                -------
                cpl.core.Vector
                    A new Vector with the parsed ASCII file values
        
                Raises
                ------
                cpl.core.FileIOError
                    if the file cannot be read
                cpl.core.BadFileFormatError
                    if the file contains no valid lines
        """
    @staticmethod
    def zeros(size: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        """
                Create a Vector of given length, initialised with 0's.
        
                Parameters
                ----------
                size : int
                    size of the new Vector
        
                Returns
                -------
                cpl.core.Vector
                    New cpl.core.Vector, length `size`, initialised with 0's
        
                Raises
                ------
                cpl.core.IllegalInputError
                    size is non-positive
        """
    def __array__(self, **kwargs) -> numpy.typing.NDArray[numpy.float64]:
        ...
    @typing.overload
    def __eq__(self, arg0: Vector) -> bool:
        ...
    @typing.overload
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> float:
        ...
    @typing.overload
    def __getitem__(self, arg0: slice) -> Vector:
        ...
    @typing.overload
    def __init__(self, data: Vector) -> None:
        ...
    @typing.overload
    def __init__(self, data: collections.abc.Iterable) -> None:
        ...
    def __iter__(self) -> VectorIterator:
        ...
    def __len__(self) -> int:
        ...
    @typing.overload
    def __ne__(self, arg0: Vector) -> bool:
        ...
    @typing.overload
    def __ne__(self, arg0: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    @typing.overload
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    def __str__(self) -> str:
        ...
    def add(self, other: Vector) -> None:
        """
                Add a cpl.core.Vector to self
        
                The other vector must have the same size as the calling vector
        
                Parameters
                ----------
                other : cpl.core.Vector
                    Vector to add
        """
    def add_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise addition of a scalar to a vector
        
                Add a number to each element of the vector.
        
                Parameters
                ----------
                value : float
                    Number to add
        """
    def binary_search(self, value: typing.SupportsFloat | typing.SupportsIndex) -> int:
        """
                In a sorted (ascending) vector find the element closest to the given value
        
                Bisection is used to find the element.
        
                If two (neighboring) elements with different values both minimize
                fabs(sorted[index] - key) the index of the larger element is returned.
        
                If the vector contains identical elements that minimize
                fabs(sorted[index] - key) then it is undefined which element has its index
                returned.
        
                Use cpl.core.Vector.sort(cpl.core.Sort.ASCENDING) before calling this function
                to ensure the vector is sorted correctly
        
                Parameters
                ----------
                value : float
                    Value to find
        
                Returns
                -------
                int
                  The index that minimizes fabs(sorted[index] - value)
        
                Raises
                ------
                cpl.core.IllegalInputError
                  If the vector is not correctly sorted in ascending order.
        """
    def copy(self) -> Vector:
        """
                Copy the contents of the Vector into a new Vector objects.
        
                Vectors can also be copied by passing a Vector to the Vector
                constructor.
        
                Returns
                -------
                cpl.core.Vector
                    New Vector containing a copy of the contents of the original.
        
                See Also
                --------
                cpl.core.Vector: Class for ordered sequences of numbers.
        """
    def cycle(self, shift: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Perform a cyclic shift to the right of the elements of the vector
        
                A shift of +1 will move the last element to the first, a shift of -1 will
                move the first element to the last, a zero-shift will perform a copy (or
                do nothing in case of an in-place operation).
        
                A non-integer shift will perform the shift in the Fourier domain. Large
                discontinuities in the vector to shift will thus lead to FFT artifacts
                around each discontinuity.
        
                Parameters
                ----------
                shift : float
                    The number of positions to cyclic right-shift
        
                Raises
                ------
                cpl.core.UnsupportedModeError
                    if the shift is non-integer and FFTW is unavailable
        """
    def divide(self, other: Vector) -> None:
        """
                Divide the calling vector by another vector, element-wise
        
                If an element in vector `other` is zero, a cpl.core.DivisionByZeroError is thrown.
        
                Parameters
                ----------
                other : cpl.core.Vector
                    Vector to divide by
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if the calling vector and `other` have different sizes
                cpl.core.DivisionByZeroError
                    if `other` contains an element equal to zero.
        """
    def divide_scalar(self, divisor: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise division of a vector with a scalar
        
                Divide each element of the vector with a number.
        
                Parameters
                ----------
                divisor : float
                    Non-zero number to divide with
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a vector contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump vector contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send vector contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the vector contents.
        
                Notes
                -----
                In principle a vector can be saved using :py:meth:`dump` re-read using :py:meth:`read`.
                This will however introduce significant precision loss due to the limited
                accuracy of the ASCII representation.
        """
    def exponential(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the exponential of all vector elements.
        
                If the base is zero all vector elements must be positive and if the base is
                negative all vector elements must be integer.
        
                Parameters
                ----------
                base : float
                    Exponential base.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    base and v are not as requested
                cpl.core.DivisionByZeroError
                    if one of the values is negative or 0
        """
    def extract(self, istart: typing.SupportsInt | typing.SupportsIndex, istop: typing.SupportsInt | typing.SupportsIndex, istep: typing.SupportsInt | typing.SupportsIndex = 1) -> Vector:
        """
                Extract a sub-vector from a vector
        
                Parameters
                ----------
                istart : int
                    Start index (from 0 to number of elements - 1), must be less than istop
                istop : int
                    Stop  index (from 0 to number of elements - 1), must be greater than istart
                istep : int, optional
                    Extract every step element (Currently does not support any value other than the default)
        
                Returns
                -------
                cpl.core.Vector
                    New sub-vector with the values of the requested range
        
                Raises
                ------
                cpl.core.AccessOutOfRangeError
                    if istart is less than 0 or istop is greater than the size of the vector
                cpl.core.IllegalInputError
                    if istep is not 1, or istart is not less than istop
        
                Notes
                -----
                istep only supporting a value of 1 is to be fixed, as is allowing istop to be greater than istart.
        """
    def fill(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Fill the Vector with a given value
        
                Parameters
                ----------
                val : float
                    Value used to fill the cpl_vector
        """
    def filter_lowpass_create(self, filter_type: Vector.LowPass, hw: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        """
                Apply a low-pass filter to a vector
        
                This type of low-pass filtering consists in a convolution with a given
                kernel. The chosen filter type determines the kind of kernel to apply for
                convolution.
        
                Supported kernels are cpl.core.Vector.LowPass.LINEAR and cpl.core.Vector.LowPass.GAUSSIAN.
        
                In the case of cpl.core.Vector.LowPass.GAUSSIAN, the gaussian sigma used is
                1/sqrt(2). As this function is not meant to be general and cover all
                possible cases, this sigma is hardcoded and cannot be changed.
        
                The returned signal has exactly as many samples as the input signal.
        
                Parameters
                ----------
                filter_type : cpl.core.Vector.LowPass
                    Type of filter to use
                hw : int
                    Filter half-width
        
                Returns
                -------
                cpl.core.Vector
                    The resulting signal
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if filter_type is not supported or if hw is bigger than half the vector size
        """
    def filter_median_create(self, hw: typing.SupportsInt | typing.SupportsIndex) -> Vector:
        """
                Apply a 1D median filter of given half-width to a Vector
        
                This function applies a median smoothing to the caller Vector and returns a
                new Vector containing a median-smoothed version of the input.
        
                The returned Vector has exactly as many samples as the input one. The
                outermost hw values are copies of the input, each of the others is set to
                the median of its surrounding 1 + 2 * hw values.
        
                For historical reasons twice the half-width is allowed to equal the
                Vector length, although in this case the returned Vector is simply a
                duplicate of the input one.
        
                If different processing of the outer values is needed or if a more general
                kernel is needed, then :py:meth:`cpl.core.Image.filter_mask` can be called instead with
                cpl.core.Filter.MEDIAN and the 1D-image input wrapped around self.
        
                Parameters
                ----------
                hw : int
                    Filter half-width
        
                Returns
                -------
                cpl.core.Vector
                    The filtered vector.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if hw is negative or bigger than half the vector
        """
    def logarithm(self, base: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the element-wise logarithm.
        
                The base and all the vector elements must be positive and the base must be
                different from 1.
        
                Parameters
                ----------
                base : float
                    Logarithm base.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if base is negative or zero or if one of the vector values is negative or zero
                cpl.core.DivisionByZeroError
                    if a division by zero occurs
        """
    def max(self) -> float:
        """
                Get the maximum of the vector
        
                Returns
                -------
                float
                    The maximum value of the vector
        """
    def maxpos(self) -> int:
        """
                Get the index of the maximum element of the vector
        
                Returns
                -------
                int
                    The index (0 for first) of the maximum value
        """
    def mean(self) -> float:
        """
                Get the mean of the elements of the vector
        
                Returns
                -------
                float
                    The mean of the elements value of the vector
        """
    def median(self) -> float:
        """
                Get the median of the elements of the vector
        
                Returns
                -------
                float
                    The median of the elements value of the vector
        """
    def min(self) -> float:
        """
                Get the minimum of the vector
        
                Returns
                -------
                float
                    The minimum value of the vector
        """
    def minpos(self) -> int:
        """
                Get the index of the minimum element of the vector
        
                Returns
                -------
                int
                    The index (0 for first) of the minimum value
        """
    def multiply(self, other: Vector) -> None:
        """
                Multiply another vector with the calling vector, component-wise
        
                Parameters
                ----------
                other : cpl.core.Vector
                    Vector to multiply with
        """
    def multiply_scalar(self, factor: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise multiplication of a vector with a scalar
        
                Multiply each element of the vector with a number.
        
                Parameters
                ----------
                factor : float
                    Number to multiply with
        """
    def power(self, exponent: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Compute the power of all vector elements.
        
                If the exponent is negative all vector elements must be non-zero and if
                the exponent is non-integer all vector elements must be non-negative.
        
                Parameters
                ----------
                exponent : float
                    Constant exponent.
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if v and exponent are not as requested
                cpl.core.DivisionByZeroError
                    if one of the values is 0
        
                Notes
                -----
                Following the behaviour of C99 pow() function, this function sets 0^0 = 1.
        """
    def product(self, other: Vector) -> float:
        """
                Compute the vector dot product of the caller vector and `other`
        
                Parameters
                ----------
                other : cpl.core.Vector
                    Another vector of the same size
        
                Returns
                -------
                float
                    The (non-negative) product
        
                Raises
                ------
                cpl.core.IncompatibleInputError
                    if `other` has a different size from the calling vector
        """
    def save(self, filename: str, type: Type, plist: cpl.core.PropertyList | None, mode: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                Save a vector to a FITS file
        
                This function saves a vector to a FITS file (``NAXIS`` = 1). If a property list
                is provided, it is written to the named file before the pixels are written.
        
                If the image is not provided, the created file will only contain the
                primary header. This can be useful to create multiple extension files.
        
                The type used in the file can be one of:
                cpl.core.Type.UCHAR  (8 bit unsigned),
                cpl.core.Type.SHORT  (16 bit signed),
                cpl.core.Type.USHORT (16 bit unsigned),
                cpl.core.Type.INT    (32 bit signed),
                cpl.core.Type.FLOAT  (32 bit floating point), or
                cpl.core.Type.DOUBLE (64 bit floating point).
                Use cpl.core.Type.DOUBLE when no loss of information is required.
        
                Supported output modes are cpl.core.IO.CREATE (create a new file) and
                cpl.core.IO.EXTEND  (append to an existing file)
        
                If you are in append mode, make sure that the file has writing
                permissions. You may have problems if you create a file in your
                application and append something to it with the umask set to 222. In
                this case, the file created by your application would not be writable,
                and the append would fail.
        
                Parameters
                ----------
                filename : cpl.core.str
                    Name of the file to write
                type : cpl.core.Type
                    The type used to represent the data in the file
                plist : cpl.core.Propertylist
                    Property list for the output header or NULL
                mode : cpl.core.IO
                    The desired output options. Can combine with bitwise or (e.g. cpl.core.IO.CREATE|cpl.core.IO.GZIP)
        
                Raises
                ------
                cpl.core.IllegalInputError
                    if the type or the mode is not supported
                cpl.core.FileNotCreatedError
                    if the output file cannot be created
                cpl.core.FileIOError
                    if the data cannot be written to the file
        """
    def sort(self, reverse: bool = False) -> None:
        """
                Sort the Vector in place.
        
                The values are sorted in either ascending or descending order. The sorting
                is done in place, modifying the Vector.
        
                Parameters
                ----------
                reverse : bool, default False
                    If `True` values will be sorted in descending order, otherwise they will
                    be sorted in ascending order.
        
                See Also
                --------
                cpl.core.Vector.sorted : Return a sorted copy of the Vector.
        
                Notes
                -----
                If two members compare as equal their order in the sorted Vector is undefined.
        """
    def sorted(self, reverse: bool = False) -> Vector:
        """
                Return a sorted copy of the Vector.
        
                The values are sorted in either ascending of descending order. The result
                is returned in a new `cpl.core.Vector`, the original is not modified.
        
                Parameters
                ----------
                reverse : bool, default False
                    If `True` values will be sorted in descending order, otherwise they will
                    be sorted in ascending order.
        
                See Also
                --------
                cpl.core.Vector.sort : Sort the Vector in place.
        
                Notes
                -----
                If two members compare as equal their order in the sorted Vector is undefined.
        """
    def sqrt(self) -> None:
        """
                Compute the sqrt of a Vector
        
                The sqrt of each data element is computed and modified.
        
                Raises
                ------
                cpl.core.IllegalInputError
                  An element is negative
        """
    def stdev(self) -> float:
        """
                Get the standard deviation of the elements of the vector
        
                Returns
                -------
                float
                    The standard deviation of the elements value of the vector
        """
    def subtract(self, other: Vector) -> None:
        """
                Subtract a cpl.core.Vector from self
        
                The other vector must have the same size as the calling vector
        
                Parameters
                ----------
                other : cpl.core.Vector
                    Vector to subtract
        """
    def subtract_scalar(self, value: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
                Elementwise subtraction of a scalar to a vector
        
                Subtract a number to each element of the vector.
        
                Parameters
                ----------
                value : float
                    Number to subtract
        """
    def sum(self) -> float:
        """
                Get the sum of the elements of the vector
        
                Returns
                -------
                float
                    The sum of the elements value of the vector
        """
    @property
    def size(self) -> int:
        """
        Number of elements in the vector. Is resizable
        """
    @size.setter
    def size(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class VectorIterator:
    def __iter__(self) -> VectorIterator:
        ...
    def __next__(self) -> float:
        ...
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
class _Mask1D:
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def load(filename: str, extension: typing.SupportsInt | typing.SupportsIndex = 0, plane: typing.SupportsInt | typing.SupportsIndex = 0, window: tuple = None) -> _Mask1D:
        ...
    def __and__(self, arg0: _Mask1D) -> _Mask1D:
        ...
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getstate__(self) -> tuple:
        ...
    @typing.overload
    def __init__(self, arg0: ..., arg1: typing.SupportsFloat | typing.SupportsIndex, arg2: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, data: typing.Any = None) -> None:
        ...
    def __invert__(self) -> _Mask1D:
        ...
    def __or__(self, arg0: _Mask1D) -> _Mask1D:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def __xor__(self, arg0: _Mask1D) -> _Mask1D:
        ...
    def collapse_cols(self) -> _Mask1D:
        ...
    def collapse_rows(self) -> _Mask1D:
        ...
    def count(self, window: tuple = None) -> int:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', window: tuple | None = None, show: bool | None = True) -> str:
        """
                Dump the mask contents to a file, stdout or a string.
        
                This function is intended just for debugging. It prints the contents of a mask 
                to the file path specified by `filename`. 
                If a `filename` is not specified, output goes to stdout (unless `show` is False). 
                In both cases the contents are also returned as a string.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump mask contents to
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
                    Send mask contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the mask contents.
        """
    def extract(self, arg0: tuple) -> _Mask1D:
        ...
    def filter(self, arg0: _Mask1D, arg1: Filter, arg2: Border) -> _Mask1D:
        ...
    def flip(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def get_bytes(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> bytes:
        ...
    def insert(self, arg0: _Mask1D, arg1: typing.SupportsInt | typing.SupportsIndex, arg2: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def is_empty(self) -> bool:
        ...
    def move(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: list) -> _Mask1D:
        ...
    def rotate(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def save(self, arg0: str, arg1: ..., arg2: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def set_bytes(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: bytes) -> None:
        ...
    def shift(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def subsample(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> _Mask1D:
        ...
    def threshold_image(self, arg0: ..., arg1: typing.SupportsFloat | typing.SupportsIndex, arg2: typing.SupportsFloat | typing.SupportsIndex, arg3: bool) -> None:
        ...
    @property
    def height(self) -> int:
        ...
    @property
    def width(self) -> int:
        ...
class _TableColumn(collections.abc.MutableSequence):
    """
    
    Provides an accessor to table via column to allow 2d indexing, for example:
    
    tableColumn = table["columnName"]
    
    where tableColumn is an instance of _TableColumn
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset()
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    def __array__(self, dtype = None, copy = None):
        ...
    def __delitem__(self, index):
        ...
    def __getitem__(self, index):
        ...
    def __init__(self, table, column):
        ...
    def __len__(self):
        ...
    def __setitem__(self, index, value):
        ...
    def insert(self, index, value):
        ...
    @property
    def as_array(self):
        ...
class io:
    """
    I/O modes for file storage operations. See http://heasarc.nasa.gov/docs/software/fitsio/compression.html for compression mode details.
    
    Members:
    
      CREATE : Overwrite the file, if it already exists.
    
      EXTEND : Append a new extension to the file.
    
      APPEND : Append to the last data unit of the file.
    
      COMPRESS_GZIP : Use FITS tiled-image compression with GZIP algorithm.
    
      COMPRESS_RICE : Use FITS tiled-image compression with RICE algorithm.
    
      COMPRESS_HCOMPRESS : Use FITS tiled-image compression with HCOMPRESS algorithm.
    
      COMPRESS_PLIO : Use FITS tiled-image compression with PLIO algorithm.
    """
    APPEND: typing.ClassVar[io]  # value = <io.APPEND: 8>
    COMPRESS_GZIP: typing.ClassVar[io]  # value = <io.COMPRESS_GZIP: 16>
    COMPRESS_HCOMPRESS: typing.ClassVar[io]  # value = <io.COMPRESS_HCOMPRESS: 64>
    COMPRESS_PLIO: typing.ClassVar[io]  # value = <io.COMPRESS_PLIO: 128>
    COMPRESS_RICE: typing.ClassVar[io]  # value = <io.COMPRESS_RICE: 32>
    CREATE: typing.ClassVar[io]  # value = <io.CREATE: 2>
    EXTEND: typing.ClassVar[io]  # value = <io.EXTEND: 4>
    __members__: typing.ClassVar[dict[str, io]]  # value = {'CREATE': <io.CREATE: 2>, 'EXTEND': <io.EXTEND: 4>, 'APPEND': <io.APPEND: 8>, 'COMPRESS_GZIP': <io.COMPRESS_GZIP: 16>, 'COMPRESS_RICE': <io.COMPRESS_RICE: 32>, 'COMPRESS_HCOMPRESS': <io.COMPRESS_HCOMPRESS: 64>, 'COMPRESS_PLIO': <io.COMPRESS_PLIO: 128>}
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
    @typing.overload
    def __or__(self, other: typing.Any) -> typing.Any:
        ...
    @typing.overload
    def __or__(self, arg0: io) -> int:
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
