DESCRIPTION = "Scripts and configuration files for TI wireless drivers"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://README;beginline=1;endline=21;md5=b8d6a0865f50159bf5c0d175d1f4a705"

PV = "R8.6+git${SRCPV}"

PR = "r7"

# Tag: R8.6
SRCREV = "693f71c709abcaa9047835cf3903aea26a63fa94"
BRANCH = "sitara-scripts"
SRC_URI = "git://git.ti.com/wilink8-wlan/wl18xx-target-scripts.git;protocol=git;branch=${BRANCH}\
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
