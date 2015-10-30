SUMMARY = "A small image just capable of allowing a device to boot and use qt5."
#require core-image-minimal-common.inc

IMAGE_INSTALL = "packagegroup-core-boot ${ROOTFS_PKGMANAGE_BOOTSTRAP} ${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_FEATURES += "\
    ${@base_contains('DISTRO_FEATURES', 'x11', 'x11-base', '', d)} \
"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

IMAGE_ROOTFS_SIZE ?= "8192"

EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh "

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-fsl-tools-gpu \
    packagegroup-fslc-gstreamer1.0 \
    ${@base_contains('DISTRO_FEATURES', 'directfb', 'packagegroup-core-directfb', '', d)} \
    ${@base_contains('DISTRO_FEATURES', 'x11', '', \
                      base_contains('DISTRO_FEATURES', 'wayland', \
                                    'weston weston-init weston-examples \
                                         gtk+3-demo clutter-1.0-examples', '', d), d)} \
"

PACKAGE_INSTALL_append = " gcc g++ binutils libgcc libgcc-dev libstdc++ libstdc++-dev libstdc++-staticdev \
    autoconf automake ccache chkconfig glib-networking glibmm \
    packagegroup-core-buildessential pkgconfig  \
    boost cmake zlib glib-2.0 packagegroup-fsl-tools-testapps  \
    ruby \
    cpufrequtils \
    nano \
    gdb \
    gstreamer \
    gst-meta-video \
    gst-fsl-plugin \
    gst-plugins-base-app \
    gst-plugins-base \
    gst-plugins-good \
    gst-plugins-good-rtsp \
    gst-plugins-good-udp \
    gst-plugins-good-rtpmanager \
    gst-plugins-good-rtp \
    gst-plugins-good-video4linux2 \
    openssh-sftp-server \
    packagegroup-fsl-gstreamer \
    packagegroup-fsl-tools-testapps \
    packagegroup-fsl-tools-benchmark \
    imx-vpu \
    imx-test \    	
    qtbase-fonts \
    qtbase-plugins \
    qtbase-tools \
    qtbase-examples \
    qtdeclarative \
    qtdeclarative-plugins \
    qtdeclarative-tools \
    qtdeclarative-examples \
    qtdeclarative-qmlplugins \
    qtmultimedia \
    qtmultimedia-plugins \
    qtmultimedia-examples \
    qtmultimedia-qmlplugins \
    qtsvg \
    qtsvg-plugins \
    qtsensors \
    qtimageformats-plugins \
    qtsystems \
    qtsystems-tools \
    qtsystems-examples \
    qtsystems-qmlplugins \
    qtscript \
    qt3d \
    qt3d-examples \
    qt3d-qmlplugins \
    qt3d-tools \
    qtwebkit \
    qtwebkit-examples-examples \
    qtwebkit-qmlplugins \
    qtgraphicaleffects-qmlplugins \
    qtconnectivity-qmlplugins \
    qtlocation-plugins \
    qtlocation-qmlplugins \
    cinematicexperience \
    cairo pango fontconfig freetype pulseaudio dbus \
    alsa-lib alsa-tools alsa-state alsa-utils-alsaconf fsl-alsa-plugins \
    i2c-tools \ 
    "

DISTRO_FEATURES_remove = "x11 wayland"
PACKAGECONFIG_remove = " x11 wayland"

inherit core-image

