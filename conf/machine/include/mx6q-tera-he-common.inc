# Provides the i.MX6 Tera-he common settings

require conf/machine/include/imx-base.inc
require conf/machine/include/tune-cortexa9.inc

SERIAL_CONSOLE = "115200 ttymxc1"

MACHINE_FIRMWARE_append_mx6 = " linux-firmware-ath6k"

MACHINE_FEATURES += " pci wifi bluetooth"
