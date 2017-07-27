

#include "qlibusb.h"
#include "qlibusb_p.h"

#include <QtCore/qdebug.h>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE

QLibUsb::QLibUsb(QObject *parent)
	: QIODevice(parent)
	, d_ptr(new QLibUsbPrivate(this))
{}

QLibUsb::QLibUsb(const quint16 vendorId, const quint16 productId, int interface, QObject *parent)
	: QIODevice(parent)
    , d_ptr(new QLibUsbPrivate(this))
    , openMode(QIODevice::NotOpen)
{
	setVendorId(vendorId);
	setProductId(productId);
	setInterface(interface);
}

QLibUsb::~QLibUsb()
{
    close();
    delete d_ptr;
}

void QLibUsb::setVendorId(quint16 vendorId)
{
	Q_D(QLibUsb);
    d->setVendorId(vendorId);
}

quint16 QLibUsb::vendorId() const
{
	Q_D(const QLibUsb);
    return d->vendorId();
}

void QLibUsb::setProductId(quint16 productId)
{
	Q_D(QLibUsb);
    d->setProductId(productId);
}

quint16 QLibUsb::productId() const
{
	Q_D(const QLibUsb);
    return d->productId();
}

void QLibUsb::setInterface(int interface)
{
    Q_D(QLibUsb);
    if (0 != interface) {
        d->setInterface(interface);
    }
}

int QLibUsb::interface() const
{
	Q_D(const QLibUsb);
    return d->interface();
}

void QLibUsb::setEndpointIn(uchar endpoint)
{
	Q_D(QLibUsb);
    if (0 != endpoint) {
        d->setEndpointIn(endpoint);
    }
}

uchar QLibUsb::endpointIn() const
{
	Q_D(const QLibUsb);
	return d->endpointIn();
}

void QLibUsb::setEndpointOut(uchar endpoint)
{
	Q_D(QLibUsb);
    if (0 != endpoint) {
        d->setEndpointOut(endpoint);
    }
}

uchar QLibUsb::endpointOut() const
{
	Q_D(const QLibUsb);
	return d->endpointOut();
}

// QIODevice overrides

bool QLibUsb::isSequential() const
{
    return true;
}

qint64 QLibUsb::bytesAvailable() const
{
    Q_D(const QLibUsb);
    return d->responseSize() + QIODevice::bytesAvailable();
}

bool QLibUsb::canReadLine() const
{
    Q_D(const QLibUsb);
    return d->responseQueueSize() > 0 || QIODevice::canReadLine();
}

bool QLibUsb::waitForReadyRead(int msecs)
{
    Q_D(QLibUsb);
    QElapsedTimer stopWatch;
    stopWatch.start();

    do {
        if (d->hasResponseData()) {
            // qDebug() << "waitForReadyRead has response data";
            return true;
        }

    } while (msecs == -1 || !stopWatch.hasExpired(msecs));

    return false;
}

bool QLibUsb::open(QIODevice::OpenMode mode)
{
    QIODevice::open(mode);

    Q_D(QLibUsb);
    bool opened = d->open();
    if (opened) {
        openMode = mode;
        writeTimer.setInterval(1000);
        connect(&writeTimer, SIGNAL(timeout()), SLOT(unlockWrite()));
    }

    if (opened && mode | QIODevice::ReadOnly) {
        if (d->endpointIn() == 0) {
            qDebug() << "qtlibusb: Opened as reader but no IN endpoint set";
        } else {
            // set up listener
            // qDebug() << "QLibUsb listening...";

            Q_D(QLibUsb);
            connect(d, SIGNAL(newInData()), SLOT(handleInData()));

            d->startReading();
        }
    }

    return opened;
}

void QLibUsb::close()
{
    QIODevice::close();

    Q_D(QLibUsb);
    if (openMode != QIODevice::NotOpen) {
        d->close();
        openMode = QIODevice::NotOpen;
    }
}


void QLibUsb::handleInData()
{
    // qDebug() << "QLibUsb handleInData";
    emit readyRead();
}

void QLibUsb::unlockWrite()
{
    writeMutex.unlock();
    if (!writeQueue.isEmpty()) {
        QByteArray data(writeQueue.dequeue());
        writeData(data.constData(), data.length());
    }
}

// QIODevice pure virtual methods

qint64 QLibUsb::readData(char *data, qint64 maxlen)
{
    Q_D(QLibUsb);

    qint64 len = 0;
    if (d->hasResponseData()) {
        len = d->responseData(data, maxlen);
        // qDebug() << "QLibUsb readData:" << QString(data) << len;
    }

    // unlock for write, assume have response to a write or will soon
    QTimer::singleShot(QLibUsbPrivate::readInterval, this, SLOT(unlockWrite()));
    return len;
}

qint64 QLibUsb::writeData(const char *data, qint64 len)
{
    Q_D(QLibUsb);
    // qDebug() << "QLibUsb writeData:" << QString(data) << len;

    qint64 lenWritten = 0;
    if (writeMutex.tryLock(QLibUsbPrivate::readInterval)) {
        // can write data now
        lenWritten = d->writeData(data, len);
        writeTimer.start();
    } else {
        // queue up the write for later
        writeQueue.enqueue(QByteArray(data, len));

        // tell caller we did write the data
        lenWritten = len;
    }

    return lenWritten;
}
