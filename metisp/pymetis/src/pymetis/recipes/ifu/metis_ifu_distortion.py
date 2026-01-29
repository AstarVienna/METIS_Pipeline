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
from cpl.core import Msg
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue
from astropy.table import QTable
import numpy as np

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.classes.inputs.common import GainMapInput, LinearityInput
from pymetis.classes.mixins import DetectorIfuMixin
from pymetis.classes.qc import QcParameterSet, QcParameter
from pymetis.dataitems.distortion import IfuDistortionRaw, IfuDistortionTable, IfuDistortionReduced
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import RawInput, MasterDarkInput, OptionalInputMixin, PersistenceMapInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_table, create_dummy_header

# dummy distortion table
def create_distortion_table(ext: Literal[1, 2, 3, 4]) -> cpl.core.Table:
    qt = QTable()

    if ext == 1:
        qt['orders'] = \
            np.array([
                [-6.31962038e-14,  1.77283929e-10, -7.99042445e-08, -6.88339533e-05,  1.93598211e+02],
                [ 3.85651542e-14, -3.67396524e-10,  5.20257484e-07, -2.06244130e-04,  3.18096869e+02],
                [-2.62280453e-13,  1.08972295e-09, -1.43777564e-06, 5.95144411e-05,  4.43088401e+02],
                [-1.28091830e-12,  6.05080398e-09, -9.08344378e-06, 3.86883193e-03,  5.67650495e+02],
                [ 9.88312754e-13, -4.41408195e-09,  6.26848822e-06, -3.65939037e-03,  6.93792085e+02],
                [ 5.03333358e-13, -2.57467777e-09,  4.61742883e-06, -3.96844515e-03,  8.19879764e+02],
                [-2.00578723e-12,  7.72242452e-09, -9.08124820e-06, 2.37076938e-03,  9.45423923e+02],
                [ 9.17918222e-13, -3.44863039e-09,  4.39961194e-06, -3.37807678e-03,  1.07271918e+03],
                [ 3.18875925e-13, -1.29949166e-09,  2.23372191e-06, -2.99028566e-03,  1.19992632e+03],
                [ 5.09700887e-14,  3.20549624e-10, -7.62327076e-07, -1.27796249e-03,  1.32728863e+03],
                [ 4.26605665e-14, -1.49399478e-10,  8.89203394e-07, -3.02205941e-03,  1.45582094e+03],
                [ 1.58920585e-13, -7.42285904e-10,  1.75231865e-06, -3.64600570e-03,  1.58462661e+03],
                [-1.66148629e-12,  6.33467307e-09, -6.96614291e-06, -1.44428019e-04,  1.71349369e+03],
                [ 3.87213381e-13, -1.57810882e-09,  2.95807679e-06, -4.89957944e-03,  1.84388054e+03]],
                dtype='>f8')
    elif ext == 2:
        qt['orders'] = \
            np.array([
                [ 1.25000709e-12, -5.08414515e-09,  6.80979457e-06, -3.83156891e-03,  1.94804200e+02],
                [ 6.00529461e-13, -2.27302861e-09,  3.14208356e-06, -2.97959257e-03,  3.20129145e+02],
                [ 1.99654799e-13, -5.70807595e-10,  8.63374660e-07, -2.31570386e-03,  4.45558488e+02],
                [ 5.15023482e-13, -2.40498472e-09,  4.10120710e-06, -4.71963780e-03,  5.71900959e+02],
                [-6.34014143e-13,  2.45923480e-09, -2.41296111e-06, -2.31079616e-03,  6.98195644e+02],
                [-5.46212051e-13,  2.04081559e-09, -1.67433428e-06, -3.28148294e-03,  8.25091119e+02],
                [ 1.44490310e-15,  1.70292039e-10,  4.61800804e-07, -4.86056579e-03,  9.52747729e+02],
                [ 2.65192438e-13, -1.30194799e-09,  3.13730425e-06, -7.12925204e-03,  1.08086241e+03],
                [-3.53026062e-13,  1.08486189e-09,  3.36429235e-07, -6.75069238e-03,  1.20919199e+03],
                [-2.10186852e-13,  6.92202787e-10,  5.57479318e-07, -7.23726179e-03,  1.33797844e+03],
                [-2.36964867e-13,  1.02419512e-09, -9.81919165e-08, -7.51959779e-03,  1.46741783e+03],
                [ 1.50541067e-13, -6.96139589e-10,  2.52271499e-06, -9.65550558e-03,  1.59762957e+03],
                [ 1.98557338e-13, -1.27892154e-09,  4.04421928e-06, -1.15296422e-02,  1.72844202e+03],
                [ 3.36692124e-13, -1.38558083e-09,  3.44859828e-06, -1.15989696e-02,  1.85964632e+03]],
                dtype='>f8')
    elif ext == 3:
        qt['orders'] = \
            np.array([
                [ 1.49275253e-13, -5.62389687e-10,  3.48024097e-08, 6.20494617e-03,  2.61860065e+02],
                [ 1.57354526e-13, -6.60008071e-10,  3.92592794e-07, 5.50287863e-03,  3.82431600e+02],
                [-2.15713852e-13,  8.96107391e-10, -1.79622208e-06, 6.40084201e-03,  5.02959771e+02],
                [-9.87464555e-14,  5.85936482e-10, -1.60267029e-06, 6.15096058e-03,  6.23799031e+02],
                [-8.86624227e-14,  1.79244298e-10, -3.76074863e-07, 4.69152717e-03,  7.45254561e+02],
                [-3.10042007e-13,  1.27442295e-09, -2.11346452e-06, 5.43578953e-03,  8.66411883e+02],
                [-3.53867106e-13,  1.46509291e-09, -2.34494197e-06, 5.19593810e-03,  9.88059772e+02],
                [ 3.97643910e-13, -1.65245295e-09,  1.97023128e-06, 2.67387775e-03,  1.11019995e+03],
                [-4.53520675e-15, -3.06337429e-10,  6.44260698e-07, 2.68692114e-03,  1.23233654e+03],
                [-1.30651747e-13,  3.81403624e-10, -3.75200308e-07, 2.77370326e-03,  1.35470583e+03],
                [ 4.13042752e-13, -1.85959274e-09,  2.72372347e-06, 7.83108941e-04,  1.47763459e+03],
                [ 2.25228972e-13, -9.43907650e-10,  1.39147007e-06, 1.05863793e-03,  1.60047396e+03],
                [ 2.55078650e-13, -8.63780808e-10,  7.62797469e-07, 1.49236485e-03,  1.72347089e+03],
                [ 3.52858493e-13, -1.85952864e-09,  3.32466934e-06, -1.13545912e-03,  1.84760419e+03]],
                dtype='>f8')
    elif ext == 4:
        qt['orders'] = \
            np.array([
                [-2.93329742e-13,  9.92591063e-10, -1.77002805e-06, 3.69354686e-03,  2.72891222e+02],
                [ 3.13253287e-13, -8.15697343e-10, -3.72825222e-07, 3.43077092e-03,  3.92729007e+02],
                [-3.37889895e-13,  1.27976610e-09, -2.40074294e-06, 3.95009482e-03,  5.12797636e+02],
                [ 1.84906026e-13, -1.04028292e-09,  8.22534795e-07, 2.36788061e-03,  6.33250090e+02],
                [-7.27990066e-13,  3.40220721e-09, -5.60337615e-06, 5.20118115e-03,  7.53556123e+02],
                [ 5.68661764e-13, -2.39213570e-09,  2.39050531e-06, 1.40540581e-03,  8.74733668e+02],
                [-2.91980311e-13,  1.01801116e-09, -1.78853566e-06, 3.04060669e-03,  9.95533674e+02],
                [-1.10125531e-14,  2.53802424e-11, -6.90643911e-07, 2.44405012e-03,  1.11682388e+03],
                [-1.91021732e-13,  5.43527988e-10, -8.91674004e-07, 2.23022335e-03,  1.23829065e+03],
                [ 8.80632281e-13, -3.42620065e-09,  3.51615611e-06, 2.74519788e-04,  1.36029392e+03],
                [ 5.49928334e-13, -2.12217447e-09,  1.87753266e-06, 1.03579493e-03,  1.48209800e+03],
                [-1.37565025e-12,  5.41373478e-09, -7.11413286e-06, 4.41999927e-03,  1.60400909e+03],
                [ 1.45487617e-12, -5.97247724e-09,  7.31683048e-06, -1.80998199e-03,  1.72705691e+03],
                [-8.06210801e-13,  2.23697555e-09, -1.19012993e-06, 2.03550949e-04,  1.84998465e+03]],
                dtype='>f8')

    qt['column_range'] = \
        np.array([
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042],
            [   6, 2042]], dtype='>i8')  
      
    table = cpl.core.Table.empty(len(qt))
    table.new_column_array('orders', cpl.core.Type.DOUBLE, 5)
    table.new_column_array('column_range', cpl.core.Type.DOUBLE, 2)

    for i, row in enumerate(qt['orders']):
        table['orders', i] = np.array(row, dtype=float)
        table['column_range', i] = np.array(qt['column_range'][i], dtype=float)
    
    return table


