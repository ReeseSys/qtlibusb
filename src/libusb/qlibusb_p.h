
#ifndef QLIBUSB_P_H
#define QLIBUSB_P_H

#include <QString>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QThread>

#include "qlibusb.h"

#include "libusb-1.0/libusb.h"


QT_BEGIN_NAMESPACE

class QLibUsb;

class QLibUsbPrivateReader : public QObject
{
    Q_OBJECT
    QLibUsbPrivate *p;

public:
    QLibUsbPrivateReader(QLibUsbPrivate *ulp);
    ~QLibUsbPrivateReader();

Q_SIGNALS:
    void haveNewResponseData(const QByteArray &data);
    void started();
    void finished();

public Q_SLOTS:
    void start();
    void stop();

protected Q_SLOTS:
    void readInData();

private:
    unsigned char *readResp;
    QByteArray respData;
    QTimer readTimer;
    QMutex respMutex;
    bool running;
};

class QLibUsbPrivate : public QObject
{
    Q_OBJECT
	Q_DECLARE_PUBLIC(QLibUsb)

public:
    QLibUsbPrivate(QLibUsb *p);
    ~QLibUsbPrivate();

    void setVendorId(quint16 id);
    quint16 vendorId() const;

    void setProductId(quint16 id);
    quint16 productId() const;

    void setInterface(int interface);
    int interface() const;

    void setEndpointIn(uchar endpoint);
	uchar endpointIn() const;

	void setEndpointOut(uchar endpoint);
    uchar endpointOut() const;

    libusb_device_handle * device() { return dev; }

    QString errorString(int err);

    bool open();
    void close();
    bool claimInterface();
    void startReading();
    void stopReading();
    bool hasResponseData() const;
    qint64 responseQueueSize() const;
    qint64 responseSize() const;
    QByteArray responseData();
    qint64 responseData(char* data, qint64 maxSize);
    qint64 writeData(const char *data, qint64 maxSize);

Q_SIGNALS:
    void newInData();

public Q_SLOTS:
    void handleReadData(const QByteArray &data);
    void readerFinished();

private:
    QLibUsb *q_ptr;
    QLibUsbPrivateReader *reader;
    QThread *readerThread;

    QQueue<QByteArray> respQueue;
    QMutex respMutex;

protected:
    quint16 vId;
    quint16 pId;
    int iface;
    uchar epIn;
    uchar epOut;

    libusb_context *ctx;
    libusb_device_handle *dev;

public:
    static int readInterval;
    static int readRespLen;

};

QT_END_NAMESPACE
#endif
