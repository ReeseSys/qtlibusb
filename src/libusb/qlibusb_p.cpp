

#include "qlibusb.h"
#include "qlibusb_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

int QLibUsbPrivate::readInterval = 250;
int QLibUsbPrivate::readRespLen = 1024;

QLibUsbPrivateReader::QLibUsbPrivateReader(QLibUsbPrivate *ulp)
    : p(ulp)
    , readResp(nullptr)
    , running(false)
{
}

QLibUsbPrivateReader::~QLibUsbPrivateReader()
{
    if (nullptr != readResp) {
        delete readResp;
    }
}

void QLibUsbPrivateReader::start()
{
    if (readResp != nullptr) {
        delete readResp;
    }

    running = true;
    readResp = new unsigned char[QLibUsbPrivate::readRespLen];
    qDebug() << "qtlibusb: Start reading every" << QLibUsbPrivate::readInterval << "ms";
    emit started();
    while (running) {
        readInData();
    }
}

void QLibUsbPrivateReader::stop()
{
    running = false;
    emit finished();
}

void QLibUsbPrivateReader::readInData()
{
    respMutex.lock();

    int len = QLibUsbPrivate::readRespLen;
    int transfer_size = 0;
    int err;

    memset(readResp, 0, len);
    err = libusb_bulk_transfer(p->device(), p->endpointIn(), readResp, len, &transfer_size, QLibUsbPrivate::readInterval);
    if (err == LIBUSB_ERROR_TIMEOUT) {
        // nothing to read
        respMutex.unlock();
        return;
    }
    if (err) {
        qDebug() << "qtlibusb: Bulk IN transfer failed:" << p->errorString(err);
        respMutex.unlock();
        return;
    }
    for (int i = 0; i < transfer_size; ++i) {
        // qDebug(" %02d ", readResp[i]);
        if (readResp[i] == 13) {
            qDebug() << "qtlibusb: Response line: " << respData;
            emit haveNewResponseData(respData);
            respData.clear();
        } else {
            respData += readResp[i];
        }
    }

    respMutex.unlock();
}

QLibUsbPrivate::QLibUsbPrivate(QLibUsb *q)
    : QObject(q)
    , q_ptr(q)
    , reader(nullptr)
    , readerThread(nullptr)
    , vId(0)
    , pId(0)
    , iface(0)
	, epIn(0)
    , epOut(0)
    , ctx(nullptr)
    , dev(nullptr)
{
}

QLibUsbPrivate::~QLibUsbPrivate()
{
    stopReading();
}

void QLibUsbPrivate::setVendorId(quint16 id)
{
    if (vId != id) {
        if (dev != nullptr) {
            // already have open device with different vendor Id, close it
            libusb_close(dev);
        }
        vId = id;
    }
}

quint16 QLibUsbPrivate::vendorId() const
{
    return vId;
}

void QLibUsbPrivate::setProductId(quint16 id)
{
    if (pId != id) {
        if (dev != nullptr) {
            // already have open device with different product Id, close it
            libusb_close(dev);
        }
    }
    pId = id;
}

quint16 QLibUsbPrivate::productId() const
{
    return pId;
}

void QLibUsbPrivate::setInterface(int interface)
{
    if (0 != interface && iface != interface) {
        iface = interface;
        claimInterface();
    }
}

int QLibUsbPrivate::interface() const
{
    return iface;
}

void QLibUsbPrivate::setEndpointIn(uchar endpoint)
{
	epIn = endpoint | LIBUSB_ENDPOINT_IN;
}

uchar QLibUsbPrivate::endpointIn() const
{
	return epIn;
}

void QLibUsbPrivate::setEndpointOut(uchar endpoint)
{
	epOut = endpoint | LIBUSB_ENDPOINT_OUT;
}

uchar QLibUsbPrivate::endpointOut() const
{
	return epOut;
}

QString QLibUsbPrivate::errorString(int err)
{
    return libusb_error_name(err);
}

bool QLibUsbPrivate::open()
{
    libusb_init(&ctx);
//    libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_DEBUG);

    dev = libusb_open_device_with_vid_pid(NULL, vId, pId);

    if (nullptr == dev) {
        qDebug("qtlibusb: Device not found, cannot open: (%04x:%04x)", vId, pId);
        return false;
    }

    claimInterface();

    return true;
}

void QLibUsbPrivate::close()
{
    if (dev != nullptr) {
        stopReading();
        libusb_close(dev);
        dev = nullptr;
    }
}

bool QLibUsbPrivate::claimInterface()
{
    if (dev != nullptr && 0 != iface) {
        int err = libusb_claim_interface(dev, iface);
        if (err) {
            qDebug() << "qtlibusb: Cannot claim interface (" << iface << ")" << errorString(err);
            return false;
        }

        err = libusb_set_interface_alt_setting(dev, iface, 0);
        if (err) {
            qDebug() << "qtlibusb: Cannot claim alt interface (" << iface << ")" << errorString(err);
            return false;
        }

        qDebug("qtlibusb: Claimed inteface %d (%04x:%04x)", iface, vId, pId);
    }

    return true;
}

void QLibUsbPrivate::startReading()
{
    if (nullptr == reader) {
        if (nullptr != readerThread) {
            delete readerThread;
        }
        readerThread = new QThread();
        reader = new QLibUsbPrivateReader(this);
        reader->moveToThread(readerThread);

        connect(readerThread, SIGNAL(started()), reader, SLOT(start()));
        connect(reader, SIGNAL(haveNewResponseData(QByteArray)), this, SLOT(handleReadData(QByteArray)));

        connect(reader, SIGNAL(finished()), reader, SLOT(deleteLater()));
        connect(reader, SIGNAL(finished()), this, SLOT(readerFinished()));
        connect(readerThread, SIGNAL(finished()), readerThread, SLOT(deleteLater()));

        readerThread->start();
    }
}

void QLibUsbPrivate::stopReading()
{
    if (nullptr != reader) {
        reader->stop();
    }
}

void QLibUsbPrivate::readerFinished()
{
    // qDebug() << "QLibUsbPrivate readerFinished";
}

bool QLibUsbPrivate::hasResponseData() const
{
    return !respQueue.isEmpty();
}

qint64 QLibUsbPrivate::responseQueueSize() const
{
    return respQueue.size();
}

qint64 QLibUsbPrivate::responseSize() const
{
    if (respQueue.isEmpty()) {
        return 0;
    }

    return respQueue.head().size();
}

QByteArray QLibUsbPrivate::responseData()
{
    if (respQueue.isEmpty()) {
        return "";
    }

    return respQueue.dequeue();
}

qint64 QLibUsbPrivate::responseData(char* data, qint64 maxSize)
{
    if (respQueue.isEmpty()) {
        return 0;
    }

    QByteArray resp = respQueue.dequeue();
    int len = qMin((qint64)resp.size(), maxSize);
    strncpy(data, resp.data(), len);

    return len;
}

qint64 QLibUsbPrivate::writeData(const char *data, qint64 maxSize)
{
    int transferSize = 0;
    int err = libusb_bulk_transfer(dev, epOut, (uchar *)data, maxSize, &transferSize, QLibUsbPrivate::readInterval);
    if (err) {
        qDebug() << "qtlibusb: Bulk OUT transfer failed:" << errorString(err);
    }

    return transferSize;
}

void QLibUsbPrivate::handleReadData(const QByteArray &data)
{
    respQueue.enqueue(data);
    emit newInData();
}
