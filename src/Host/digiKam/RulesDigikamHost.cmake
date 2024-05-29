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

if(BUILD_WITH_QT6)

    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)

else()

    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)

endif()

if(NOT QT_QMAKE_EXECUTABLE)

    message(FATAL_ERROR "qmake is not found.")

endif()

# execute the command "qmake -query QT_INSTALL_PLUGINS" to get the path of plugins dir.

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
                OUTPUT_VARIABLE QT_PLUGINS_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT QT_PLUGINS_DIR)

    message(FATAL_ERROR "Qt plugin directory cannot be detected.")

endif()

if(MSVC)

    file(COPY        ${CMAKE_SOURCE_DIR}/src/Host/digiKam/translations/
         DESTINATION ${CMAKE_SOURCE_DIR}/translations/)

endif()

# --- digiKam dependencies.

find_package(DigikamCore CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>)

find_package(DigikamGui CONFIG REQUIRED)

set_package_properties(DigikamGui PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam gui library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamgui,INTERFACE_INCLUDE_DIRECTORIES>)

find_package(DigikamDatabase CONFIG REQUIRED)

set_package_properties(DigikamDatabase PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam database library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamdatabase,INTERFACE_INCLUDE_DIRECTORIES>)

# --- Compile common codes.

include_directories(${CMAKE_SOURCE_DIR}/src/Host/digiKam/common/)

if(BUILD_WITH_QT6)

    qt6_wrap_ui(gmic_qt_SRCS ${gmic_qt_FORMS})

else()

    qt5_wrap_ui(gmic_qt_SRCS ${gmic_qt_FORMS})

endif()

add_definitions(-DGMIC_HOST=digikam)
add_definitions(-D_GMIC_QT_DISABLE_THEMING_)
add_definitions(-D_GMIC_QT_DISABLE_HDPI_)
add_definitions(-D_GMIC_QT_DISABLE_LOGO_)

add_library(gmic_qt_common STATIC
            ${gmic_qt_SRCS}
            ${CMAKE_SOURCE_DIR}/src/Host/digiKam/common/gmicqtimageconverter.cpp
            ${CMAKE_SOURCE_DIR}/src/Host/digiKam/common/gmicqtwindow.cpp
)

target_link_libraries(gmic_qt_common

                      PRIVATE

                      ${gmic_qt_LIBRARIES}
)

# --- Host Plugins Compilation Rules.

include(${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/EditorPluginRules.cmake)
#include(${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/BqmPluginRules.cmake)
