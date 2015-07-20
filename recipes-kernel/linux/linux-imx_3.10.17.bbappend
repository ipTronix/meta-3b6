# TERA-HE 

DESCRIPTION = "kernel for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI_append_tera-he = " \
   file://000-tera-he-dts.patch \
   file://001-ltc3676.patch \
   file://004-fix-mmc-addresses.patch \
   file://defconfig \
"

