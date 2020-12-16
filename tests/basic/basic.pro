QT = core testlib
TARGET = tst_basic
#CONFIG += testcase

include (../../src/libusb/libusb-lib.pri)

SOURCES += tst_basic.cpp

QMAKE_CXXFLAGS += -std=c++11
