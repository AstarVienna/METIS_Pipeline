"""
CPL DFS submodule

  This module provides the necessary functionality to create data products
  which comply to the ESO Data Interface Control Document (DICD) standard.
  The utility functions provided here make sure that data products are
  written using the correct format and structure, and that all required
  date product keywords are present.

  The use of these utilities is mandatory for the data processing pipelines
  of the ESO VLT and ELT instruments, or any other software component which
  is intended to be operated by ESO for the purpose of generating data
  products to be stored in the ESO Archive or to be used for data quality
  control.
  
"""
from __future__ import annotations
import cpl.core
import cpl.ui
import typing
__all__: list[str] = ['save_image', 'save_imagelist', 'save_paf', 'save_propertylist', 'save_table', 'setup_product_header', 'sign_products', 'update_product_header']
def save_image(allframes: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, usedframes: cpl.ui.FrameSet, image: cpl.core.Image, recipe: str, applist: cpl.core.PropertyList, pipe_id: str, filename: str, header: typing.Any = None, inherit: typing.Any = None, remregexp: typing.Any = None) -> None:
    """
        Save an image as a DFS-compliant pipeline product.
    
        Parameters
        ----------
        allframes : cpl.ui.FrameSet
            The list of input frames for the recipe
        parlist : cpl.ui.ParameterList
            The list of input parameters
        usedframes : cpl.ui.FrameSet
            The list of raw/calibration frames used for this product
        image : cpl.core.Image
            The image to be saved
        recipe : str
            The recipe name
        applist : cpl.core.PropertyList
            Propertylist to append to primary header
        pipe_id : str
            Pipeline package (unique) identifier
        filename : str
            Filename of created product
        header : cpl.core.PropertyList, optional
            None, or filled with properties written to product header
        inherit : cpl.ui.Frame, optional
            None, or product frames inherit their header from this frame
        remregexp : str, optional
            Optional regexp of properties not to put in main header (may be None)
        
        Returns
        -------
        None
    
        Notes
        -----
        The optional regular expression to filter out properties `remregexp` may be None.
    
        The list of properties passed as `applist` must contain a string-property with key
        ``ESO PRO CATG`` (ESO hierarchical keyword). Its value must be the pipeline specific,
        unique data product classification tag.
    
        The pipeline identifier string `pipe_id` is composed of the pipeline package
        name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
    
        On success and only if `header` is not None, it will be emptied and then
        filled with the properties written to the primary header of the product
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    """
def save_imagelist(allframes: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, usedframes: cpl.ui.FrameSet, imagelist: cpl.core.ImageList, data_type: cpl.core.Type, recipe: str, applist: cpl.core.PropertyList, pipe_id: str, filename: str, header: typing.Any = None, inherit: typing.Any = None, remregexp: typing.Any = None) -> None:
    """
        Save an imagelist as a DFS-compliant pipeline product.
    
        The FITS header of the created product is created from the provided applist
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided remregexp.
    
        Parameters
        ----------
        allframes : cpl.ui.FrameSet
            The list of input frames for the recipe
        parlist : cpl.ui.ParameterList
            The list of input parameters
        usedframes : cpl.ui.FrameSet
            The list of raw/calibration frames used for this product
        imagelist : cpl.core.ImageList
            The imagelist to be saved
        data_type : Type
            The type used to represent the data in the file
        recipe : str
            The recipe name
        applist : cpl.core.PropertyList
            Propertylist to append to primary header
        pipe_id : str
            Pipeline package (unique) identifier
        filename : str
            Filename of created product
        header : cpl.core.PropertyList, optional
            None, or filled with properties written to product header
        inherit : cpl.ui.Frame, optional
            None, or product frames inherit their header from this frame
        remregexp : str, optional
            Optional regexp of properties not to put in main header (may be None)
    
        Returns
        -------
        None
    
        Notes
        -----
        The optional regular expression to filter out properties `remregexp` may be None.
    
        The list of properties passed as `applist` must contain a string-property with key
        ``ESO PRO CATG`` (ESO hierarchical keyword). Its value must be the pipeline specific,
        unique data product classification tag.
    
        The pipeline identifier string `pipe_id` is composed of the pipeline package
        name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by cpl_dfs_setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    """
