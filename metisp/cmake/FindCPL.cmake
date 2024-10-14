# This file is part of the ESO Common Pipeline Library
# Copyright (C) 2001-2022 European Southern Observatory
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

#[=======================================================================[.rst:
FindCPL
-------

Find the Common Pipeline Library headers and libraries. The module can be
configured using the following variables:

    CPL_ROOT:    Location where CPL is installed

or the corresponding environment variable CPL_ROOT.

The module also takes into account the environment variable CPLDIR.
If it is set it should point to the location of the CPL installation.
Using the environment variable is equivalent to setting the option
CPL_ROOT. However, if both, CPL_ROOT and CPLDIR, are set the CPL_ROOT
variable setting takes precedence.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``CPL::cplcore``
  The cplcore library component.

``CPL::cplui``
  The cplui library component.

``CPL::cpldfs``
  The cpldfs library component.

``CPL::cpldrs``
  The cpldrs library component.

``CPL::cplgasgano``
  The cplgasgano library component.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``CPL_FOUND``
  "True" if both, CPL header files and the CPL libraries were found.

``CPL_INCLUDE_DIR``
  Path where the CPL header files are located.

``CPL_CPLCORE_LIBRARY``
  Path where the cplcore library is located.

``CPL_CPLUI_LIBRARY``
  Path where the cplui library is located.

``CPL_CPLDFS_LIBRARY``
  Path where the cpldfs library is located.

``CPL_CPLDRS_LIBRARY``
  Path where the cpldrs library is located.

``CPL_CPLGASGANO_LIBRARY``
  Path where the cplgasgano library is located.

``CPL_VERSION``
  Full version string of the CPL libraries.

``CPL_VERSION_MAJOR``
  Major version number of the CPL libraries.

``CPL_VERSION_MINOR``
  Minor version number of the CPL libraries.

``CPL_VERSION_PATCH``
  Patch version number of the CPL libraries.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CPL_INCLUDE_DIR``
  Path where the CPL header files are located.

``CPL_CPLCORE_LIBRARY``
  Path where the cplcore library is located.

``CPL_CPLUI_LIBRARY``
  Path where the cplui library is located.

``CPL_CPLDFS_LIBRARY``
  Path where the cpldfs library is located.

``CPL_CPLDRS_LIBRARY``
  Path where the cpldrs library is located.

``CPL_CPLGASGANO_LIBRARY``
  Path where the cplgasgano library is located.
#]=======================================================================]

cmake_policy(VERSION 3.12)

include(FindPackageHandleStandardArgs)

if(NOT PKG_CONFIG_FOUND)
    find_package(PkgConfig QUIET)
endif()

# For backwards compatibility also honor the CPLDIR environment variable.
if(NOT CPL_ROOT AND (NOT "$ENV{CPLDIR}" STREQUAL ""))
    set(CPL_ROOT "$ENV{CPLDIR}")
endif()

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_CPL QUIET IMPORTED_TARGET GLOBAL
                      cplcore
                      cplui
                      cpldfs
                      cpldrs
                      cplgasgano)
    if(PC_CPL_FOUND)
        set(CPL_VERSION ${PC_CPL_VERSION})
    endif()
endif()

# Search for the header files
find_path(CPL_INCLUDE_DIR
          NAMES cpl.h
          PATH_SUFFIXES cpl
          HINTS ${CPL_ROOT}/include ${PC_CPL_INCLUDE_DIRS})
mark_as_advanced(CPL_INCLUDE_DIR)

# Search for the component libraries. Also look into the 'lib64'
# directoy when searching for the libraries.
set(FIND_LIBRARY_USE_LIB64_PATHS True)
find_library(CPL_CPLCORE_LIBRARY
             NAMES cplcore
             HINTS ${CPL_ROOT}/lib ${PC_CPL_LIBRARY_DIRS})
mark_as_advanced(CPL_CPLCORE_LIBRARY)
if(CPL_CPLCORE_LIBRARY)
    set(CPL_cplcore_FOUND True)
endif()

find_library(CPL_CPLUI_LIBRARY
             NAMES cplui
             HINTS ${CPL_ROOT}/lib ${PC_CPL_LIBRARY_DIRS})
mark_as_advanced(CPL_CPLUI_LIBRARY)
if(CPL_CPLUI_LIBRARY)
    set(CPL_cplui_FOUND True)
endif()

find_library(CPL_CPLDFS_LIBRARY
             NAMES cpldfs
             HINTS ${CPL_ROOT}/lib ${PC_CPL_LIBRARY_DIRS})
mark_as_advanced(CPL_CPLDFS_LIBRARY)
if(CPL_CPLDFS_LIBRARY)
    set(CPL_cpldfs_FOUND True)
endif()

find_library(CPL_CPLDRS_LIBRARY
             NAMES cpldrs
             HINTS ${CPL_ROOT}/lib ${PC_CPL_LIBRARY_DIRS})
