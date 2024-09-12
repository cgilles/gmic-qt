#
# Copyright (c) 2010-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Editor Plugin Compilation Rules
#

set(gmic_qt_editor_SRCS
    ${CMAKE_SOURCE_DIR}/src/editor/host_digikam_editor.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/gmicqttoolplugin.cpp
)

foreach(_file ${gmic_qt_editor_SRCS})
    set_property(SOURCE ${_file} PROPERTY COMPILE_DEFINITIONS ${modern_qt_definitions})
endforeach()

add_library(Editor_GmicQt_Plugin
            MODULE
            ${gmic_qt_QRC}
            ${gmic_qt_QM}
            ${gmic_qt_editor_SRCS}
)

set_target_properties(Editor_GmicQt_Plugin PROPERTIES PREFIX "")

set_target_properties(Editor_GmicQt_Plugin
                      PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO
)

target_link_libraries(Editor_GmicQt_Plugin
                      PRIVATE

                      gmic_qt_common

                      Digikam::digikamcore

                      ${gmic_qt_LIBRARIES}
)

install(TARGETS Editor_GmicQt_Plugin
        DESTINATION ${QT_PLUGINS_DIR}/digikam/editor)

# Install debug symbols

if(MSVC)
    install(FILES "$<TARGET_PDB_FILE:Editor_GmicQt_Plugin>"
            DESTINATION ${QT_PLUGINS_DIR}/digikam/editor
            CONFIGURATIONS Debug RelWithDebInfo
    )
endif()

if(APPLE)
    install(FILES "$<TARGET_FILE:Editor_GmicQt_Plugin>.dSYM"
            DESTINATION ${QT_PLUGINS_DIR}/digikam/editor
            CONFIGURATIONS Debug RelWithDebInfo
    )
endif()
