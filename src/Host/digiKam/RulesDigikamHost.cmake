# Some useful cmake macros for general purposes
#
# Copyright (c) 2010-2023 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

include(GNUInstallDirs)

# Debug Symbols rules under MacOS
MACOS_DEBUG_POLICIES()

# --- Manage C++ exception rules -----------------------------------------------

string(REPLACE " -DQT_NO_EXCEPTIONS " " " CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ")
string(REPLACE " -fno-exceptions "    " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif()

string(STRIP "${CMAKE_CXX_FLAGS}" ${CMAKE_CXX_FLAGS})

# Increase the stack size on MacOS targets
# https://github.com/c-koi/gmic-qt/issues/160#issuecomment-1211248615
# https://github.com/ethereum/solidity/blob/develop/cmake/EthCompilerSettings.cmake

if (APPLE)

    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

        message(STATUS "Increase GCC linker stack size to 16MB under MacOS")

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,16777216")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--stack,16777216")
#        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--stack,16777216")

    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

        message(STATUS "Increase Clang linker stack size to 16MB under MacOS")

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")
#       set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x1000000")

    endif()

endif()

# --- For the Installation Rules ------------------------------------------------------------

get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)

if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

# execute the command "qmake -query QT_INSTALL_PLUGINS" to get the path of plugins dir.
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
                OUTPUT_VARIABLE QT_PLUGINS_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT QT_PLUGINS_DIR)
    message(FATAL_ERROR "Qt5 plugin directory cannot be detected.")
endif()

# --- Plugins Compilation Rules -----------------------------------------

include(${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/EditorPluginRules.cmake)
#include(${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/BqmPluginRules.cmake)