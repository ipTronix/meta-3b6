# Copyright (C) 2014 O.S. Systems Software LTDA.
# Copyright (C) 2014-2015 Freescale Semiconductor

DESCRIPTION = "Manufacturing Tool compatible Linux Kernel to be used in updater environment for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}_${PV}:"

SRC_URI_append_tera-he = " \
   file://0000-tera-he-dts.patch \
   file://0001-splashlogo.patch \
   file://defconfig \
"
