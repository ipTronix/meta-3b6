DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
# DEPENDS += "opencv"

SRC_URI = "file://main.c \
           file://can.c \
           file://can.h "

S = "${WORKDIR}"

do_compile() {
	${CC} main.c can.c can.h -o can-test
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 can-test ${D}${bindir}
	install -m 0755 can-test /tftpboot
}
