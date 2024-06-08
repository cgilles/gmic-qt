#
# Copyright (c) 2019-2024, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(gmicqt-digiKam)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/gmicqt/cmake/modules ${CMAKE_MODULE_PATH})

include_directories(${CMAKE_SOURCE_DIR}/gmicqt/src)

add_subdirectory(gmicqt)