class MetisIfuDistortionImpl(DetectorIfuMixin, DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class MasterDarkInput(MasterDarkInput):
            pass

        class PinholeTableInput(PinholeTableInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass


        class RawInput(RawInput):
            Item = IfuDistortionRaw

    class ProductSet(PipelineProductSet):
        DistortionTable = IfuDistortionTable
        DistortionReduced = IfuDistortionReduced

    class Qc(QcParameterSet):
        class Rms(QcParameter):
            _name_template = "QC IFU DISTORT RMS"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "Root mean square deviation between measured position and model"

        class Fwhm(QcParameter):
            _name_template = "QC IFU DISTORT FWHM"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "Measure FWHM of spots"

        class NSpots(QcParameter):
            _name_template = "QC IFU DISTORT NSPOTS"
            _type = int
            _unit = "1"
            _default = None
            _description_template = "Number of identified spots"

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        """
        Find the distortion coefficients for a single detector of the IFU.

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
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')
        combined_image = self.combine_images(raw_images, "average")

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))
        table = create_distortion_table(detector)

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        return {
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
        }

    def process(self) -> set[DataItem]:
        header_table = create_dummy_header()
        header_reduced = create_dummy_header()

        output = [self._process_single_detector(det) for det in [1, 2, 3, 4]]

        product_distortion = self.ProductSet.DistortionTable(
            header_table,
            *[out['TABLE'] for out in output],
        )
        product_distortion_reduced = self.ProductSet.DistortionReduced(
            header_reduced,
            *[out['IMAGE'] for out in output],
        )

        return {product_distortion, product_distortion_reduced}


class MetisIfuDistortion(MetisRecipe):
    _name = "metis_ifu_distortion"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Calculate table mapping pixel position to position on sky."""

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


    Impl = MetisIfuDistortionImpl
