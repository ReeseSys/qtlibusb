#include "dialog.h"
#include "ui_dialog.h"

#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    usb(new QLibUsb(this))
{
    ui->setupUi(this);

    ui->addNewLine->setChecked(true);
    ui->vendorId->setText("<hex vendor id>");
    ui->productId->setText("<hex product id>");
    ui->iface->setValue(1);

    connect(usb, SIGNAL(readyRead()), SLOT(usbReadyRead()));
}

Dialog::~Dialog()
{
    delete ui;
    if (usb != nullptr) {
        usb->close();
        delete usb;
    }
}

void Dialog::on_connect_clicked()
{
    QString vendorId(ui->vendorId->text());
    QString productId(ui->productId->text());
    int iface = ui->iface->value();

    bool ok;
    quint16 vid = vendorId.toUInt(&ok, 16);
    quint16 pid = productId.toUInt(&ok, 16);
    qDebug() << "Connecting to" << vid << pid << iface;

    usb->setVendorId(vid);
    usb->setProductId(pid);
    usb->setInterface(iface);
    usb->setEndpointOut(1);
    usb->setEndpointIn(2);

    if (usb->open(QIODevice::ReadWrite)) {

    } else {
        qDebug() << "Failed to open USB";
    }

}

void Dialog::on_send_clicked()
{
    qDebug() << "Sending" << ui->sendData->text();
    QByteArray sendData;
    sendData.append(ui->sendData->text());
    if (ui->addNewLine->isChecked()) {
        sendData.append('\r');
    }
    usb->write(sendData);
}

void Dialog::on_sendData_returnPressed()
{

}

void Dialog::on_done_clicked()
{
    close();
}

void Dialog::usbReadyRead()
{
    QByteArray data(usb->readAll());
    ui->receiveData->addItem(data.constData());
    ui->receiveData->setCurrentRow(ui->receiveData->count() - 1);
}
