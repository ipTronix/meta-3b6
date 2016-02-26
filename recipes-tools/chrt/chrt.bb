DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://chrt.c \
           file://c.h \
           file://nls.h \
          "

S = "${WORKDIR}"

do_compile() {
	${CC} chrt.c -o chrt
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 chrt ${D}${bindir}
	install -m 0755 chrt /tftpboot
}