def save_paf(instrume: str, recipe: str, paflist: cpl.core.PropertyList, filename: str) -> None:
    """
        Create a new PAF file
    
        Parameters
        ----------
        instrume : str
            Name of the instrument in capitals (NACO, VISIR, etc.)
        recipe : str
            Name of recipe
        paflist : cpl.core.PropertyList
            Propertylist to save
        filename : str
            Filename of created Parameter File
    
        Returns
        -------
        None
    """
def save_propertylist(allframes: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, usedframes: cpl.ui.FrameSet, recipe: str, applist: cpl.core.PropertyList, pipe_id: str, filename: str, header: typing.Any = None, inherit: typing.Any = None, remregexp: typing.Any = None) -> None:
    """
        Save a propertylist as a DFS-compliant pipeline product.
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    
        Parameters
        ----------
        allframes : cpl.ui.FrameSet
            The list of input frames for the recipe
        parlist : cpl.ui.ParameterList
            The list of input parameters
        usedframes : cpl.ui.FrameSet
            The list of raw/calibration frames used for this product
        recipe : str
            The recipe name
        applist : cpl.core.PropertyList
            Propertylist to append to primary header
        pipe_id : str
            Pipeline package (unique) identifier
        filename : str
            Filename of created product
        header : cpl.core.PropertyList, optional
            None, or filled with properties written to product header
        inherit : cpl.ui.Frame, optional
            None, or product frames inherit their header from this frame
        remregexp : str, optional
            Optional regexp of properties not to put in main header (may be None)
    
        Returns
        -------
        None
    
        Notes
        -----
        The optional regular expression to filter out properties `remregexp` may be None.
    
        The list of properties passed as `applist` must contain a string-property with key
        ``ESO PRO CATG`` (ESO hierarchical keyword). Its value must be the pipeline specific,
        unique data product classification tag.
    
        The pipeline identifier string `pipe_id` is composed of the pipeline package
        name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    
        The FITS data unit will be empty.
    """
def save_table(allframes: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, usedframes: cpl.ui.FrameSet, table: cpl.core.Table, recipe: str, applist: cpl.core.PropertyList, pipe_id: str, filename: str, header: typing.Any = None, tablelist: typing.Any = None, inherit: typing.Any = None, remregexp: typing.Any = None) -> None:
    """
        Save a table as a DFS-compliant pipeline product
    
        The FITS header of the created product is created from the provided applist
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided remregexp.
    
        Parameters
        ----------
        allframes : cpl.ui.FrameSet
            The list of input frames for the recipe
        parlist : cpl.ui.ParameterList
            The list of input parameters
        usedframes : cpl.ui.FrameSet
            The list of raw/calibration frames used for this product
        table : Table
            The table to be saved
        recipe : str
            The recipe name
        applist : cpl.core.PropertyList
            Propertylist to append to primary header
        pipe_id : str
            Pipeline package (unique) identifier
        filename : str
            Filename of created product
        header : cpl.core.PropertyList, optional
            None, or filled with properties written to product header
        inherit : cpl.ui.Frame, optional
            None, or product frames inherit their header from this frame
        remregexp : str, optional
            Optional regexp of properties not to put in main header (may be None)
        
        Returns
        -------
        None
    
        Notes
        -----
        The optional regular expression to filter out properties `remregexp` may be None.
    
        The list of properties passed as `applist` must contain a string-property with key
        ``ESO PRO CATG`` (ESO hierarchical keyword). Its value must be the pipeline specific,
        unique data product classification tag.
    
        The pipeline identifier string `pipe_id` is composed of the pipeline package
        name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
    
        The FITS header of the created product is created from the provided `applist`
        and the cards copied by setup_product_header(), with exception of
        the cards whose keys match the provided `remregexp`.
    """
