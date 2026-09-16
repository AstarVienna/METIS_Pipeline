"""
CPL UI submodule
  
  This module provides the features to implement data processing modules 
  (recipes) which can be executed using the ESO data processing environments.
  It provides the plugin API needed to implement these modules as well as the
  data types to pass data and configuration parameters to these modules. 
  
"""
from __future__ import annotations
import abc as abc
import collections.abc
import typing
import cpl.core
__all__: list[str] = ['AbstractRecipe', 'CRecipe', 'Frame', 'FrameSet', 'Parameter', 'ParameterEnum', 'ParameterList', 'ParameterRange', 'ParameterValue', 'PyRecipe', 'RecipeNotFoundException', 'abc']
class AbstractRecipe(abc.ABC):
    """
    
    Abstract Base Class to be used by PyRecipe
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset({'run'})
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
    _author = None
    _copyright = None
    _description = None
    _email = None
    _name = None
    _synopsis = None
    _version = None
    @staticmethod
    def run(frameset, settings):
        ...
    @classmethod
    def __new__(cls):
        ...
    def __repr__(self):
        ...
    @property
    def author(self):
        ...
    @property
    def copyright(self):
        ...
    @property
    def description(self):
        ...
    @property
    def email(self):
        ...
    @property
    def name(self):
        ...
    @property
    def synopsis(self):
        ...
    @property
    def version(self):
        ...
class CRecipe:
    """
    
          Interface for initialising and executing compiled CPL recipes written in C. The default recipe dir is set
          on installation as the esopipes-plugin directory in the configured CPLDIR/lib directory. 
    
          Modifying any parameters stored in the the recipe's `parameters` property will not have any effect on execution. 
          Any settings from the default parameters must be passed to the settings parameter in the run method.
        
    """
    recipe_dir: typing.ClassVar[list] = ['/project/build/deps-linux-x86_64/install/lib/esopipes-plugins']
    recipes: typing.ClassVar[list] = list()
    description: str
    def __getstate__(self) -> tuple:
        ...
    def __init__(self, name: str) -> None:
        """
        Initialise the recipe with a given name.
        """
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def _execute_in_process(self, arg0: FrameSet, arg1: collections.abc.Mapping[str, tuple[typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex, bool]], arg2: typing.Any) -> None:
        ...
    def _run_in_process(self, input_frames: FrameSet, settings: collections.abc.Mapping[str, typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex] = {}) -> FrameSet:
        """
              Execute a recipe within the current process.
              Allows for multiple instances of Pyesorex to be launched
              from a Python multiprocessing pool (see Example).
        
              Intended for expert developers, rather than the casual user.
              Error handling is less robust than the standard run method.
        
              As the parameters property is just a copy, custom parameter settings should be set in this method call.
        
              Parameters
              ----------
              input_frames : cpl.ui.FrameSet
                Input FrameSet for the recipe to use as inputs
              settings : dict
                {parameter_name : value} pairs for configuring recipe parameters for execution.
              
              Returns
              -------
              cpl.ui.FrameSet
                A FrameSet of all frames with cpl.ui.Frame.FrameGroup.PRODUCT, indicating them as the recipe's output
        
              Raises
              ------
              RuntimeError
                if a recipe error occurs.
        
              Notes
              -----
              All files generated during execution are saved in ./products/(recipe_name)_(folder_no) relative to the current working directory.
              After running, these files will not be deleted by PyCPL if not deleted by the recipe binaries themselves and remain on disk for
              the user to manage themselves for their own uses.
              
              Example
              -------
              .. code-block:: python
        
                def run_me(self,path,recipe,param1,param2):
                   # change cwd to path
                   os.chdir(path)
                   # wait for all workers to have switched over to their folders...
                   time.sleep(5)
                   p = Pyesorex()
                   p.recipe = recipe
                   
                   # setup Pyesorex parameters
                   
                   # run Pyesorex
                   results = p.recipe._run_in_process(sof,p.recipe_parameters.as_dict())
               
                   # make sure to return the path to correct the results FrameSet
                   return (results, path)
                   
                # When collecting the results from the pool:
                # create an empty frameset to store all the results from the pool
                total_results = FrameSet()
                
                # run your pool...
                
                # handle the results
                fset, path = result
                
                # Since fset does not have absolute paths, make sure to set absolute paths for filenames
                # otherwise the returned FrameSet will be invalid and will raise an Exception
                for fr in fset:
                    total_results.append(Frame(str(path / fr.file),fr.tag))
        """
    def run(self, input_frames: FrameSet, settings: collections.abc.Mapping[str, typing.SupportsFloat | typing.SupportsIndex | bool | str | typing.SupportsInt | typing.SupportsIndex] = {}) -> FrameSet:
        """
              Execute a recipe. As the parameters property is just a copy, custom parameter settings should be set in this method call.  
        
              Parameters
              ----------
              input_frames : cpl.ui.FrameSet
                Input FrameSet for the recipe to use as inputs
              settings : dict
                {parameter_name : value} pairs for configuring recipe parameters for execution. 
              
              Returns
              -------
              cpl.ui.FrameSet
                A FrameSet of all frames with cpl.ui.Frame.FrameGroup.PRODUCT, indicating them as the recipe's output
        
              Raises
              ------
              RuntimeError
                if a recipe error occurs.
        
              Notes
              -----
              All files generated during execution are saved in ./products/(recipe_name)_(folder_no) relative to the current working directory.
              After running, these files will not be deleted by PyCPL if not deleted by the recipe binaries themselves and remain on disk for 
              the user to manage themselves for their own uses.
        """
    @property
    def author(self) -> str:
        """
        Name of the recipe's author
        """
    @author.setter
    def author(self, arg1: str) -> None:
        ...
    @property
    def copyright(self) -> str:
        """
        Recipe's license and copyright information. Must be compatible with that of CPL.
        """
    @copyright.setter
    def copyright(self, arg1: str) -> None:
        ...
    @property
    def email(self) -> str:
        """
        Author's contact information
        """
    @email.setter
    def email(self, arg1: str) -> None:
        ...
    @property
    def name(self) -> str:
        """
        Unique name of the Recipe
        """
    @name.setter
    def name(self, arg1: str) -> None:
        ...
    @property
    def parameters(self) -> ParameterList:
        """
        A list of recipe parameters and their defaults. This is a copy, not a reference.
        """
    @property
    def synopsis(self) -> str:
        """
        Detailed description of a plugin.
        """
    @synopsis.setter
    def synopsis(self, arg1: str) -> None:
        ...
    @property
    def version(self) -> str:
        """
        Recipe version number
        """
    @version.setter
    def version(self, arg1: str) -> None:
        ...
class Frame:
    """
    
         A frame is a container for descriptive attributes related to a data file. The attributes are related to a data file through the 
         file name member of the frame type. Among the attributes which may be assigned to a data file is an attribute identifying the 
         type of the data stored in the file (image or table data), a classification tag indicating the kind of data the file contains 
         and an attribute denoting to which group the data file belongs (raw, processed or calibration file). For processed data a 
         processing level indicates whether the product is an temporary, intermediate or final product.
      
    """
    class FrameGroup:
        """
        Members:
        
          NONE
        
          CALIB
        
          PRODUCT
        
          RAW
        """
        CALIB: typing.ClassVar[Frame.FrameGroup]  # value = <FrameGroup.CALIB: 2>
        NONE: typing.ClassVar[Frame.FrameGroup]  # value = <FrameGroup.NONE: 0>
        PRODUCT: typing.ClassVar[Frame.FrameGroup]  # value = <FrameGroup.PRODUCT: 3>
        RAW: typing.ClassVar[Frame.FrameGroup]  # value = <FrameGroup.RAW: 1>
        __members__: typing.ClassVar[dict[str, Frame.FrameGroup]]  # value = {'NONE': <FrameGroup.NONE: 0>, 'CALIB': <FrameGroup.CALIB: 2>, 'PRODUCT': <FrameGroup.PRODUCT: 3>, 'RAW': <FrameGroup.RAW: 1>}
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
    class FrameLevel:
        """
        Members:
        
          NONE
        
          TEMPORARY
        
          INTERMEDIATE
        
          FINAL
        """
        FINAL: typing.ClassVar[Frame.FrameLevel]  # value = <FrameLevel.FINAL: 3>
        INTERMEDIATE: typing.ClassVar[Frame.FrameLevel]  # value = <FrameLevel.INTERMEDIATE: 2>
        NONE: typing.ClassVar[Frame.FrameLevel]  # value = <FrameLevel.NONE: 0>
        TEMPORARY: typing.ClassVar[Frame.FrameLevel]  # value = <FrameLevel.TEMPORARY: 1>
        __members__: typing.ClassVar[dict[str, Frame.FrameLevel]]  # value = {'NONE': <FrameLevel.NONE: 0>, 'TEMPORARY': <FrameLevel.TEMPORARY: 1>, 'INTERMEDIATE': <FrameLevel.INTERMEDIATE: 2>, 'FINAL': <FrameLevel.FINAL: 3>}
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
    class FrameType:
        """
        Members:
        
          NONE
        
          MATRIX
        
          IMAGE
        
          PAF
        
          TABLE
        
          ANY
        """
        ANY: typing.ClassVar[Frame.FrameType]  # value = <FrameType.ANY: 32>
        IMAGE: typing.ClassVar[Frame.FrameType]  # value = <FrameType.IMAGE: 2>
        MATRIX: typing.ClassVar[Frame.FrameType]  # value = <FrameType.MATRIX: 4>
        NONE: typing.ClassVar[Frame.FrameType]  # value = <FrameType.NONE: 1>
        PAF: typing.ClassVar[Frame.FrameType]  # value = <FrameType.PAF: 16>
        TABLE: typing.ClassVar[Frame.FrameType]  # value = <FrameType.TABLE: 8>
        __members__: typing.ClassVar[dict[str, Frame.FrameType]]  # value = {'NONE': <FrameType.NONE: 1>, 'MATRIX': <FrameType.MATRIX: 4>, 'IMAGE': <FrameType.IMAGE: 2>, 'PAF': <FrameType.PAF: 16>, 'TABLE': <FrameType.TABLE: 8>, 'ANY': <FrameType.ANY: 32>}
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
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getstate__(self) -> tuple:
        ...
    def __init__(self, file: str, tag: str = '', group: Frame.FrameGroup = ..., level: Frame.FrameLevel = ..., frameType: Frame.FrameType = ...) -> None:
        """
                Container for descriptive attributes related to a data file. The attributes are related to a data file through the
                file name member of the frame type.
        
                Parameters
                ----------
                file : str
                  path of the data file
                group : cpl.ui.FrameGroup
                  The frame group data type
                level : cpl.ui.FrameLevel
                  The frame processing level
                type : cpl.ui.FrameType
                  The frame type data type
        
                Raises
                ------
                cpl.core.FileNotFoundError
                  if `file` cannot be found 
        """
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def __str__(self) -> str:
        ...
    def as_ccddata(self, **kwargs) -> typing.Any:
        """
        Wrapper function of astropy's astropy.nddata.read constructor to convert the frame to astropy CCDData object. Refer to the documentation of astropy.nddata.read for more details 
        """
    def as_hdulist(self, **kwargs) -> typing.Any:
        """
        Convenience function to convert the frame to astropy HDUList object. Any kwargs passed to this function is passed down to astropy.io.fits.open. Refer to the documentation of astropy.io.fits.open for more details 
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump the Frame contents to a file, stdout or a string.
                  
                This function is mainly intended for debug purposes.
        
                Parameters
                ----------
                filename : str, optional
                    file path to dump frame contents to
                mode : str, optional
                    File mode to save the file, default 'w' overwrites contents.
                show : bool, optional
                    Send frame contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the frame contents.
        """
    @property
    def file(self) -> str:
        """
        str: filename of the frame
        """
    @file.setter
    def file(self, arg1: str) -> None:
        ...
    @property
    def group(self) -> Frame.FrameGroup:
        """
        cpl.ui.FrameGroup : The frame group data type.
        """
    @group.setter
    def group(self, arg1: Frame.FrameGroup) -> None:
        ...
    @property
    def level(self) -> Frame.FrameLevel:
        """
        cpl.ui.FrameLevel : The frame processing level
        """
    @level.setter
    def level(self, arg1: Frame.FrameLevel) -> None:
        ...
    @property
    def tag(self) -> str:
        """
        str: Category tag for the frame
        """
    @tag.setter
    def tag(self, arg1: str) -> None:
        ...
    @property
    def type(self) -> Frame.FrameType:
        """
        cpl.ui.FrameType : The frame type data type.
        """
    @type.setter
    def type(self, arg1: Frame.FrameType) -> None:
        ...
class FrameSet:
    """
    
        Frames can be stored in a frame set and retrieved by index and sequential access. Frame sets can be created, filled and saved to a ‘set of frames’ file or loaded from such a file.
    
        Framesets are intended to be used for passing fits file information to and from recipes. 
    
        Frames are accessed by index or iteration.
        
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Frame:
        ...
    def __getstate__(self) -> tuple:
        ...
    @typing.overload
    def __init__(self) -> None:
        """
        create an empty frameset
        """
    @typing.overload
    def __init__(self, sof_filename: str) -> None:
        """
                Generate a frameset from a given sof file. 
        
                A sof file contains a list of the input data. This data is specified in an sof file (which is just a text file), where each 
                input file is specified with its associated classification and category. The format of each line in the sof file is as follows:
        
                full-path-to-file  classification
        
                An example file, for the mythological "ZIMOS" instrument, might look like this:
        
                /home/user/data/mos/ZIMOS.2003-12-26T01:05:06.233.fits  MOS_SCIENCE
                /home/user/data/mos/ZIMOS.2003-12-26T01:26:00.251.fits  MOS_SCIENCE
                /home/user/data/mos/ZIMOS.2003-12-26T01:47:04.050.fits  MOS_SCIENCE
                /home/user/data/cal/master_bias4.fits                   MASTER_BIAS
                /home/user/data/cal/grs_LR_red.3.tfits                  GRISM_TABLE
                /home/user/gasgano/extract_table2.fits                  EXTRACT_TABLE
                /home/user/data/cal/badpixel.3.tfits                    CCD_TABLE
        
                For an concrete example for a specific instrument, check the documentation for that instrument.
        
                Optionally, a third column may be provided. Permitted values are either RAW or CALIB. This is for when a recipe does not identify 
                the type of input file, but as all ESO recipes are required to do so, this column is typically not needed. 
                
                Parameters
                ----------
                sof_filename : str
                  filename of the sof file
                
                Raises
                ------
                cpl.core.FileIOError
                  if the sof file cannot be found
        """
    @typing.overload
    def __init__(self, frames: collections.abc.Iterable) -> None:
        """
                Generate a frameset object with an iterable of the cpl.ui.Frame objects
        
                Parameters
                ----------
                frames : iterable
                  iterable container with the frames to store in the frameset
        """
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, arg0: tuple) -> None:
        ...
    def __str__(self) -> str:
        ...
    def append(self, frame: Frame) -> None:
        """
                Insert a frame into the given frame set.
        
                The function adds the frame frame to the frame set using the
                frame's tag as key.
        
                Parameters
                ----------
                frame : cpl.ui.Frame
                    The frame to insert.
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump the FrameSet contents to a file, stdout or a string.
                  
                This function is mainly intended for debug purposes.
        
                Parameters
                ----------
                filename : str, optional
                    file path to dump frameset contents to
                mode : str, optional
                    File mode to save the file, default 'w' overwrites contents.
                show : bool, optional
                    Send frameset contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the frameset contents.
        """
    def sign_products(self, compute_md5: bool = True, compute_checksum: bool = True) -> None:
        """
            Update DFS and DICB required header information of frames in the frameset
        
            Parameters
            ----------
            compute_md5 : bool, optional
              Boolean Flag to compute the ``DATAMD5`` hash and add to the product header
            compute_checksum : bool, optional
              Flag to compute the standard FITS checksums
        
            Return
            ------
            None
        
            Notes
            -----
            The function takes all frames marked as products from the input frameset.
        """
    def update_product_header(self) -> None:
        """
            Perform any DFS-compliancy required actions (``DATAMD5``/``PIPEFILE`` update) on the
            frames in the framest
        
            Returns
            -------
            None
        
            Raises
            ------
            cpl.core.DataNotFoundError
              If the input framelist contains a frame of type
              product with a missing filename.
            cpl.core.BadFileFormatError
              If the input framelist contains a frame of type
              product without a FITS card with key ``DATAMD5`` could not be updated.
        
            Notes
            -----
            Each product frame must correspond to a FITS file created with a CPL
            FITS saving function.
        """
