QT -= gui

TARGET = qtlibusb
TEMPLATE = lib

#
# Boilerplate
#
BUILD_DIR = ../../build/src
DESTDIR = ../../lib
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
PRECOMPILED_DIR = $$BUILD_DIR

CONFIG += module create_prl

#
# Project settings
#
DEFINES += QT_BUILD_LIBUSB_LIB

#QMAKE_MOC = $$QMAKE_MOC -nw     # Make MOC shut up about non-QObject classes

include(libusb-lib.pri)

#
# Deploy
#
isEmpty(QTLIBUSB_INSTALL_PREFIX) {  # If the user had set this, honor that
    QTLIBUSB_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]
    unix {
        QTLIBUSB_INSTALL_PREFIX = /usr/local
    }
}

headers.files = $${PUBLIC_HEADERS}
headers.path = $${QTLIBUSB_INSTALL_PREFIX}/include/qusb
target.path = $${QTLIBUSB_INSTALL_PREFIX}/lib

INSTALLS += headers target