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

# --- BQM Plugin Compilation Rules -----------------------------------------

find_package(DigikamCore CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>/digikam)

find_package(DigikamGui CONFIG REQUIRED)

set_package_properties(DigikamGui PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam gui library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamgui,INTERFACE_INCLUDE_DIRECTORIES>/digikam)

find_package(DigikamDatabase CONFIG REQUIRED)

set_package_properties(DigikamDatabase PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam database library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamdatabase,INTERFACE_INCLUDE_DIRECTORIES>/digikam)

set(gmic_qt_bqm_SRCS
    ${gmic_qt_SRCS}
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/host_digikam.cpp
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/bqm_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/bqm_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/gmicqtbqmtool.cpp
    ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/gmicqtplugin.cpp
)

qt5_wrap_ui(gmic_qt_bqm_SRCS ${CMAKE_SOURCE_DIR}/src/Host/digiKam/bqm/bqm_widget.ui)

add_definitions(-DGMIC_HOST=digikam)
add_definitions(-D_GMIC_QT_DISABLE_THEMING_)
add_definitions(-D_GMIC_QT_DISABLE_HDPI_)
add_definitions(-D_GMIC_QT_DISABLE_LOGO_)
add_library(Bqm_GmicQt_Plugin
            MODULE ${gmic_qt_bqm_SRCS} ${gmic_qt_QRC} ${qmic_qt_QM})

set_target_properties(Bqm_GmicQt_Plugin PROPERTIES PREFIX "")

target_link_libraries(Bqm_GmicQt_Plugin
                      PRIVATE
                      ${gmic_qt_LIBRARIES}
                      Digikam::digikamcore
                      Digikam::digikamgui
                      Digikam::digikamdatabase)

install(TARGETS Bqm_GmicQt_Plugin
        DESTINATION ${QT_PLUGINS_DIR}/digikam/bqm)