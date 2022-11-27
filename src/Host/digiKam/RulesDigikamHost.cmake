# Some useful cmake macros for general purposes
#
# Copyright (c) 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(GNUInstallDirs)

# Debug Symbols rules under MacOS
MACOS_DEBUG_POLICIES()

find_package(DigikamCore CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

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

# --- Compilation Rules --------------------------------------------------------

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>/digikam)

set (gmic_qt_SRCS ${gmic_qt_SRCS} src/Host/digiKam/host_digikam.cpp
                                  src/Host/digiKam/gmicqttoolplugin.cpp
                                  src/Host/digiKam/gmicqtwindow.cpp
)

qt5_wrap_ui(gmic_qt_SRCS ${gmic_qt_FORMS})
add_definitions(-DGMIC_HOST=digikam)
add_library(Editor_GmicQt_Plugin
            MODULE ${gmic_qt_SRCS} ${gmic_qt_QRC} ${qmic_qt_QM})

set_target_properties(Editor_GmicQt_Plugin PROPERTIES PREFIX "")

target_link_libraries(Editor_GmicQt_Plugin
                      PRIVATE
                      ${gmic_qt_LIBRARIES}
                      Digikam::digikamcore)

# --- Install rules ------------------------------------------------------------

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

install(TARGETS Editor_GmicQt_Plugin
        DESTINATION ${QT_PLUGINS_DIR}/digikam/editor)
