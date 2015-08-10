DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
# DEPENDS += "opencv"

SRC_URI = "file://main.c \
           file://tools.c \
           file://tools.h \
           file://linux_nfc_api.h "

S = "${WORKDIR}"

do_compile() {
	${CC} main.c tools.c -o nfc-demo
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 nfc-demo ${D}${bindir}
	install -m 0755 nfc-demo /tftpboot
}
