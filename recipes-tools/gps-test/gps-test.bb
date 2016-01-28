DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://gps-test.c \
          "

S = "${WORKDIR}"

do_compile() {
	${CC} gps-test.c -o gps-test
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 gps-test ${D}${bindir}
	install -m 0755 gps-test /tftpboot
}
