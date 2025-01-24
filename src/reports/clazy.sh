#!/bin/bash

# SPDX-FileCopyrightText: 2013-2025 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Run Clazy analyzer on whole digiKam source code.
# https://github.com/KDE/clazy
# Dependencies : Python BeautifulSoup and SoupSieve at run-time.
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

# Analyzer configuration.
. .clazy

# Check run-time dependencies

if [ ! -f /opt/clazy/bin/clazy ] ; then

    echo "Clazy static analyzer is not installed in /opt/clazy."
    echo "Please install Clazy from https://github.com/KDE/clazy"
    echo "Aborted..."
    exit -1


else

    echo "Check Clazy static analyzer passed..."

fi

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.clazy"

# Get active git branches to create report description string
TITLE="GmicQt-$(parseGitBranch)$(parseGitHash)"
echo "Clazy Static Analyzer task name: $TITLE"

echo "IGNORE DIRS CONFIGURATION: $CLAZY_IGNORE_DIRS"
echo "CHECKERS CONFIGURATION:    $CLAZY_CHECKS"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR
mkdir -p $REPORT_DIR

cd ../..

rm -fr build.clazy
mkdir -p build.clazy
cd build.clazy

export PATH=$PATH:/opt/clazy/bin

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

$CMAKE_BINARY -G "Unix Makefiles" . \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=clazy \
      -DBUILD_WITH_QT6=$BUILD_WITH_QT6 \
      -DBUILD_TESTING=ON \
      -DENABLE_ASAN=OFF \
      -DENABLE_SYSTEM_GMIC=OFF \
      -DGMIC_QT_HOST=digikam \
      -Wno-dev \
      ..

make -j$CPU_CORES 2> ${REPORT_DIR}/trace.log

cd $ORIG_WD

python3 ./clazy_visualizer.py $REPORT_DIR/trace.log

rm -f $REPORT_DIR/trace.log
mv clazy.html $REPORT_DIR/index.html

cd $ORIG_DIR

rm -fr ../../build.clazy
