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

import os
import subprocess
import pytest


@pytest.mark.config
class TestConfigured:
    """
    These tests are just a quick diagnosis of your environment.
    If they fail, most probably something is not set up correctly.
    # ToDo: Everything here is currently designed rather ad hoc.
    """

    def test_is_pyesorex_reachable(self):
        output = subprocess.run(['pyesorex', '--help'])
        assert output.returncode == 0, \
            "Could not run `pyesorex --help`, review your PATH"

    def test_can_we_find_the_whole_package(self):
        """
        If this fails, your Python is unable to find the package.
        Set PYTHONPATH to the root directory.
        # FIXME This is just a temporary workaround and should be solved properly instead.
        """
        from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl
        assert MetisDetDarkImpl.__name__ == 'MetisDetDarkImpl', \
            "Could not import the pipeline, review your PYTHONPATH"

    def test_can_we_import_at_least_raw_input(self):
        from pymetis.instruments.metis.inputs import RawInput
        assert RawInput.__name__ == 'RawInput', \
            "Could not import the pipeline, review your PYTHONPATH"

    @pytest.mark.external
    def test_is_simulated_data_present(self):
        """
        The data-driven tests replay an EDPS run over the output of METIS_Simulations;
        see `tests/data.py` for the two locations and how to override them.
        """
        from tests.data import SIM_DATA, EDPS_DATA, edps_jobs
        assert SIM_DATA.is_dir(), \
            f"No simulated data at {SIM_DATA}; set `METIS_SIM_DATA`"
        assert edps_jobs(), \
            f"No EDPS jobs under {EDPS_DATA}; run `edps -w metis.metis_wkf -i {SIM_DATA}` first"

    def test_is_recipe_dir_set(self):
        assert os.environ['PYESOREX_PLUGIN_DIR'] is not None, \
            "Environment variable `PYESOREX_PLUGIN_DIR` is not defined"
