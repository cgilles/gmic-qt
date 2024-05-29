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

# --- Editor Plugin Compilation Rules -----------------------------------------

set(gmic_qt_editor_SRCS
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/host_digikam_editor.cpp
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/gmicqttoolplugin.cpp
)

add_library(Editor_GmicQt_Plugin
            MODULE
            ${gmic_qt_QRC}
            ${gmic_qt_QM}
            ${gmic_qt_editor_SRCS}
)

set_target_properties(Editor_GmicQt_Plugin PROPERTIES PREFIX "")

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
