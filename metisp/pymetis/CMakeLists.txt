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

set(pymodule "py${PACKAGE_NAME}")

set(_pip_args "--force --upgrade --no-deps")
if(NOT DEFINED ENV{VIRTUAL_ENV} AND NOT DEFINED ENV{CONDA_PREFIX})
  if(NOT $ENV{EUID} EQUAL 0) 
    set(_pip_args "--user" ${_pip_args})
  endif()
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/${pymodule}/setup.cfg.in"
  "${PROJECT_SOURCE_DIR}/${pymodule}/setup.cfg"
  IMMEDIATE @ONLY)

add_custom_target(${pymodule} ALL
  ${Python3_EXECUTABLE} -m pip wheel --isolated --no-deps --wheel-dir=${PROJECT_BINARY_DIR}/${pymodule}/dist ${PROJECT_SOURCE_DIR}/${pymodule})

# FIXME: For demonstration purposes this is good enough. A final solution
#        should likely resort to a cmake script to have some additional checks
#        and logic in place.
add_custom_target(${pymodule}-clean 
  rm -rf ${PROJECT_BINARY_DIR}/${pymodule}/dist
  COMMAND rm -rf ${PROJECT_SOURCE_DIR}/${pymodule}/build
  COMMAND rm -rf ${PROJECT_SOURCE_DIR}/${pymodule}/${pymodule}.egg-info)
  
add_custom_target(${pymodule}-uninstall
  ${Python3_EXECUTABLE} -m pip uninstall --yes ${pymodule})
add_dependencies(uninstall ${pymodule}-uninstall)

install(CODE "execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install ${_pip_args} ${PROJECT_BINARY_DIR}/${pymodule}/dist/${pymodule}-${PACKAGE_VERSION}-py3-none-any.whl)")
