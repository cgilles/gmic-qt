#!/bin/bash

# Copyright (c) 2008-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Script to configure cmake build.

export MAKEFILES_TYPE='Unix Makefiles'

ln -s ../gmic ./gmic

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

/opt/qt6/bin/cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DENABLE_ASAN=OFF \
      -DENABLE_SYSTEM_GMIC=OFF \
      -DGMIC_QT_HOST=digikam \
      -DBUILD_WITH_QT6=ON \
      -Wno-dev \
      ..
