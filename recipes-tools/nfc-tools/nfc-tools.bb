DESCRIPTION = "Public platform independent Near Field Communication (NFC) library"
LICENSE = "GNU"
LIC_FILES_CHKSUM = "file://COPYING;md5=b52f2d57d10c4f7ee67a7eb9615d5d24"
#;beginline=1;endline=34;"

PR = "r1"
PV = "1.0"

SRCREV = "42d455c4e70e3ff699a49b18b5c88d11e40ca245"
SRC_URI = "git://github.com/nfc-tools/libnfc.git \
           file://libnfc.conf"

SRC_URI[md5sum] = "d4863cf59ec27a87f64e3adcaaa936ca"
SRC_URI[sha256sum] = "67a169b822c3f0f8ba41785471050c29bd2d385e6e896aa6fc7b5d3912c7f468"

S = "${WORKDIR}/git"

do_configure() {
        autoreconf -vis
        ./configure \
                --with-drivers=pn532_i2c \
                --host=${TARGET_SYS} \
                --build=${BUILD_SYS} \
                --prefix=${D} \
                --sysconfdir=/etc
                #--enable-debug \
                #--enable-doc \

}

do_compile() {
        oe_runmake
}

do_install() {
	oe_runmake install
	install -d ${D}/etc
	install -d ${D}/etc/nfc
	install -m 0644 ${WORKDIR}/libnfc.conf ${D}/etc/nfc
	rm -rf ${D}/share
	rm -rf ${D}/include
	rm -rf ${D}/lib/pkgconfig
}
