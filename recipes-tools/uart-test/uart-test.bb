DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://uart.c \
           file://uart.h \
          "

S = "${WORKDIR}"

do_compile() {
	${CC} main.c uart.c -o uart-test
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 uart-test ${D}${bindir}
}
