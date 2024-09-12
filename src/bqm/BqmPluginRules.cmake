#
# Copyright (c) 2010-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# BQM Plugin Compilation Rules
#

set(gmic_bqm_SRCS
    ${CMAKE_SOURCE_DIR}/src/bqm/host_digikam_bqm.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfiltermodel.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilternode.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilternode_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilternode_writer.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfiltermngr.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfiltermngr_stack.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfiltermngr_proxy.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfiltermngr_delegate.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilterwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilterdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicbqmprocessor.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicbqmtool.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicbqmplugin.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilterchain.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilterchain_item.cpp
    ${CMAKE_SOURCE_DIR}/src/bqm/gmicfilterchain_view.cpp
)

foreach(_file ${gmic_bqm_SRCS})
    set_property(SOURCE ${_file} PROPERTY COMPILE_DEFINITIONS ${modern_qt_definitions})
endforeach()

add_library(Bqm_Gmic_Plugin
            MODULE

            ${gmic_bqm_SRCS}
            ${gmic_qt_QRC}
            ${gmic_qt_QM}
)

set_target_properties(Bqm_Gmic_Plugin PROPERTIES PREFIX "")

set_target_properties(Bqm_Gmic_Plugin
                      PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO
)

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
