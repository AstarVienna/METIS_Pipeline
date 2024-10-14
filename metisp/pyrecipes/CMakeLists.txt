# This file is part of the ESO METIS Pipeline
# Copyright (C) 2001-2024 European Southern Observatory
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set(recipe_SOURCES
    metis_det_lingain.py
    img/metis_lm_basic_reduction.py
    img/metis_n_img_flat.py
    img/metis_lm_img_flat.py
    ifu/metis_ifu_calibrate.py
    ifu/metis_ifu_distortion.py
    ifu/metis_ifu_telluric.py
    ifu/metis_ifu_reduce.py
    ifu/metis_ifu_postprocess.py
    metis_det_dark.py)

install(FILES ${recipe_SOURCES}
        DESTINATION ${PACKAGE_RECIPE_DIR}/${PACKAGE_NAME}-${PACKAGE_VERSION}
)
