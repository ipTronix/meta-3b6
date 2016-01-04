# Copyright (C) 2014 O.S. Systems Software LTDA.

DESCRIPTION = "MFG tool bootloader for 3b6 imx6q tera-he"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
   file://0000-Add_Tera_HE_target.patch \
"

