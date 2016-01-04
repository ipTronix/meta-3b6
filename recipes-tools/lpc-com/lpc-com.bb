DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
# DEPENDS += "opencv"

SRC_URI = "file://lpc-com.c \
          "

S = "${WORKDIR}"

do_compile() {
	${CC} lpc-com.c -o lpc-com
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 lpc-com ${D}${bindir}
	install -m 0755 lpc-com /tftpboot
}
