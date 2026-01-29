"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

import cpl
from cpl.core import Msg
import numpy as np
from typing import Literal

from pyesorex.parameter import ParameterList, ParameterEnum, ParameterRange

# is this legal?
from astropy.table import QTable

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.classes.mixins import DetectorIfuMixin, BandIfuMixin
from pymetis.classes.qc import QcParameterSet, QcParameter
from pymetis.dataitems.badpixmap import BadPixMapIfu
from pymetis.dataitems.gainmap import GainMapIfu
from pymetis.dataitems.linearity.linearity import LinearityMapIfu
from pymetis.dataitems.distortion import IfuDistortionTable
from pymetis.dataitems.masterdark.masterdark import MasterDarkIfu
from pymetis.dataitems.masterflat import MasterFlatIfu
from pymetis.dataitems.rsrf import IfuRsrfRaw, IfuRsrfBackground, RsrfIfu
from pymetis.dataitems.raw.wcuoff import IfuWcuOffRaw
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import (BadPixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    WavecalInput, DistortionTableInput, LinearityInput, OptionalInputMixin,
                                    SinglePipelineInput, PersistenceMapInput)
from pymetis.utils.dummy import create_dummy_table, create_dummy_header

ma = np.ma
EXT = 4  # TODO: update to read multi-extension files and index by EXTNAME instead of integer


