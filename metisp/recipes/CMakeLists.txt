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

set(metis_calib_SOURCES
    metis_calib.c)

add_library(metis_calib MODULE ${metis_calib_SOURCES})

target_include_directories(metis_calib
    #PUBLIC
    #    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/metis>
    #    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/metis
        ${PROJECT_BINARY_DIR})

target_compile_options(metis_calib
    PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-pipe -g3 -ggdb -O0 -rdynamic -fno-inline -fno-builtin -pedantic -Wextra -Wall -W -Wcast-align -Winline -Wmissing-noreturn -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wunreachable-code -Wwrite-strings -Wmissing-field-initializers -Wmissing-format-attribute>
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang,AppleClang>>:-pipe -g3 -O0 -fno-inline -fno-builtin -pedantic -Wextra -Wall -W -Wcast-align -Winline -Wimplicit-function-declaration -Wmissing-noreturn -Wincompatible-pointer-types -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wunreachable-code -Wwrite-strings -Wmissing-field-initializers -Wmissing-format-attribute>)

target_link_libraries(metis_calib PRIVATE
    METIS::metis
    #HDRL::hdrl
    CPL::cpldfs
    CPL::cplui
    CPL::cplcore)

set_target_properties(metis_calib PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
    SKIP_BUILD_RPATH NO
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME metis_calib)

install(TARGETS metis_calib
        LIBRARY DESTINATION ${PACKAGE_RECIPE_DIR}/${PACKAGE_NAME}-${PACKAGE_VERSION}
)
