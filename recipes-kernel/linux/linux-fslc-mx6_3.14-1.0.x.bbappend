# TERA-HE 

DESCRIPTION = "kernel for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
   file://0000-tera-he-dts.patch \
   file://0001-ltc3676.patch \
   file://0002-lsm330.patch \
   file://0003-mxc_tvp5150.patch \
   file://0004-advancedsilicon_multitouch.patch \
   file://0005-backported-new-isl29018-driver.patch \
   file://0006-Bluetooth-Add-tty-HCI-driver.patch \
   file://0007-st_kim-do-not-use-debugfs-functions-if-not-enabled.patch \
   file://0008-st_kim-allow-suspend-if-callback-is-not-registered.patch \
   file://0009-btwilink-add-minimal-device-tree-support.patch \
   file://0010-ti-st-add-device-tree-support.patch \
   file://0011-Add-SDIO-function-devicetree-subnode-parsing.patch \
   file://defconfig \
"


KERNEL_IMAGE_BASE_NAME ?= "${KERNEL_IMAGETYPE}-${PKGE}-${PKGV}-${PKGR}"


