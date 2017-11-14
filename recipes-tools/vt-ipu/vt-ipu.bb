DESCRIPTION = "Video tests application"
SECTION = "Tests"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "virtual/libg2d"

SRC_URI = "file://main.c \
           file://cap.c \
           file://cap.h \
           file://out.c \
           file://out.h \
           file://v4l2.c \
           file://v4l2.h \
           file://fs.c \
           file://fs.h \
           file://fb.c \
           file://fb.h \
           file://grx.c \
           file://grx.h \
           file://blt.c \
           file://blt.h \
           file://ipu.c \
           file://ipu.h \
           file://rng.c \
           file://rng.h \
           file://event.c \
           file://event.h \
           file://mxcfb.h \
           "

S = "${WORKDIR}"

do_compile() {
	${CC} main.c cap.c out.c v4l2.c fb.c blt.c ipu.c grx.c fs.c rng.c event.c -lpthread -lg2d -o vt-ipu -g3 -D_GNU_SOURCE
}

do_install() {
	install -d ${D}${bindir}
  install -m 0755 vt-ipu ${D}${bindir}
}
