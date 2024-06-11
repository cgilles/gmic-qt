#!/bin/bash

# SPDX-FileCopyrightText: 2013-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Run Clang static analyzer on whole digiKam plugin source code.
# https://clang-analyzer.llvm.org/
# Dependencies : LLVM clang static analyzer version >= 16.
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

# Analyzer configuration.
. .scan-build

checksCPUCores

# Check run-time dependencies

if ! which scan-build ; then

    if ! which scan-build-17 ; then

        echo "SCAN-BUILD tool from LLVM is not installed!"
        echo "See https://clang-analyzer.llvm.org/scan-build.html for details."
        exit -1

    else

        SCAN_BUILD_BIN=scan-build-17

    fi

else

    SCAN_BUILD_BIN=scan-build

fi

echo "Found SCAN_BUILD tool: $SCAN_BUILD_BIN"

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.scan"

# Get active git branches to create report description string
TITLE="GmicQt-$(parseGitBranch)$(parseGitHash)"
echo "Clang Scan Static Analyzer task name: $TITLE"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR

cd ../..

rm -fr build.scan
mkdir -p build.scan
cd build.scan

if [[ -d /opt/qt6 ]] ; then

    export BUILD_WITH_QT6=1
    export Qt6_DIR=/opt/qt6
    QTPATHS="/opt/qt6/bin/qtpaths6"
    export CMAKE_BINARY=/opt/qt6/bin/cmake

else

    export BUILD_WITH_QT6=0
    QTPATHS="qtpaths"
    export CMAKE_BINARY=cmake

fi

$SCAN_BUILD_BIN $CMAKE_BINARY -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTING=ON \
      -DBUILD_WITH_QT6=$BUILD_WITH_QT6 \
      -DENABLE_ASAN=OFF \
      -DENABLE_SYSTEM_GMIC=OFF \
      -DGMIC_QT_HOST=digikam \
      -Wno-dev \
      ..

# Print the skipped directories taken from the config file.

for DROP_ITEM in $IGNORE_DIRS ; do

    if [[ $DROP_ITEM != *exclude ]] ; then

        echo "Skipped dir: $DROP_ITEM"

    fi

done

$SCAN_BUILD_BIN -o $REPORT_DIR \
           -v \
           -k \
           -no-failure-reports \
           --keep-empty \
           --show-description \
           --html-title $TITLE \
           $IGNORE_DIRS \
           make -j$CPU_CORES

cd $ORIG_WD

SCAN_BUILD_DIR=$(find ${REPORT_DIR} -maxdepth 1 -not -empty -not -name `basename ${REPORT_DIR}`)
echo "Clang Report $TITLE is located to $SCAN_BUILD_DIR"

cd $ORIG_DIR

rm -fr ../../build.scan