class Parameter:
    """
    
            Parameters provide a standard way to pass, for instance, command line information to
            different components of an application.
    
            The fundamental parts of a parameter are its name, a context to which it belongs (a
            specific component of an application for instance), its current value and a default
            value.
    
            The implementation supports three classes of parameters:
    
              - A plain value (cpl.ui.ParameterValue)
              - A range of values (cpl.ui.ParameterRange)
              - An enumeration value (cpl.ui.ParameterEnum)
    
            cpl.ui.Parameter is the base class for the three parameter classes.
    
            When a parameter is created it is created for a particular value type. The type of
            a parameter's current and default value may be:
    
              - cpl.core.Type.BOOL
              - cpl.core.Type.INT
              - cpl.core.Type.DOUBLE
              - cpl.core.Type.STRING
    
            These types are inferred upon Parameter creation.
    
            (NOTE: as of writing the validation of parameter values on assignment is not yet
            implemented in CPL. PyCPL does not intend to layer this feature over CPL and thus will
            not include validation until CPL itself does.)
    
        
    """
    class ParameterMode:
        """
        Members:
        
          CLI
        
          ENV
        
          CFG
        """
        CFG: typing.ClassVar[Parameter.ParameterMode]  # value = <ParameterMode.CFG: 4>
        CLI: typing.ClassVar[Parameter.ParameterMode]  # value = <ParameterMode.CLI: 1>
        ENV: typing.ClassVar[Parameter.ParameterMode]  # value = <ParameterMode.ENV: 2>
        __members__: typing.ClassVar[dict[str, Parameter.ParameterMode]]  # value = {'CLI': <ParameterMode.CLI: 1>, 'ENV': <ParameterMode.ENV: 2>, 'CFG': <ParameterMode.CFG: 4>}
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
    @property
    def context(self) -> str:
        """
        The context in which the parameter belongs to
        """
    @property
    def data_type(self) -> cpl.core.Type:
        """
        CPL data type of the parameter
        """
    @property
    def description(self) -> str:
        """
        comment or description describing the parameter
        """
    @property
    def help(self) -> str:
        """
        description on how the parameter is used and its effects
        """
    @property
    def name(self) -> str:
        """
        The read-only unique name of the parameter
        """
    @property
    def tag(self) -> str:
        """
        user definable tag
        """
    @tag.setter
    def tag(self, arg1: str) -> None:
        ...
