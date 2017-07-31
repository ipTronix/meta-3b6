SUMMARY = "SysV init scripts"
DESCRIPTION = "Initscripts override"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe"
PR = "r155"

do_install_append () {
#
# Create directories and install device independent scripts
#
	update-rc.d -f -r ${D} banner.sh remove
    update-rc.d -f -r ${D} save-rtc.sh remove
}