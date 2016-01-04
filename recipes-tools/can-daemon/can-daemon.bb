DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
# DEPENDS += "opencv"

SRC_URI = "file://can_daemon.c \
           file://can_daemon.h \
           file://run_eoloader.c \
           file://run_eoloader.h \
           file://can_shm.h \
           file://hal.c \
           file://hal.h \
           file://version_can_daemon.h \
          "

S = "${WORKDIR}"

do_compile() {
	${CC} can_daemon.c hal.c -o can-daemon
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 can-daemon ${D}${bindir}
	install -m 0755 can-daemon /tftpboot
}
