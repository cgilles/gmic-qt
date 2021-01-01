# Some useful cmake macros for general purposes
#
# Copyright (c) 2010-2021 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# -------------------------------------------------------------------------

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

