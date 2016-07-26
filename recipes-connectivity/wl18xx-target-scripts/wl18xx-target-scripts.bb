DESCRIPTION = "Scripts and configuration files for TI wireless drivers"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://README;beginline=1;endline=21;md5=b8d6a0865f50159bf5c0d175d1f4a705"

PV = "R8.7+git${SRCPV}"

PR = "r7"

# Tag: R8.7
SRCREV = "R8.7"
BRANCH = "R8.7"

SRC_URI = "git://git.ti.com/wilink8-wlan/wl18xx-target-scripts.git;protocol=git;tag=${BRANCH};nobranch=1 \
           file://0001-fix-system-bin.patch"

S = "${WORKDIR}/git"

FILES_${PN} += "${datadir}/wl18xx/"
RDEPENDS_${PN} += " bash "

do_install() {
	install -d ${D}${datadir}/wl18xx/
	cd ${S}
        find ./ -type f -name '*.*' ! -name "print_stat.sh" | while read f

#	scripts="`find ./ -type f -name "*.*"`"
#	for f in $scripts
	do
		install -m 0755 $f ${D}${datadir}/wl18xx/
	done
}
