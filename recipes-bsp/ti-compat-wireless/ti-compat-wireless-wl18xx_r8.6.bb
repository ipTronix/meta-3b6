# This is a TI specific version of the compat-wireless recipe using a
# compat-wireless package created from the TI Systems Tested mac80211 releases.

DESCRIPTION = "ti compat-wireless drivers for wl18xx"
HOMEPAGE = "https://git.ti.com/wilink8-wlan/wl18xx"
SECTION = "kernel/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

RDEPENDS_${PN} = "wireless-tools"

PV = "r8.6"
inherit module

PR = "${PV}"

SRCREV_wl18xx = "3f5b34f607645d94c7ff5c5a01f93b8ba59e5574"
BRANCH_wl18xx = "upstream_41"
SRCREV_backports = "4677dc3f5d23242060a8ddc33e38838b2430ff61"
BRANCH_backports = "upstream_41"

SRCREV_FORMAT = "wl18xx"

S = "${WORKDIR}/compat-wireless"

SRC_URI = "git://git.ti.com/wilink8-wlan/wl18xx.git;branch=${BRANCH_wl18xx};destsuffix=wl18xx;name=wl18xx \
           git://git.ti.com/wilink8-wlan/backports.git;branch=${BRANCH_backports};destsuffix=backports;name=backports \
 "

export KLIB_BUILD="${STAGING_KERNEL_DIR}"
export KLIB="${D}"
export KCFLAGS="-O2 -pipe -g -feliminate-unused-debug-types -Wno-error=date-time"

do_configure() {
    cd "${WORKDIR}/backports"
    unset CC
    #Generate compat-wireless
    python ./gentree.py --clean  "${WORKDIR}/wl18xx" "${WORKDIR}/compat-wireless"

    cd ${S}
    
    KLIB_BUILD=${KBUILD_OUTPUT} make -d defconfig-wl18xx

    #KCFLAGS="-O2 -pipe -g -feliminate-unused-debug-types -Wno-error=date-time" KLIB_BUILD=${KBUILD_OUTPUT} oe_runmake 
}

do_compile() {
    cd ${S}

    KCFLAGS="${KCFLAGS} -Wno-error=date-time" KLIB_BUILD=${KBUILD_OUTPUT} oe_runmake
}

do_install() {
    cd ${S}
    # Install modules
    KCFLAGS="${KCFLAGS} -Wno-error=date-time" KLIB_BUILD=${KBUILD_OUTPUT} oe_runmake modules_install
}

