#include customization in core-image-minimal-common.inc
require core-image-minimal-common.inc

#add further recipes for development (e.g. ssh connectivity)
PACKAGE_INSTALL += " hid-test i2c-tools memtool "
#PACKAGE_INSTALL += " packagegroup-core-ssh-openssh yavta "