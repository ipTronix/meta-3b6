DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://tvin.c \
           file://capture.c \
           file://capture.h \
           "

S = "${WORKDIR}"

do_compile() {
	${CC} tvin.c capture.c -lpthread -o tvin
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 tvin ${D}${bindir}
	install -m 0755 tvin /tftpboot/3b6
}
