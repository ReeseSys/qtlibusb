INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/qlibusb.cpp \
    $$PWD/qlibusb_p.cpp

HEADERS += \
    $$PWD/qlibusb.h \
    $$PWD/qlibusb_p.h \
    $$PWD/qlibusbglobal.h

PUBLIC_HEADERS = \
    $$PWD/qlibusb.h

isEmpty($(LIBUSB_PREFIX)) {
	INCLUDEPATH += /usr/local/include
	LIBS += -L/usr/local/lib
}
!isEmpty(!$(LIBUSB_PREFIX)) {		# User defined LIBUSB_PREFIX
    INCLUDEPATH += $(LIBUSB_PREFIX)/include
    LIBS += -L$(LIBUSB_PREFIX)/lib
}
LIBS += -lusb-1.0
