#
# Copyright (c) 2010-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_executable(GmicQt_FilterSelector_test
               ${gmic_qt_QRC}
               ${gmic_qt_QM}

               ${CMAKE_SOURCE_DIR}/digiKam/tests/host_test.cpp
               ${CMAKE_SOURCE_DIR}/digiKam/tests/main_filterselector.cpp
)

target_link_libraries(GmicQt_FilterSelector_test
                      PRIVATE

                      gmic_qt_common

                      Digikam::digikamcore
                      Digikam::digikamgui
                      Digikam::digikamdatabase

                      ${gmic_qt_LIBRARIES}
)

###

include_directories(${CMAKE_SOURCE_DIR}/digiKam/bqm/)

add_executable(GmicQt_Processor_test
               ${gmic_qt_QRC}
               ${gmic_qt_QM}

               ${CMAKE_SOURCE_DIR}/digiKam/bqm/gmicbqmprocessor.cpp

               ${CMAKE_SOURCE_DIR}/digiKam/tests/host_test.cpp
               ${CMAKE_SOURCE_DIR}/digiKam/tests/main_processor.cpp
)

target_link_libraries(GmicQt_Processor_test
                      PRIVATE

                      gmic_qt_common

                      Digikam::digikamcore

                      ${gmic_qt_LIBRARIES}
)
