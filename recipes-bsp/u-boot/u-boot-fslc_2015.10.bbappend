# 3B6 Tera-HE

DEPENDS = "ncurses"
DESCRIPTION = "bootloader for 3b6 imx6q tera-he"

# PV = "2015.10"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_tera-he = " \
   file://0000-Add_Tera_HE_target.patch \
   file://0001-SplashScreen.patch \
   file://0002-Imx-boot-control.patch \
"
SRC_URI_append_tera-he-dep = " \
   file://0000-Add_Tera_HE_target.patch \
   file://0001-SplashScreen.patch \
   file://0002-Imx-boot-control.patch \
   file://0003-Add_Tera_HE_dep_target.patch \
"

UBOOT_LOCALVERSION_append_tera-he = "[1.0_rc10]"
UBOOT_LOCALVERSION_append_tera-he-dep = "[1.0.dep_rc10]"