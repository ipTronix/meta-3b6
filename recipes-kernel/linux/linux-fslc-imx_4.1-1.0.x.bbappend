# TERA-HE

DESCRIPTION = "kernel for 3b6 tera-he platform"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
  file://0000-tera-he-dts.patch \
  file://0001-ltc3676.patch \
  file://0002-lsm330.patch \
  file://0003-mxc_tvp5150.patch \
  file://0004-advancedsilicon_multitouch.patch \
  file://0005-splashlogo.patch \
  file://0006-Fix-flexcan-fifo-overrun.patch \
  file://0006-mcp251x-spi-reduce-latency.patch \
  file://0007-WDOG-Assert-WDOG_B-on-Time-out.patch \
  file://0008-altera-cvifw.patch \
  file://0009-MCP2515_FPGA_SPI.patch \
  file://0010-usb-capture-update.patch \
  file://0011-TVP5150-i2c.patch \
  file://0012-Bluetooth-Add-tty-HCI-driver.patch \
  file://0013-btwilink-add-minimal-device-tree-support.patch \
  file://0014-TI_WL18xx.patch \
  file://0015-FIX_v4l2-videobuf2_core.patch \
  file://0016-FIX_mac80211-disable-WMM.patch \
  file://0017-imx_thermal-disable_thermal_zone_bind_cooling_device.patch \
  file://0018-Added_F-RAM_in_dtb.patch \
  file://0019-Altera_CVIFW-Fix_first_frame.patch \
  file://0020-Fix_BUG_flexcan_scheduling_while_atomic.patch \
  file://defconfig \
  file://0021-mxc_tvp5150-Add_contrast.patch \
  file://0022-DTB_display_timing_to_1280x800-60.patch \
  file://0023-FIX-deadlock-altera_cvifw.patch \
  file://0025-Add_wilink8_bt.patch \
  file://0026-Fix_altera_cvifw_for_gst.patch \
  file://0027-Fix_regulator.patch \
  file://0028-Fix_mxc_v4l2_capture_memory_allocation.patch \
  file://0030-Fix_altera_cvi_driver.patch \
"

SRC_URI_append_tera-he-dep = " \
  file://0000-tera-he-dep-dts.patch \
  file://0001-ltc3676.patch \
  file://0002-lsm330.patch \
  file://0003-mxc_tvp5150.patch \
  file://0004-advancedsilicon_multitouch.patch \
  file://0005-splashlogo.patch \
  file://0006-Fix-flexcan-fifo-overrun.patch \
  file://0006-mcp251x-spi-reduce-latency.patch \
  file://0007-WDOG-Assert-WDOG_B-on-Time-out.patch \
  file://0015-FIX_v4l2-videobuf2_core.patch \
  file://0030-Fix_altera_cvi_driver.patch \
  file://defconfig \
"
do_configure_append_tera-he-dep() {
    install -m 0755 ${WORKDIR}/defconfig .config
}

do_configure_append_tera-he() {
    install -m 0755 ${WORKDIR}/defconfig .config
}

KERNEL_IMAGE_BASE_NAME ?= "${KERNEL_IMAGETYPE}-${PKGE}-${PKGV}-${PKGR}"
