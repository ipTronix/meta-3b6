DESCRIPTION = "dev Image Frame Buffer Image QT5"

LICENSE = "MIT"

inherit core-image

IMAGE_FSTYPES = "tar.bz2"

IMAGE_INSTALL_append = " \
    firmware-imx-vpu-imx6q \
    firmware-imx-vpu-imx6d \
"

IMAGE_OVERHEAD_FACTOR = "2.0"

export IMAGE_BASENAME = "dev-fb-qt5"

EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh tools-testapps "

IMAGE_INSTALL_append = " binutils libgcc libstdc++ \
    ccache chkconfig glib-networking glibmm \
    packagegroup-core-buildessential pkgconfig  \
    boost cmake zlib glib-2.0 \
    fpga fram \
    canutils libsocketcan \
    nfc-tools \
    iproute2 \
    packagegroup-qt5-qtcreator-debug \
    cpufrequtils \
    gdb \
    gstreamer1.0 \
    gstreamer1.0-plugins-imx \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-meta-base \
    gstreamer1.0-rtsp-server \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-base \
    openssh-sftp-server \
    imx-vpu \
    imx-test \
    qtbase-fonts \
    qtbase-plugins \
    qtbase-tools \
    qtdeclarative \
    qtdeclarative-plugins \
    qtdeclarative-tools \
    qtdeclarative-qmlplugins \
    qtmultimedia \
    qtmultimedia-plugins \
    qtmultimedia-qmlplugins \
    qtsvg \
    qtquick1 \
    qtquickcontrols-qmlplugins \
    qtsvg-plugins \
    qtimageformats-plugins \
    qtsystems \
    qtsystems-tools \
    qtsystems-qmlplugins \
    qtwebkit \
    qtwebkit-examples-examples \
    qtwebkit-qmlplugins \
    qtscript \
    qtgraphicaleffects-qmlplugins \
    qtconnectivity-qmlplugins \
    cairo pango fontconfig freetype pulseaudio dbus \
    alsa-lib alsa-tools alsa-state fsl-alsa-plugins \
    tslib evtest tslib-conf tslib-tests tslib-calibrate \
    i2c-tools \
    mtd-utils imx-kobs mtd-utils-ubifs \
    e2fsprogs \
    cinematicexperience \
    script-install \
    lpc-com \
    packagegroup-wilink-connectivity \
    packagegroup-fsl-tools-gpu \
    packagegroup-fsl-tools-gpu-external \
    packagegroup-fsl-tools-testapps \
    packagegroup-fsl-tools-benchmark \
    bluetopia \
    "

DISTRO_FEATURES_remove = "x11 wayland"

removefiles_postprocess_function() {
   rm -rf ${IMAGE_ROOTFS}/etc/timestamp
}

ROOTFS_POSTPROCESS_COMMAND_append = " \
  removefiles_postprocess_function; \
"
