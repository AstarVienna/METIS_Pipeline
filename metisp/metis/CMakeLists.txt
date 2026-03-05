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

# Public header files
set(metis_HEADERS
    metis_dfs.h
    metis_pfits.h
    metis_utils.h)

set(metis_SOURCES
    metis_dfs.c
    metis_pfits.c
    metis_utils.c)

add_library(metis SHARED ${metis_SOURCES})
add_library(METIS::metis ALIAS metis)

target_include_directories(metis
    PUBLIC
        $<INSTALL_INTERFACE:include/${PACKAGE_NAME}>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${PROJECT_BINARY_DIR})

target_compile_options(metis
    PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-pipe -g3 -ggdb -O0 -rdynamic -fno-inline -fno-builtin -pedantic -Wextra -Wall -W -Wcast-align -Winline -Wmissing-noreturn -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wunreachable-code -Wwrite-strings -Wmissing-field-initializers -Wmissing-format-attribute>
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang,AppleClang>>:-pipe -g3 -O0 -fno-inline -fno-builtin -pedantic -Wextra -Wall -W -Wcast-align -Winline -Wimplicit-function-declaration -Wmissing-noreturn -Wincompatible-pointer-types -Wpointer-arith -Wshadow -Wsign-compare -Wundef -Wunreachable-code -Wwrite-strings -Wmissing-field-initializers -Wmissing-format-attribute>)

target_link_libraries(metis
    PRIVATE 
        #HDRL::hdrl
        CPL::cpldfs
        CPL::cplui
        CPL::cplcore)

set_target_properties(metis PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
    SKIP_BUILD_RPATH YES
    VERSION ${PACKAGE_LIBRARY_VERSION}
    SOVERSION ${PACKAGE_LT_CURRENT}
    EXPORT_NAME metis
    PUBLIC_HEADER "${metis_HEADERS}")

install(TARGETS metis
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PACKAGE_NAME}-${PACKAGE_VERSION}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME})
