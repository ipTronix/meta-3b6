# TERA-HE 

DESCRIPTION = "kernel for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI_append_tera-he = " \
   file://000-tera-he-dts.patch \
   file://001-ltc3676.patch \
   file://002-lsm330.patch \
   file://004-fix-mmc-addresses.patch \
   file://005-patch-rtc-for-dts.patch \
   file://006-altera_framebuffer.patch \
   file://007-add-advancedsilicon-multitouch.patch \
   file://defconfig \
"


KERNEL_IMAGE_BASE_NAME ?= "${KERNEL_IMAGETYPE}-${PKGE}-${PKGV}-${PKGR}"
