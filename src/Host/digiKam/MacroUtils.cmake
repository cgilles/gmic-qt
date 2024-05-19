# Some useful cmake macros for general purposes
#
# Copyright (c) 2010-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# G'MIC-Qt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# G'MIC-Qt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

macro(MACOS_DEBUG_POLICIES)

    # Cmake do not support yet the dSYM scheme.
    # See details from the CMake report: https://gitlab.kitware.com/cmake/cmake/-/issues/20256

    if(APPLE)

        if((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))

            message(STATUS "MacOS optimized build, symbol generation turned-OFF" )

            # on optimized builds, do NOT turn-on symbol generation.

        else()

            message(STATUS "MacOS non-optimized build, symbol generation turned-ON")

            # Incredibly, for both clang and g++, while a single compile-and-link
            # invocation will create an executable.dSYM/ dir with debug info,
            # with separate compilation the final link does NOT create the
            # dSYM dir.
            # The "dsymutil" program will create the dSYM dir for us.
            # Strangely it takes in the executable and not the object
            # files even though it's the latter that contain the debug info.
            # Thus it will only work if the object files are still sitting around.

            find_program(DSYMUTIL_PROGRAM dsymutil)

            if (DSYMUTIL_PROGRAM)

                set(CMAKE_C_LINK_EXECUTABLE
                    "${CMAKE_C_LINK_EXECUTABLE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_C_CREATE_SHARED_LIBRARY
                    "${CMAKE_C_CREATE_SHARED_LIBRARY}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_LINK_EXECUTABLE
                    "${CMAKE_CXX_LINK_EXECUTABLE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_CREATE_SHARED_LIBRARY
                    "${CMAKE_CXX_CREATE_SHARED_LIBRARY}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

                set(CMAKE_CXX_CREATE_SHARED_MODULE
                    "${CMAKE_CXX_CREATE_SHARED_MODULE}"
                    "${DSYMUTIL_PROGRAM} <TARGET>")

            endif()

        endif()

    endif()

endmacro()

