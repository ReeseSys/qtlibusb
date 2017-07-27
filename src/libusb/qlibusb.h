
#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QtCore/qiodevice.h>
#include <QtCore/qmutex.h>
#include <QtCore/qqueue.h>
#include <QtCore/qtimer.h>

#include <qlibusbglobal.h>

#define QIODEVICE_DEBUG

QT_BEGIN_NAMESPACE

class QLibUsbPrivate;

class QT_LIBUSB_EXPORT QLibUsb : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLibUsb)

public:

    explicit QLibUsb(QObject *parent = Q_NULLPTR);
    explicit QLibUsb(const quint16 vendorId, const quint16 productId, int interface = 0, QObject *parent = Q_NULLPTR);
    virtual ~QLibUsb();

    void setVendorId(quint16 vendorId);
    quint16 vendorId() const;

    void setProductId(quint16 productId);
    quint16 productId() const;

    void setInterface(int interface);
    int interface() const;

    void setEndpointIn(uchar endpoint);
    uchar endpointIn() const;

    void setEndpointOut(uchar endpoint);
    uchar endpointOut() const;

    // QIODevice overrides
    virtual bool isSequential() const;
    virtual qint64 bytesAvailable() const;
//    virtual qint64 bytesToWrite() const;
    virtual bool canReadLine() const;
    virtual bool waitForReadyRead(int msecs);
    virtual bool open(QIODevice::OpenMode mode);
    virtual void close();

protected Q_SLOTS:
    void handleInData();
    void unlockWrite();


protected:
    // QIODevice pure virtual methods
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    QLibUsbPrivate * const d_ptr;
    QIODevice::OpenMode openMode;
    QQueue<QByteArray> writeQueue;
    QTimer writeTimer;
    QMutex writeMutex;
};

#endif
