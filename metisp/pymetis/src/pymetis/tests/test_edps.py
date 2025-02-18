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
import contextlib
import re
import pytest
import os
import subprocess


@pytest.fixture(autouse=True, scope="module")
def reset_edps():
    def inner():
        os.system("edps -shutdown")

        with contextlib.suppress(OSError):
            os.remove("/tmp/EDPS_Data/")

        os.makedirs("/tmp/EDPS_Data/", exist_ok=True)

    return inner


workflows = ['metis_lm_img_wkf', 'metis_ifu_wkf', 'metis_pupil_imaging_wkf', 'metis_wkf']


@pytest.mark.edps
class TestEDPS:
    @pytest.mark.parametrize('workflow_name', workflows)
    def test_does_edps_classify(self, workflow_name, reset_edps):
        reset_edps()
        output = subprocess.run(['edps', '-w', f'metis.{workflow_name}', '-i', os.path.expandvars('$SOF_DATA'), '-c'],
                       capture_output=True)
        message = str(output.stdout.decode('utf8'))
        assert output.returncode == 0, f"EDPS exited with a non-zero return code {output.returncode}"
        assert output.stderr == b'', f"EDPS exited with a non-empty stderr: {output.stderr}"
        assert re.findall('[eE]rror', message) == [], f"EDPS run resulted in errors: {message}"

    @pytest.mark.parametrize('workflow_name', workflows)
    def test_does_edps_run(self, workflow_name, reset_edps):
        reset_edps()
        output = subprocess.run(['edps', '-w', f'metis.{workflow_name}', '-i', os.path.expandvars('$SOF_DATA')],
                                capture_output=True)
        message = str(output.stdout.decode('utf8'))
        assert output.returncode == 0
        assert output.stderr == b''
        assert re.findall('[eE]rror', message) == [], f"EDPS run resulted in errors: {message}"
        assert re.findall('FAILED', message) == [], f"EDPS workflow failed: {message}"