class ParameterEnum(ParameterValue):
    """
    
            Enumeration parameter. On construction expects the default value, followed by the list of the possible enumeration
            values. Note that the default value must be a member of the list of possible enumeration.
            values.
    
            CPL data type is inferred on default value given.
    
            Inherits all properties in cpl.ui.ParameterValue and cpl.ui.Parameter
    
            Parameters
            ----------
            name : str
              The unique name of the parameter
            description :str
              comment or description describing the parameter
            context : str
              The context in which the parameter belongs to
            default : int, float or str
              The default and initialised value of the parameter
            alternatives : list of int, float or str
              list of enumeration alternatives, including the default value. Must be of the same type as default.
                 
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsInt | typing.SupportsIndex, alternatives: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsFloat | typing.SupportsIndex, alternatives: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: str, alternatives: collections.abc.Sequence[str]) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a parameter contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump parameter contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send parameter contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the parameter contents.
        """
    @property
    def alternatives(self) -> list[float | bool | str | int]:
        """
        possible enumeration alternatives value can be
        """
    @property
    def default(self) -> float | bool | str | int:
        ...
    @property
    def value(self) -> float | bool | str | int:
        ...
    @value.setter
    def value(self, arg1: typing.Any) -> None:
        ...
class ParameterList:
    """
    
            Container type for cpl.ui.Parameter.
    
            It provides a convenient way to pass a set of parameters to various functions e.g. recipes.
    
            Parameters are accessed by index or iteration.
        
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> Parameter:
        """
        Retrieve a parameter by index
        """
    @typing.overload
    def __getitem__(self, name: str) -> Parameter:
        """
        Retrieve a parameter by name
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Create an empty ParameterList
        """
    @typing.overload
    def __init__(self, params: collections.abc.Iterable) -> None:
        """
                Generate a ParameterList object with an iterable of the cpl.ui.Parameter objects
        
                Parameters
                ----------
                params : iterable
                  iterable container with the parameters to store in the ParameterList
        """
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def append(self, param: Parameter) -> None:
        """
                    Append a parameter to the end of a ParameterList.
        
                    Parameters
                    ----------
                    param : cpl.ui.Parameter
                      parameter to insert
        """
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a parameter list contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump parameter list contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send parameter list contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the parameter list contents.
        """
