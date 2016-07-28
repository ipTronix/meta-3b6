SUMMARY = "Basic TCP/IP networking init scripts and configuration files"
DESCRIPTION = "Limit eth0 link speed to 100Mbps"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/copyright;md5=3dd6192d306f582dee7687da3d8748ab"
PR = "r7"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
