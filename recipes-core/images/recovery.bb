SUMMARY = "A small image just capable of allowing a device to boot and update system."

IMAGE_INSTALL = "packagegroup-core-boot ${ROOTFS_PKGMANAGE_BOOTSTRAP} ${CORE_IMAGE_EXTRA_INSTALL} recovery-script"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit image_types_uboot
IMAGE_FSTYPES += "cpio.gz.u-boot"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"
IMAGE_ROOTFS_EXTRA_SPACE_append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "" ,d)}"
