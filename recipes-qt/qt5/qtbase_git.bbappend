# TERA-HE

DESCRIPTION = "patches to compile qt"

FILESEXTRAPATHS_prepend := "${THISDIR}/qtbase:"

SRC_URI_append_tera-he = " \
    file://000_fix_compilation_error.patch \
    "

