#include <QtTest/QtTest>

#include "qlibusb.h"
#include "libusb-1.0/libusb.h"

class tst_Basic : public QObject
{
    Q_OBJECT
public:
    explicit tst_Basic();

private slots:
    void initTestCase();

    void constructors();
    void endpoints();
    void openNoExist();
    void openLaunchPadCC1350();

private:
    quint16 m_vendorId;
    quint16 m_productId;
    int m_interface;
    uchar m_epIn;
    uchar m_epOut;
};

tst_Basic::tst_Basic()
 : m_vendorId(0)
 , m_productId(0)
 , m_interface(0)
{
}

void tst_Basic::initTestCase()
{
	m_vendorId = 0x1234;
	m_productId = 0x5678;
	m_interface = 1;
	m_epIn = 0x02;
	m_epOut = 0x01;
}

void tst_Basic::constructors()
{
	QLibUsb empty;

    QLibUsb vendProd(m_vendorId, m_productId, m_interface);
    QCOMPARE(m_vendorId, vendProd.vendorId());
    QCOMPARE(m_productId, vendProd.productId());
    QCOMPARE(m_interface, vendProd.interface());
}

void tst_Basic::endpoints()
{
	QLibUsb l;

	l.setEndpointIn(m_epIn);
	l.setEndpointOut(m_epOut);
	QCOMPARE((uchar)(m_epIn | LIBUSB_ENDPOINT_IN), l.endpointIn());
	QCOMPARE((uchar)(m_epOut | LIBUSB_ENDPOINT_OUT), l.endpointOut());
}

void tst_Basic::openNoExist()
{
    QLibUsb vendProd(m_vendorId, m_productId);
    QCOMPARE(vendProd.open(QIODevice::ReadWrite), false);
}

void tst_Basic::openLaunchPadCC1350()
{
    quint16 v = 0x0451;
    quint16 p = 0xbef3;
    int iface = 1;

    QLibUsb ti(v, p, iface);
    QCOMPARE(ti.open(QIODevice::ReadWrite), true);

    if (ti.waitForReadyRead(20000)) {
        qDebug() << "reading data";
        int maxlen = 1024;
        char * data = new char[maxlen];
        int len = ti.read(data, maxlen);
        qDebug() << "read" << len << data;
    } else {
        qDebug() << "no data read";
    }

}

QTEST_MAIN(tst_Basic)
#include "tst_basic.moc"
