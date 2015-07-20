# 3B6 Tera-HE 

DESCRIPTION = "bootloader for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
   file://001_add_tera-he_target.patch \
"
