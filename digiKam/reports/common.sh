#!/bin/bash

# SPDX-FileCopyrightText: 2013-2024 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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
