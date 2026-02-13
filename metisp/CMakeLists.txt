# This file is part of the ESO METIS Pipeline
# Copyright (C) 2023-2024 European Southern Observatory
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

cmake_minimum_required(VERSION 3.12)

project(METIS VERSION 0.0.1 LANGUAGES C CXX)

# Add local cmake module search path
list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

# Set policies
cmake_policy(VERSION 3.12)
# Safer library linking
cmake_policy(SET CMP0003 NEW)
# Automatically escape preprocessor definitions
cmake_policy(SET CMP0005 NEW)
# Make syntax problems errors
cmake_policy(SET CMP0010 NEW)
# Input directories must have CMakeLists.txt
cmake_policy(SET CMP0014 NEW)
# Do not interpret quoted or bracketed variables in if statments
cmake_policy(SET CMP0054 NEW)
# Usage of <PackageName>_ROOT variables
cmake_policy(SET CMP0074 NEW)
# Escape RPATH entries in intermediate scripts
cmake_policy(SET CMP0095 NEW)
# Deprecated FindPythonInterp and FindPythonLibs
cmake_policy(SET CMP0148 NEW)
# exec_program() command should not be called.
cmake_policy(SET CMP0153 NEW)

# Setup basic pipeline package information
include(EsoPipelinePackage)

# Immediately before every release do:
#-------------------------------------
#   if (the interface is totally unchanged from previous release)
#       REVISION++;
#   else {
#       /* interfaces have been added, removed or changed */
#       REVISION = 0;
#       CURRENT++;
#       if (any interfaces have been _added_ since last release)
#           AGE++;
#       if (any interfaces have been _removed_ or incompatibly changed)
#           AGE = 0;
#   }
eso_set_library_version(0 0 0)

# Require out-of-source build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    if (NOT DEFINED WITH_IN_SOURCE_BUILD)
        message(FATAL_ERROR "CMake generation for this project is not allowed
                            within the source directory!")
    endif()
endif()

# Default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
        options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

# Set install directories
if (UNIX AND NOT WIN32)
    include(GNUInstallDirs)
    set(PACKAGE_RECIPE_DIR "${CMAKE_INSTALL_LIBDIR}/esopipes-plugins")
endif()


# Find external packages
find_package(PkgConfig)
find_package(CPL "7.2" REQUIRED COMPONENTS cplcore cplui cpldfs cpldrs)
#find_package(HDRL "1.5" REQUIRED COMPONENTS hdrl)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Package creation
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
    .git
    .svn
    .vscode
    .clang-format
    README.DEV
    ${PROJECT_BINARY_DIR})
include(CPack)

# Create config.h
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h")
add_definitions(-DHAVE_CONFIG_H)

set_source_files_properties(${PROJECT_BINARY_DIR}/config.h
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake"
    "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# Add subdirectories
add_subdirectory(metis)
add_subdirectory(recipes)
add_subdirectory(pymetis)
add_subdirectory(pyrecipes)