mark_as_advanced(CPL_CPLDRS_LIBRARY)
if(CPL_CPLDRS_LIBRARY)
    set(CPL_cpldrs_FOUND True)
endif()

find_library(CPL_CPLGASGANO_LIBRARY
             NAMES cplgasgano
             HINTS ${CPL_ROOT}/lib ${PC_CPL_LIBRARY_DIRS})
mark_as_advanced(CPL_CPLGASGANO_LIBRARY)
if(CPL_CPLGASGANO_LIBRARY)
    set(CPL_cplgasgano_FOUND True)
endif()

# Determine the library version from the header files if
# it is not yet known. Potentially this has already been 
# set by "pkg-config".
if(CPL_INCLUDE_DIR AND NOT CPL_VERSION)
    if(EXISTS ${CPL_INCLUDE_DIR}/cpl_version.h)
        file(STRINGS "${CPL_INCLUDE_DIR}/cpl_version.h"
             cpl_version
             REGEX "^[\t ]*#define[\t ]+CPL_VERSION_STRING[\t ]+\"[0-9]+\.[0-9]+.*\"")
        string(REGEX REPLACE
               "^[\t ]*#define[\t ]+CPL_VERSION_STRING[\t ]+\"([0-9]+\.[0-9][^a-z \"]*).*" "\\1"
               CPL_VERSION ${cpl_version})
        unset(cpl_version)
    endif()
endif()

# Decompose the version string into major, minor and patch version numbers
set(cpl_version_regex "([0-9]+)\.([0-9]+)(\.([0-9]+))?.*")
string(REGEX MATCH ${cpl_version_regex}
       cpl_version_string "${CPL_VERSION}")
set(CPL_VERSION_MAJOR ${CMAKE_MATCH_1})
set(CPL_VERSION_MINOR ${CMAKE_MATCH_2})
if(NOT "${CMAKE_MATCH_4}" STREQUAL "")
    set(CPL_VERSION_PATCH ${CMAKE_MATCH_4})
endif()
unset(cpl_version_string)
unset(cpl_version_regex)

find_package_handle_standard_args(CPL
                                  REQUIRED_VARS CPL_INCLUDE_DIR
                                  VERSION_VAR CPL_VERSION
                                  HANDLE_COMPONENTS)

if(CPL_FOUND)
    if(NOT TARGET CPL::cplcore)
        add_library(CPL::cplcore UNKNOWN IMPORTED)
        set_target_properties(CPL::cplcore PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${CPL_INCLUDE_DIR}")
        if(EXISTS "${CPL_CPLCORE_LIBRARY}")
            set_target_properties(CPL::cplcore PROPERTIES
                                  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                  IMPORTED_LOCATION ${CPL_CPLCORE_LIBRARY})
        endif()
    endif()

    if(NOT TARGET CPL::cplui)
        add_library(CPL::cplui UNKNOWN IMPORTED)
        set_target_properties(CPL::cplui PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${CPL_INCLUDE_DIR}")
        if(EXISTS "${CPL_CPLUI_LIBRARY}")
            set_target_properties(CPL::cplui PROPERTIES
                                  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                  IMPORTED_LOCATION ${CPL_CPLUI_LIBRARY})
            target_link_libraries(CPL::cplui INTERFACE CPL::cplcore)
        endif()
    endif()

    if(NOT TARGET CPL::cpldfs)
        add_library(CPL::cpldfs UNKNOWN IMPORTED)
        set_target_properties(CPL::cpldfs PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${CPL_INCLUDE_DIR}")
        if(EXISTS "${CPL_CPLDFS_LIBRARY}")
            set_target_properties(CPL::cpldfs PROPERTIES
                                  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                  IMPORTED_LOCATION ${CPL_CPLDFS_LIBRARY})
            target_link_libraries(CPL::cpldfs INTERFACE CPL::cplcore)
        endif()
    endif()

    if(NOT TARGET CPL::cpldrs)
        add_library(CPL::cpldrs UNKNOWN IMPORTED)
        set_target_properties(CPL::cpldrs PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${CPL_INCLUDE_DIR}")
        if(EXISTS "${CPL_CPLDRS_LIBRARY}")
            set_target_properties(CPL::cpldrs PROPERTIES
                                  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                  IMPORTED_LOCATION ${CPL_CPLDRS_LIBRARY})
            target_link_libraries(CPL::cpldrs INTERFACE CPL::cplcore)
        endif()
    endif()

    if(NOT TARGET CPL::cplgasgano)
        add_library(CPL::cplgasgano UNKNOWN IMPORTED)
        set_target_properties(CPL::cplgasgano PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${CPL_INCLUDE_DIR}")
        if(EXISTS "${CPL_CPLGASGANO_LIBRARY}")
            set_target_properties(CPL::cplgasgano PROPERTIES
                                  IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                  IMPORTED_LOCATION ${CPL_CPLGASGANO_LIBRARY})
            target_link_libraries(CPL::cplgasgano INTERFACE CPL::cpldfs CPL::cplcore)
        endif()
    endif()
endif()
