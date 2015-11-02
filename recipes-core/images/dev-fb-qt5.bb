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

EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh "

IMAGE_INSTALL_append = " binutils libgcc libstdc++ \
ccache chkconfig glib-networking glibmm \
packagegroup-core-buildessential pkgconfig  \
boost cmake zlib glib-2.0 \
    packagegroup-qt5-qtcreator-debug \    
    cpufrequtils \
    gdb \
    gstreamer \
    gst-meta-video \
    gst-plugins-base-app \
    gst-plugins-base \
    gst-plugins-good \
    gst-plugins-good-rtsp \
    gst-plugins-good-udp \
    gst-plugins-good-rtpmanager \
    gst-plugins-good-rtp \
    gst-plugins-good-video4linux2 \
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
    qtsvg-plugins \
    qtimageformats-plugins \
    qtsystems \
    qtsystems-tools \
    qtsystems-qmlplugins \
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
    "

DISTRO_FEATURES_remove = "x11 wayland"
