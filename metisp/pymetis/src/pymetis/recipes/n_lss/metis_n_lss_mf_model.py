"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.mixins import BandNMixin
from pymetis.classes.prefab.lss.mf_model import MetisLssMfModelImpl
from pymetis.classes.recipes import MetisRecipe


class MetisNLssMfModelImpl(MetisLssMfModelImpl):
    class InputSet(BandNMixin, MetisLssMfModelImpl.InputSet):
        pass


class MetisNLssMfModel(MetisRecipe):
    _name: str = "metis_n_lss_mf_model"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Calculation of molecfit model"
    _description: str = """\
    Calculation of molecfit model

    Inputs
        N_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object
        N_LSS_STD_1D: Coadded, wavelength calibrated, collapsed 1D spectrum of the standard star
        LSF_KERNEL:         LSF Kernel file
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        ATMP_PROFILE:       Atmospheric input profile

     Matched Keywords
        DRS.SLIT    ***TBChecked***

    Outputs
        MF_BEST_FIT_TAB: Table with best-fit parameters
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD***"""

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO: Implement real parameters
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}parameter1",
            context=_name,
            description="Description of parameter 1",
            default="value1",
            alternatives=("value2", "value1"),
        ),
    ])
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    implementation_class = MetisNLssMfModelImpl