class MetisIfuRsrfImpl(DetectorIfuMixin, BandIfuMixin, DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuRsrfRaw

        class MasterDarkInput(MasterDarkInput):
            Item = MasterDarkIfu

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(OptionalInputMixin, GainMapInput):
            Item = GainMapIfu

        class LinearityInput(OptionalInputMixin, LinearityInput):
            Item = LinearityMapIfu

        class RsrfWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            Item = IfuWcuOffRaw

        # TBC: could this be replaced by the MASTER_DARK_IFU input?
        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            Item = BadPixMapIfu

        class DistortionTableInput(DistortionTableInput):
            Item = IfuDistortionTable

        # TBD: schema to be defined
        WavecalInput = WavecalInput

    class ProductSet(PipelineProductSet):
        RsrfBackground = IfuRsrfBackground
        MasterFlat = MasterFlatIfu
        RsrfIfu = RsrfIfu
        BadPixMap = BadPixMapIfu

    class Qc(QcParameterSet):
        class NBadPix(QcParameter):
            _name_template = "QC IFU RSRF NBADPIX"
            _type = int
            _unit = 'px'
            _default = None
            _description_template = "Number of bad pixels in the image mask"


    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        """
        Caclulate the RSRF for a single detector of the IFU.
        This function processes the input images, for each detector:
        - stack the wcu_off images into background_img
        - subtract background_img from raw_images and stack
        - calculate the black-body image from the wavecal_img
        - divide the stacked raw image by the black-body image
        - create the 1D RSRF curves from the flat image
        - create the bad pixel map from the master dark and update it

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4] # FixMe: Maybe make this fully customizable for any detector count?

        Returns
        -------
        dict[str, Hdu]
            Distortion coefficients for a single detector of the IFU, in a form of table and image
            # FixMe this does not make much sense but works for now [MB]
        """

        det = rf'{detector:1d}'

        # TODO: FUNC: basic raw processing of RSRF and WCU_OFF input frames:
        # - dark subtraction? (subtracting WCU_OFF frame should suffice?)
        # - gain / linearity correction?

        # load MASTER_DARK_IFU image and extract bad pixel map
        master_dark_img = self.inputset.master_dark.load_data(extension=rf'DET{det}.SCI')
        badpix_map = master_dark_img.bpm

        # load IFU trace definition file
        distortion_table = self.inputset.distortion_table.load_data(extension=rf'DET{det}')
        trace_list = self.inputset.distortion_table.item.read(distortion_table=distortion_table)

        # load wavelength calibration image
        wavecal_img = self.inputset.wavecal.load_data(extension=rf'DET{det}')

        # create master WCU_OFF background image
        Msg.info(self.__class__.__qualname__,
                    f"Creating WCU_OFF background image...")
        background_hdr = cpl.core.PropertyList()
        background_hdr.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        # self.inputset.background.frameset.dump() # debug
        bg_images = self.inputset.rsrf_wcu_off.use().load_data(extension=rf'DET{det}.DATA')
        background_img = self.combine_images(bg_images, self.stackmethod)

        # TODO: define usedframes?
        # TODO: Add product keywords - currently none defined in DRLD

        # create 2D flat image (raw images are added together)
        Msg.info(self.__class__.__qualname__,
                    f"Creating 2D spectral flat image...")
        spec_flat_hdr = cpl.core.PropertyList()
        spec_flat_hdr.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        # load RSRF_RAW images, subtract the background and stack them
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')
        raw_images.subtract_image(background_img)
        spec_flat_img = self.combine_images(raw_images, self.stackmethod)
        # propagate badpixel mask
        spec_flat_img.reject_from_mask(badpix_map)
        # TODO: propagate errors

        # obtain black-body temperature from first frame's header
        # NOTE: this assumes raw frames were grouped by BB temperature

        rsrf_raw_hdr = self.inputset.raw.items[0]['PRIMARY'].header
        # bb_temp = rsrf_raw_hdr['HIERARCH ESO INS WCU_BB SOURCETEMP'].value
        # TBD: read from header
        bb_temp = 800

        # create black-body image
        Msg.info(self.__class__.__qualname__,
                    f"Creating black-body image...")
        bb_img = create_ifu_blackbody_image(wavecal_img, bb_temp)

        # scale the BB image to the RSRF image before dividing
        raw_level = spec_flat_img.get_max()
        bb_level = bb_img.get_median()
        if bb_level == 0:
            Msg.warning(self.__class__.__qualname__,
                "Zero median value for blackbody image, skipping normalisation")
        else:
            bb_img.multiply_scalar(raw_level / bb_level)

        # divide the RSRF image by the black-body spectrum
        # (inherits the bb bad-pixel mask)
        spec_flat_img.divide(bb_img)
        # TODO: propagate errors

        # SKEL: Add QC keywords
        qc_badpix_count = spec_flat_img.count_rejected()
        spec_flat_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                "Number of bad pixels"
            )
        )

        # create bad pixel map product
        Msg.info(self.__class__.__qualname__,
                    f"Creating bad pixel map...")
        # TODO: FUNC: create updated bad pixel map
        badpix_hdr = cpl.core.PropertyList()
        badpix_hdr.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))
        # placeholder data for now -- bad-pixel map based spec_flat_img
        badpix_img = master_dark_img
        badpix_img.reject_from_mask(spec_flat_img.bpm)  # update with master_dark bpm
        # TODO: create QC1 parameters:
        # Add QC keywords
        badpix_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                "Number of bad pixels"
            )
        )

        # extract 1D RSRF curves
        Msg.info(self.__class__.__qualname__,
                    f"Extracting 1D RSRF curves...")
        rsrf_1d_list = extract_ifu_1d_spectra(spec_flat_img, trace_list,
                                              trace_width=self.extract_hwidth)

        # global normalisation of the 1D RSRF curves
        rsrf_med = np.zeros(len(rsrf_1d_list))
        for i in range(len(rsrf_1d_list)):
            # avoid calling cpl.core.Vector.median() as this sorts the vector!
            rsrf_med[i] = np.median(rsrf_1d_list[i])

        scale = np.mean(rsrf_med)

        if scale == 0:
            Msg.warning(self.__class__.__qualname__,
                "Zero average scale for RSRF curves, skipping normalisation")
        else:
            for rsrf_1d in rsrf_1d_list:
                rsrf_1d /= scale

        # create 1D RSRF product
        rsrf_hdr = cpl.core.PropertyList()
        rsrf_hdr.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))
        # TODO: FUNC: Add product keywords - currently none defined in DRLD
        table = QTable()
        for i, rsrf in enumerate(rsrf_1d_list, start=1):
            table[f'rsrf_{i}'] = rsrf
            table[f'rsrf_{i}'].name = f'rsrf_{i}'
            table[f'rsrf_{i}'].unit = 'normalised'
            table[f'rsrf_{i}'].description = '1D RSRF curve {i}'

        rsrf_table = cpl.core.Table(table)

        return {
            'BACKGROUND': Hdu(background_hdr, background_img, name=rf'DET{det}.DATA'),
            'MASTERFLAT': Hdu(spec_flat_hdr, spec_flat_img, name=rf'DET{det}.SCI'),
            'BADPIXMAP': Hdu(badpix_hdr, badpix_img, name=rf'DET{det}.SCI'),
            '1DRSRF': Hdu(rsrf_hdr, rsrf_table, name=rf'DET{det}.DATA'),
        }

    def process(self) -> set[DataItem]:

        # load parameters
        self.stackmethod = self.parameters[f"{self.name}.stacking.method"].value
        self.extract_hwidth = self.parameters[f"{self.name}.extract.hwidth"].value

        output = [self._process_single_detector(det) for det in [1, 2, 3, 4]]

        Msg.info(self.__class__.__qualname__,
                    f"Finalising recipe products...")

        # TBD: define final product primary headers, for now just dummy headers
        header_background = create_dummy_header()
        header_mflat = create_dummy_header()
        header_1drsrf = create_dummy_header()
        header_badpixmap = create_dummy_header()

        product_background = self.ProductRsrfBackground(
            header_background,
            *[out['BACKGROUND'] for out in output],
        )
        product_master_flat_ifu = self.ProductMasterFlat(
            header_mflat,
            *[out['MASTERFLAT'] for out in output],
        )
        product_rsrf_ifu = self.ProductRsrfIfu(
            header_1drsrf,
            *[out['1DRSRF'] for out in output],
        )
        product_badpix_map_ifu = self.ProductBadPixMap(
            header_badpixmap,
            *[out['BADPIXMAP'] for out in output],
        )

        return {product_background, product_master_flat_ifu, product_rsrf_ifu, product_badpix_map_ifu}


