DESCRIPTION = "Video tests application"
SECTION = "Tests"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://cap.c \
           file://cap.h \
           file://out.c \
           file://out.h \
           file://v4l2.c \
           file://v4l2.h \
           file://fs.c \
           file://fs.h \
           file://rng.c \
           file://rng.h \
           file://event.c \
           file://event.h \
           file://nodev.uyvy \
           file://noin.uyvy \
           file://wait.uyvy \
           "

S = "${WORKDIR}"

do_compile() {
	${CC} main.c cap.c out.c v4l2.c fs.c rng.c event.c -lpthread -o videotest -g3 -D_GNU_SOURCE
}

do_install() {
	install -d ${D}${bindir}
  install -m 0755 videotest  ${D}${bindir}
  install -m 0755 nodev.uyvy ${D}${bindir}
  install -m 0755 noin.uyvy  ${D}${bindir}
  install -m 0755 wait.uyvy  ${D}${bindir}
}
