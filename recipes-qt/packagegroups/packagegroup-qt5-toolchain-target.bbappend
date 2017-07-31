# Copyright (C) 2014 O.S. Systems Software LTDA.

DESCRIPTION = "Target packages for Qt5 SDK"
LICENSE = "MIT"

RDEPENDS_${PN} += " \
    qtwebkit \
    packagegroup-fsl-tools-gpu \
    packagegroup-fsl-tools-gpu-external \
"
