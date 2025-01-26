#!/bin/bash

# SPDX-FileCopyrightText: 2013-2025 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

########################################################################
# checks if branch has something pending
function parseGitDirty()
{
    git diff --quiet --ignore-submodules HEAD 2>/dev/null; [ $? -eq 1 ] && echo "M"
}

########################################################################
# gets the current git branch
function parseGitBranch()
{
    git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e "s/* \(.*\)/\1/"
}

########################################################################
# get last commit hash prepended with @ (i.e. @8a323d0)
function parseGitHash()
{
    git rev-parse --short HEAD 2> /dev/null | sed "s/\(.*\)/-rev-\1/"
}

########################################################################
# Check CPU core available (Linux or MacOS)
function checksCPUCores()
{
    CPU_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

    if [[ $CPU_CORES -gt 1 ]]; then
        CPU_CORES=$((CPU_CORES-1))
    fi

    echo "CPU Cores to use : $CPU_CORES"
}

########################################################################
# For time execution measurement ; startup
StartScript()
{

BEGIN_SCRIPT=$(date +"%s")

}

########################################################################
# For time execution measurement : shutdown
TerminateScript()
{

TERMIN_SCRIPT=$(date +"%s")
difftimelps=$(($TERMIN_SCRIPT-$BEGIN_SCRIPT))
echo "Elaspsed time for script execution : $(($difftimelps / 3600 )) hours $((($difftimelps % 3600) / 60)) minutes $(($difftimelps % 60)) seconds"

}