def setup_product_header(header: cpl.core.PropertyList, product_frame: cpl.ui.Frame, framelist: cpl.ui.FrameSet, parlist: cpl.ui.ParameterList, recid: str, pipeline_id: str, dictionary_id: str, inherit_frame: typing.Any = None) -> None:
    """
        Add product keywords to a pipeline product property list.
    
        Parameters
        ----------
        header : cpl.core.PropertyList
            Property list where keywords must be written
        product_frame : Frame
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
        This function checks the `header` associated to a pipeline product,
        to ensure that it is DICB compliant. In particular, this function does
        the following:
    
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
    
        2. Copy to `header`, if they are present, the following primary
           FITS keywords from the first input frame in the `framelist`:
           ``ORIGIN``, ``TELESCOPE``, ``INSTRUME``, ``OBJECT``, ``RA``,
           ``DEC``, ``EPOCH``, ``EQUINOX``, ``RADESYS``, ``DATE-OBS``,
           ``MJD-OBS``, ``UTC``, ``LST``, ``PI-COI``, ``OBSERVER``. If those
           keywords are already present in the `header` property list, they
           are overwritten only in case they have the same type. If any of
           these keywords are present with an unexpected type, a warning is
           issued, but the keywords are copied anyway (provided that the
           above conditions are fulfilled), and no error is set.
    
        3. Copy all the ``HIERARCH ESO *`` keywords from the primary FITS header
           of the `inherit_frame` in `framelist`, with the exception of
           the ``HIERARCH ESO DPR *``, and of the ``HIERARCH ESO PRO *`` and
           ``HIERARCH ESO DRS *`` keywords if the `inherit_frame` is a calibration.
           If those keywords are already present in `header`, they are overwritten.
    
        4. If found, remove the ``HIERARCH ESO DPR *`` keywords from `header`.
    
        5. If found, remove the ``ARCFILE`` and ``ORIGFILE`` keywords from `header`.
    
        6. Add to `header` the following mandatory keywords from the PRO
           dictionary: ``PIPEFILE``, ``ESO PRO DID``, ``ESO PRO REC1 ID``,
           ``ESO PRO REC1 DRS ID``, ``ESO PRO REC1 PIPE ID``, and
           ``ESO PRO CATG``. If those keywords are already present in
           `header`, they are overwritten. The keyword ``ESO PRO CATG`` is
           always set identical to the tag in `product_frame`.
    
        7. Only if missing, add to `header` the following mandatory keywords
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
    
        8. Check the existence of the keyword ``ESO PRO DATANCOM`` in `header`. If
           this keyword is missing, one is added, with the value of the total
           number of raw input frames.
    
        9. Add to `header` the keywords ``ESO PRO REC1 RAW1 NAME``,
           ``ESO PRO REC1 RAW1 CATG``, ``ESO PRO REC1 CAL1 NAME``, ``ESO PRO REC1 CAL1 CATG``,
           to describe the content of the input set-of-frames.
    
        See the DICB PRO dictionary for details on the mentioned PRO keywords.
    
        Non-FITS files are handled as files with an empty FITS header.
        
        The pipeline identifier string `pipe_id` is composed of the pipeline package
        name and its version number in the form PACKAGE "/" PACKAGE_VERSION.
    """
def sign_products(frameset: cpl.ui.FrameSet, compute_md5: bool = True, compute_checksum: bool = True) -> None:
    """
        Update DFS and DICB required header information of frames.
    
        Parameters
        ----------
        frameset : cpl.ui.FrameSet
            The frameset from which the product frames are taken.
        compute_md5 : bool, default=True
            Boolean Flag to compute the ``DATAMD5`` hash and add to the product header
        compute_checksum : bool, default=True
            Flag to compute the standard FITS checksums
    
        Returns
        -------
        None
    
        Notes
        -----
        The function takes all frames marked as products from the input frameset.
    """
def update_product_header(frameset: cpl.ui.FrameSet) -> None:
    """
        Perform any DFS-compliancy required actions (``DATAMD5``/``PIPEFILE`` update).
    
        Parameters
        ----------
        frameset : cpl.ui.FrameSet
            The list of frames with FITS products created by the recipe
    
        Returns
        -------
        None
    
        Raises
        ------
        cpl.core.DataNotFoundError
            If the input framelist contains a frame of type product with a missing filename.
        cpl.core.BadFileFormatError
            If the input framelist contains a frame of type product without a FITS card with
            key ``DATAMD5``.
        cpl.core.BadFileFormatError
            If the The input framelist contains a frame of type product for which the FITS
            card with key ``DATAMD5`` could not be updated.
    
        Notes
        -----
        Each product frame must correspond to a FITS file created with a CPL FITS saving function.
    """
