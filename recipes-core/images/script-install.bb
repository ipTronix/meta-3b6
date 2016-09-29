SUMMARY = "Add an init script to handle gpio for tera-he"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYRIGHT;md5=ae06596d194c8fcae93f44a22bcd2e15"
RDEPENDS_${PN} = "initscripts"
PR = "r0"
SRC_URI = " \
           file://S00boot-confirm \
           file://S98lvds \
           file://S89reset \
           file://80-usbdisk.rules \
           file://usbdevinserted \
           file://usbdevinserted_ \
           file://libnfc.conf \
           file://COPYRIGHT \
"

S = "${WORKDIR}"

do_install () {
  install -d ${D}${sysconfdir}/rcS.d
  install -m 0755 S00boot-confirm ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/rc5.d
  install -m 0755 S98lvds ${D}${sysconfdir}/rc5.d
  install -d ${D}${sysconfdir}/rc6.d
  install -m 0755 S89reset ${D}${sysconfdir}/rc6.d
  install -d ${D}/lib/udev/rules.d
  install -m 0755 80-usbdisk.rules ${D}/lib/udev/rules.d
  install -d ${D}/bin
  install -m 0755 usbdevinserted ${D}/bin
  install -m 0755 usbdevinserted_ ${D}/bin
  install -d ${D}${sysconfdir}/nfc
  install -m 0755 libnfc.conf ${D}${sysconfdir}/nfc
}
