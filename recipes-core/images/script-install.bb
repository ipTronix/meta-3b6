SUMMARY = "Add an init script to handle gpio for tera-he"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYRIGHT;md5=ae06596d194c8fcae93f44a22bcd2e15"
RDEPENDS_${PN} = "initscripts"
PR = "r0"
SRC_URI = "file://S98lvds \
           file://COPYRIGHT \
"

S = "${WORKDIR}"

do_install () {
  install -d ${D}${sysconfdir}/rc5.d
  install -m 0755 S98lvds ${D}${sysconfdir}/rc5.d
}
