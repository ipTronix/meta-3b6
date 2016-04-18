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
   file://0007-st-kim-do-not-use-debugfs-if-disble-allow-suspend-for-unregister-callback.patch \
   file://0008-mcp251x-spi-patch-to-reduce-latency.patch \
   file://0009-btwilink-add-minimal-device-tree-support.patch \
   file://0010-ti-st-add-device-tree-support.patch \
   file://0011-Add-SDIO-function-devicetree-subnode-parsing.patch \
   file://0012-altera-cvifw.patch \
   file://0013-mxc_tvp5150-std_autodet.patch \
   file://0014-nxp_pn5xx.patch \
   file://0015-splashlogo.patch \
   file://0016-Flexcan-fix-fifo-overrun.patch \
   file://0017-DTB_added_rev2_gpio-Audio_fixed_for_kernel.patch \
   file://0018-WDOG-Assert-WDOG_B-on-Time-out.patch \
   file://0019-usb-capture-update.patch \
   file://0020-can2-3-inverted.patch \
   file://0021-fixed-altera-cvifw.patch \
   file://0022-MCP2515_FPGA_SPI.patch \
   file://0023-TVP5150-i2c.patch \
   file://defconfig \
"

KERNEL_IMAGE_BASE_NAME ?= "${KERNEL_IMAGETYPE}-${PKGE}-${PKGV}-${PKGR}"


