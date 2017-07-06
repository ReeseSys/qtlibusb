#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "qlibusb.h"
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

public Q_SLOTS:

private slots:
    void on_done_clicked();

    void on_connect_clicked();

    void on_send_clicked();

    void on_sendData_returnPressed();

    void usbReadyRead();

private:
    Ui::Dialog *ui;
    QLibUsb *usb;
};

#endif // DIALOG_H
