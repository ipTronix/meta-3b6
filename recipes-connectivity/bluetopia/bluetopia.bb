SUMMARY = "TI Bluetopia stack"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYRIGHT;md5=d28a19ddc73e1eb786a2dae60b3104dc"

SRC_URI = "file://bluetopia.tar.gz \
           file://COPYRIGHT \
	        "
S = "${WORKDIR}"

FILES_${PN} = "BluetopiaPM"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

INSANE_SKIP_${PN} = "ldflags"

do_install () {

	cd ${S}
	install -d BluetopiaPM

	find BluetopiaPM -type d -name '*' ! -name '*.debug*' | while read f
	do
		install -d ${D}/${f}
	done
	
	find BluetopiaPM -type f -name '*' ! -name '*.debug*' ! -name '*.a' ! -name '*.c' ! -name '*.h' ! -name '*.mak' | while read f
	do
		install ${f} ${D}/${f}
	done
}


