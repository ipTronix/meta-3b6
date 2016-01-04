DESCRIPTION = "Tests application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://fb_test.c \
           file://v4l2dev.c \
           file://v4l2dev.h \
           file://capture-example.c "

S = "${WORKDIR}"

do_compile() {
	${CC} fb_test.c v4l2dev.c -o fb_test
	${CC} capture-example.c -o capture-example
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 fb_test ${D}${bindir}
	install -m 0755 fb_test /tftpboot/3b6
	install -m 0755 capture-example /tftpboot/3b6
}