def create_ifu_blackbody_image(wavecal_img, bb_temp) -> cpl.core.Image:
    """
    Create a blackbody image from the RSRF image and the wavelength calibration image.
    """

    wdata = wavecal_img.as_array()

    # create a new image to hold the black-body spectrum
    # each pixel will hold the BB flux at the wavelength of that pixel
    bb_data = np.zeros_like(wdata)

    # create wavelength lookup table [im um]
    wlookup = np.unique(wdata)[1:]  # remove the first element (0)
    wavelengths = cpl.core.Vector(wlookup / 1e6)  # Wavelengths in meters

    # Calculate the black-body flux at each wavelength
    flux = cpl.drs.photom.fill_blackbody(cpl.drs.photom.Unit.LESS,  # output unit
                                         wavelengths,  # Wavelengths in meters
                                         cpl.drs.photom.Unit.LENGTH,  # input unit
                                         bb_temp)  # Temperature in Kelvin

    # convert lookup table to vector for binary search functionality
    wlookup = cpl.core.Vector(wlookup)

    # fill the BB data array with the flux values
    for i in range(bb_data.shape[0]):
        for j in range(bb_data.shape[1]):
            if wdata[i, j] > 0:  # only fill valid pixels
                # find the index of the closest wavelength in the lookup table
                # and assign the corresponding flux value
                bb_data[i, j] = flux[wlookup.binary_search(wdata[i, j])]

    # mask the zero values with the bad-pixel mask to avoid division by zero
    bb_img = cpl.core.Image(bb_data)
    bb_img.reject_value({0})

    return bb_img


def extract_ifu_1d_spectra(img, trace_list, trace_width: int = 10) -> list:
    """
    Extract 1D spectra from the given image using the provided list of
    spectral trace coordinates.

    Parameters:
    img : cpl.core.Image
        The image from which to extract the spectra.
    trace_list : list of tuples
        Each tuple contains two arrays: x-coordinates and y-coordinates
        of the spectral trace.
    trace_width : int
        The width of the trace to be used for extraction.

    Returns:
    list of 1d array
        A list of 1D spectra extracted from the image.
    """

    # copy data to np.masked_array for processing
    rej_mask = np.array(img.bpm)  # set bad pixels
    mdata = ma.masked_array(img.as_array(), mask=rej_mask)
    imwidth = img.width

    # TODO: check that traces are within the image bounds

    # create a list of 1D RSRF curves (width is the image width)
    rsrf_1d_list = []
    for trace in trace_list:
        x_arr, y_arr = trace[0], trace[1]
        rsrf_1d = np.zeros(imwidth, dtype=float)
        for i, x in enumerate(x_arr):
            yc = y_arr[i]
            rsrf_1d[int(x)] = mdata[int(yc - trace_width):int(yc + trace_width), int(x)].mean()
        rsrf_1d_list.append(rsrf_1d)

    return rsrf_1d_list


class MetisIfuRsrf(MetisRecipe):
    _name: str = "metis_ifu_rsrf"
    _version: str = "0.1"
    _author: str = "Janus Brink, A*"
    _email: str = "janus.brink27@gmail.com"
    _synopsis: str = "Determine the relative spectral response function for the IFU detector."

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """Average / median stack WCU_OFF images to create background image
        Subtract background image from individual RSRF RAW frames
        Stack the RSRF RAW frames
        TBC: subtract master_dark from above frames first?
        TBC: apply gain / linearity corrections to above frames?
        TBC: obtain bad pixel map from master_dark?
        Create continuum image by mapping Planck spectrum at Tlamp to wavelength image
        Scale continuum image to match the RSRF image
        Divide stacked RAW frame by continuum image and save as MASTER_FLAT_IFU (2D RSRF)
        Average in spatial direction for each spectral trace to obtain
            relative response function (1D RSRF) - table with 1D spectra
        Normalise the set of 1D spectra to a common level before saving as RSRF_IFU
        Create bad pixel map from the master flat and update with locations
            of zero values in the continuum image - save as BADPIX_MAP_IFU"""

    parameters = ParameterList([
        # --stacking.method
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("average", "median"),
            cli_alias="stacking.method",
        ),
        # --extract.hwidth
        ParameterRange(
            name=f"{_name}.extract.hwidth",
            context=_name,
            description="Half width of trace for 1D RSRF extraction [pix]",
            cli_alias="extract.hwidth",
            default=20,
            min=1,
            max=30,
        ),
    ])

    Impl: type[MetisRecipeImpl] = MetisIfuRsrfImpl