class ParameterRange(ParameterValue):
    """
    
            Range parameter. On construction expects the default value, followed by the minimum value and the maximum value.
            CPL data type is inferred on default value given.
    
            Inherits all properties in cpl.ui.ParameterValue and cpl.ui.Parameter
    
            Parameters
            ----------
            name : str
              The unique name of the parameter
            description :str
              comment or description describing the parameter
            context : str
              The context in which the parameter belongs to
            default : int or float
              The default and initialised value of the parameter
            min : int or float
              Minimum value of the parameter. Must be of the same data type as default.
            max : int or float
              Maximum value of the parameter. Must be of the same data type as default.
                 
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsInt | typing.SupportsIndex, min: typing.SupportsInt | typing.SupportsIndex, max: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsFloat | typing.SupportsIndex, min: typing.SupportsFloat | typing.SupportsIndex, max: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a parameter contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump parameter contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send parameter contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the parameter contents.
        """
    @property
    def default(self) -> float | bool | str | int:
        """
        Default value of the parameter
        """
    @property
    def max(self) -> float | bool | str | int:
        """
        Maximum value of the parameter
        """
    @property
    def min(self) -> float | bool | str | int:
        """
        Minimum value of the parameter
        """
    @property
    def value(self) -> float | bool | str | int:
        """
        Current value of the parameter
        """
    @value.setter
    def value(self, arg1: typing.Any) -> None:
        ...
