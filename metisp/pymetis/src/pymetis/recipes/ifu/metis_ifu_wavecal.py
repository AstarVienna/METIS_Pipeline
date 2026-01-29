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
from typing import Literal

import cpl
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

import numpy as np

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.classes.qc import QcParameter, QcParameterSet
from pymetis.dataitems.wavecal import IfuWavecalRaw, IfuWavecal
from pymetis.classes.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import MasterDarkInput, RawInput, DistortionTableInput, OptionalInputMixin, \
    PersistenceMapInput, GainMapInput, LinearityInput
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header


class MetisIfuWavecalImpl(BandIfuMixin, DetectorIfuMixin, DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuWavecalRaw

        class MasterDarkInput(MasterDarkInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass

        class DistortionTableInput(DistortionTableInput):
            pass

    class ProductSet(PipelineProductSet):
        IfuWavecal = IfuWavecal

    class Qc(QcParameterSet):
        class NLines(QcParameter):
            _name_template = "QC IFU WAVECAL NLINES"
            _type = int
            _unit = "1"
            _default = None
            _description_template = "Number of detected laser lines; should be constant"

        class Rms(QcParameter):
            _name_template = "QC IFU WAVECAL RMS"
            _type = float
            _unit = "Å"
            _default = None
            _description_template = "Root mean square of the residuals of the wavelength calibration fit"

        class PeakCounts(QcParameter):
            _name_template = "QC IFU WAVECAL PEAK CNTS"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "Peak counts of the laser line"

        class LineWidth(QcParameter):
            _name_template = "QC IFU WAVECAL LINE WIDTH"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "FWHM of the laser line as measured by fitting a Gaussian profile to it"
            _comment = "This fulfils METIS-6073"

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> Hdu:
        det = rf'DET{detector:1d}'

        # load the raw wavecal images and combine
        raw_images = self.inputset.raw.use().load_data(extension=rf'{det}.DATA')
        combined_image = self.combine_images(raw_images, "average")
        imsize_x = combined_image.shape[1]
        imsize_y = combined_image.shape[0]

        # load the IFU distortion table
        distortion_table = self.inputset.distortion_table.load_data(extension=det)
        trace_list = self.inputset.distortion_table.item.read(distortion_table=distortion_table)

        # create static dummy wavecal image
        
        # static wavecal parameters for each detector
        wstart = np.array([3.5565, 3.5284, 3.5275, 3.5557]) # in microns
        wend =   np.array([3.5823, 3.5547, 3.5541, 3.5820]) # in microns
        wdelta = (wend - wstart) / (imsize_x - 1)
        # linear dispersion model
        wavelen = np.zeros(imsize_x, dtype=float)
        for i in range(imsize_x):
            wavelen[i] = wstart[detector - 1] + i * wdelta[detector - 1]

        # create wavecal image based on distortion table
        trc_h = 57 # total trace height is twice this [pix]
        wc_image = np.zeros(combined_image.shape, dtype=float)
        for i, trace in enumerate(trace_list):
            x_coords, y_coords = trace
            for x, y in zip(x_coords.astype(int), y_coords.astype(int)):
                if 0 <= x < imsize_x:
                    y_min = y - trc_h
                    y_max = y + trc_h
                    if 0 <= y_min < imsize_y and 0 <= y_max < imsize_y:
                        for yy in range(y_min, y_max + 1):
                            wc_image[yy, x] = wavelen[x]

        # self.correct_telluric()
        # self.apply_fluxcal()

        wc_image = cpl.core.Image(wc_image)

        header_wcal = create_dummy_header(EXTNAME=det)

        return Hdu(header_wcal, wc_image, name=det)

    def process(self) -> set[DataItem]:
        primary_header = cpl.core.PropertyList()

        product_wavecal = self.ProductSet.IfuWavecal(
            primary_header,
            *map(self._process_single_detector, [1, 2, 3, 4])
        )

        return {product_wavecal}


class MetisIfuWavecal(MetisRecipe):
    _name: str = "metis_ifu_wavecal"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Determine the relative spectral response function"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _algorithm = """Measure line locations (left and right edges, centroid by Gaussian fit).
        Compute deviation from optical models.
        Compute wavelength solution ξ(x, y, i), λ(x, y, i).
        Compute wavelength map."""
    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisIfuWavecalImpl
