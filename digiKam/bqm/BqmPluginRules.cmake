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

# --- BQM Plugin Compilation Rules -----------------------------------------

set(gmic_bqm_SRCS
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/host_digikam_bqm.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilterwidget.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilterdialog.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilternode.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfiltermngr.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicbqmprocessor.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicbqmtool.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicbqmplugin.cpp

    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilterchain.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilterchain_item.cpp
    ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicfilterchain_view.cpp
)

add_library(Bqm_Gmic_Plugin
            MODULE

            ${gmic_bqm_SRCS}
            ${gmic_qt_QRC}
            ${gmic_qt_QM}
)

set_target_properties(Bqm_Gmic_Plugin PROPERTIES PREFIX "")

target_link_libraries(Bqm_Gmic_Plugin
                      PRIVATE

                      gmic_qt_common

                      Digikam::digikamcore
                      Digikam::digikamgui
                      Digikam::digikamdatabase

                      ${gmic_qt_LIBRARIES}
)

install(TARGETS Bqm_Gmic_Plugin
        DESTINATION ${QT_PLUGINS_DIR}/digikam/bqm)

# Install debug symbols

if(MSVC)
    install(FILES "$<TARGET_PDB_FILE:Bqm_Gmic_Plugin>"
            DESTINATION ${QT_PLUGINS_DIR}/digikam/bqm
            CONFIGURATIONS Debug RelWithDebInfo
    )
endif()

if(APPLE)
    install(FILES "$<TARGET_FILE:Bqm_Gmic_Plugin>.dSYM"
            DESTINATION ${QT_PLUGINS_DIR}/digikam/bqm
            CONFIGURATIONS Debug RelWithDebInfo
    )
endif()
