# TERA-HE 

DESCRIPTION = "kernel for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
  file://0000-tera-he-dts.patch \
  file://0003-mxc_tvp5150.patch \
"

KERNEL_IMAGE_BASE_NAME ?= "${KERNEL_IMAGETYPE}-${PKGE}-${PKGV}-${PKGR}"