class ParameterValue(Parameter):
    """
    
            Plain parameter value. Stores a single value with no boundaries. CPL data type is inferred on default value given.
    
            Inherits all properties in cpl.ui.Parameter
    
            Parameters
            ----------
            name : str
              The unique name of the parameter
            description :str
              comment or description describing the parameter
            context : str
              The context in which the parameter belongs to
            default : bool, int, float or str
              The default and initialised value of the parameter
                 
    """
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: typing.Any) -> bool:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: bool) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, name: str, description: str, context: str, default: str) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def dump(self, filename: str | None = '', mode: str | None = 'w', show: bool | None = True) -> str:
        """
                Dump a parameter contents to a file, stdout or a string.
        
                Each element is preceded by its index number (starting with 1!) and
                written on a single line.
        
                Comment lines start with the hash character.
        
                Parameters
                ----------
                filename : str, optional
                    File to dump parameter contents to
                mode : str, optional
                    Mode to open the file with. Defaults to "w" (write, overwriting the contents of the file if it already exists),
                    but can also be set to "a" (append, creating the file if it does not already exist or appending to the end of
                    it if it does).
                show : bool, optional
                    Send parameter contents to stdout. Defaults to True.
        
                Returns
                -------
                str 
                    Multiline string containing the dump of the parameter contents.
        """
    @property
    def cfg_alias(self) -> str:
        """
        named used to identify the parameter being set in a .cfg file
        """
    @cfg_alias.setter
    def cfg_alias(self, arg1: str) -> None:
        ...
    @property
    def cli_alias(self) -> str:
        """
        named used to identify the parameter being set as a the command line parameter
        """
    @cli_alias.setter
    def cli_alias(self, arg1: str) -> None:
        ...
    @property
    def context(self) -> str:
        ...
    @property
    def data_type(self) -> cpl.core.Type:
        """
        CPL data type of the parameter
        """
    @property
    def default(self) -> float | bool | str | int:
        """
        default value of the parameter
        """
    @property
    def description(self) -> str:
        ...
    @property
    def env_alias(self) -> str:
        """
        named used to identify the parameter being set as an environment variable
        """
    @env_alias.setter
    def env_alias(self, arg1: str) -> None:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def presence(self) -> bool:
        """
        flag to indicate if the parameter has been changed from its default
        """
    @presence.setter
    def presence(self, arg1: bool) -> None:
        ...
    @property
    def value(self) -> float | bool | str | int:
        """
        current value of the parameter
        """
    @value.setter
    def value(self, arg1: typing.Any) -> None:
        ...
class PyRecipe(AbstractRecipe):
    """
    
    PyRecipe base class for the implementation of custom Python recipes. 
    
    When inheriting this class the following members are expected to be overwitten:
    
    - _name
    - _author
    - _copyright
    - _description
    - _email
    - _synopsis
    - _version
    - run(frameset,settings)
    
    It is also recommended that new recipes include their own docstrings. New __init__ and __del__ methods 
    can be written to handle data before/after execution.
    """
    __abstractmethods__: typing.ClassVar[frozenset]  # value = frozenset({'run'})
    _abc_impl: typing.ClassVar[_abc._abc_data]  # value = <_abc._abc_data object>
class RecipeNotFoundException(Exception):
    pass
