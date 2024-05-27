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

find_package(DigikamCore CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories(${CMAKE_SOURCE_DIR}/src/Host/digiKam/common/
                    $<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(gmic_qt_editor_SRCS ${gmic_qt_SRCS}
                        ${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/host_digikam_editor.cpp
                        ${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/gmicqttoolplugin.cpp
                        ${CMAKE_SOURCE_DIR}/src/Host/digiKam/editor/gmicqtwindow.cpp

                        ${CMAKE_SOURCE_DIR}/src/Host/digiKam/common/gmicqtimageconverter.cpp
)

if(BUILD_WITH_QT6)

    qt6_wrap_ui(gmic_qt_editor_SRCS ${gmic_qt_FORMS})

else()

    qt5_wrap_ui(gmic_qt_editor_SRCS ${gmic_qt_FORMS})

endif()

add_definitions(-DGMIC_HOST=digikam)
add_definitions(-D_GMIC_QT_DISABLE_THEMING_)
add_definitions(-D_GMIC_QT_DISABLE_HDPI_)
add_definitions(-D_GMIC_QT_DISABLE_LOGO_)

add_library(Editor_GmicQt_Plugin
            MODULE ${gmic_qt_editor_SRCS} ${gmic_qt_QRC} ${gmic_qt_QM})

set_target_properties(Editor_GmicQt_Plugin PROPERTIES PREFIX "")

target_link_libraries(Editor_GmicQt_Plugin
                      PRIVATE
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
