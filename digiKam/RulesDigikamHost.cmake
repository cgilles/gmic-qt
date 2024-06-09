#
# Copyright (c) 2010-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(GNUInstallDirs)

# Debug Symbols rules under MacOS

MACOS_DEBUG_POLICIES()

# To compile test codes.

option(BUILD_TESTING "Compile test codes" OFF)

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

# This is dirty: even if gmic-qt code use CMake to compile, the translation rules
# are hard-coded with Makefiles running in source dir as well.
# This is not compatible with bundle constrution on non-linux.

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/gmicqt/translations/fr.ts")

    message(STATUS "Copy pre-compiled translation files into gmicqt source dir...")

    file(COPY        ${CMAKE_SOURCE_DIR}/digiKam/translations/
         DESTINATION ${CMAKE_SOURCE_DIR}/gmicqt/translations/)

endif()

# --- digiKam dependencies.

find_package(DigikamCore CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "https://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>)

find_package(DigikamGui CONFIG REQUIRED)

set_package_properties(DigikamGui PROPERTIES
                       URL "https://www.digikam.org"
                       DESCRIPTION "digiKam gui library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamgui,INTERFACE_INCLUDE_DIRECTORIES>)

find_package(DigikamDatabase CONFIG REQUIRED)

set_package_properties(DigikamDatabase PROPERTIES
                       URL "https://www.digikam.org"
                       DESCRIPTION "digiKam database library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamdatabase,INTERFACE_INCLUDE_DIRECTORIES>)

# --- Compile common codes.

include_directories(${CMAKE_SOURCE_DIR}/digiKam/common/)

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
            ${CMAKE_SOURCE_DIR}/digiKam/common/gmicqtimageconverter.cpp
            ${CMAKE_SOURCE_DIR}/digiKam/common/gmicqtwindow.cpp
            ${CMAKE_SOURCE_DIR}/digiKam/common/gmicqtcommon.cpp
)

target_link_libraries(gmic_qt_common

                      PRIVATE

                      ${gmic_qt_LIBRARIES}
)

# --- Host Plugins Compilation Rules.

include(${CMAKE_SOURCE_DIR}/digiKam/editor/EditorPluginRules.cmake)
include(${CMAKE_SOURCE_DIR}/digiKam/bqm/BqmPluginRules.cmake)

if(BUILD_TESTING)

    include(${CMAKE_SOURCE_DIR}/digiKam/tests/TestsRules.cmake)

endif()
