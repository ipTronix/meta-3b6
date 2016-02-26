
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
		file://000-add-device-to-tvin.patch \
		file://001-add-capture-whi-to-tvin.patch \
	 "

