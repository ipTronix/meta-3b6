SUMMARY = "Library which allows userspace application access to NFC devices."
HOMEPAGE = "http://nfc-tools.org/index.php?title=Libnfc"
SECTION = "libs"
LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=b52f2d57d10c4f7ee67a7eb9615d5d24"

DEPENDS = "libusb-compat"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

#SRCREV = "bc8bb8de895688af520909ee1e16d243d71abddf"
#SRC_URI = "git://github.com/nfc-tools/libnfc.git"

SRCREV = "42d455c4e70e3ff699a49b18b5c88d11e40ca245"
SRC_URI = "git://github.com/nfc-tools/libnfc.git \
           file://libnfc.conf"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

# The autotools configuration I am basing this on seems to have a problem with a race condition when parallel make is enabled
PARALLEL_MAKE = ""

EXTRA_OECONF_append = "--with-drivers=pn532_i2c \
                       --host=${TARGET_SYS} "
 
