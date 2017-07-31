SUMMARY = "Add an init script to handle gpio for tera-he"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYRIGHT;md5=ae06596d194c8fcae93f44a22bcd2e15"
RDEPENDS_${PN} = "initscripts"
PR = "r0"
SRC_URI = "file://S00boot-confirm \
           file://fstab \
           file://inittab \
           file://S89reset \
	   file://libnfc.conf \
           file://COPYRIGHT \
           file://S01Vt3 \
           file://versionBSP \
"

FILES_${PN} += "/mnt/floppy \ 
                /mnt/cdrom \
                /mnt/userfs1 \
                /mnt/usb1Vt3 \
                /mnt/usb2Vt3 \
                /mnt/usb3Vt3 "
S = "${WORKDIR}"

do_install () {
  install -d ${D}${sysconfdir}/rcS.d
  install -m 0755 S00boot-confirm ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/rc5.d
  install -d ${D}${sysconfdir}/rc6.d
  install -m 0755 S89reset ${D}${sysconfdir}/rc6.d
  install -m 0755 fstab ${D}/etc/
  install -m 0755 inittab ${D}/etc/
  install -d ${D}/mnt/floppy
  install -d ${D}/mnt/cdrom
  install -d ${D}/mnt/userfs1
  install -d ${D}/mnt/usb1Vt3
  install -d ${D}/mnt/usb2Vt3
  install -d ${D}/mnt/usb3Vt3
  install -m 0755 S01Vt3 ${D}/etc/rc5.d/
  install -m 0755 versionBSP ${D}/etc/
}
