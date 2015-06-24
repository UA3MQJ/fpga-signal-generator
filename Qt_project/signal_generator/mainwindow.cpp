#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QByteArray>
#include <QDataStream>
#include <QKeyEvent>

QString nowStr()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    freq = 1000;

    calculate();

    ui->e10000000->installEventFilter(this);
    ui->e1000000->installEventFilter(this);
    ui->e100000->installEventFilter(this);
    ui->e10000->installEventFilter(this);
    ui->e1000->installEventFilter(this);
    ui->e100->installEventFilter(this);
    ui->e10->installEventFilter(this);
    ui->e1->installEventFilter(this);

    ui->pbM10000000->installEventFilter(this);
    ui->pbM1000000->installEventFilter(this);
    ui->pbM100000->installEventFilter(this);
    ui->pbM10000->installEventFilter(this);
    ui->pbM1000->installEventFilter(this);
    ui->pbM100->installEventFilter(this);
    ui->pbM10->installEventFilter(this);
    ui->pbM1->installEventFilter(this);

    ui->pbP10000000->installEventFilter(this);
    ui->pbP1000000->installEventFilter(this);
    ui->pbP100000->installEventFilter(this);
    ui->pbP10000->installEventFilter(this);
    ui->pbP1000->installEventFilter(this);
    ui->pbP100->installEventFilter(this);
    ui->pbP10->installEventFilter(this);
    ui->pbP1->installEventFilter(this);

    ui->rbRAMP ->installEventFilter(this);
    ui->rbSAW  ->installEventFilter(this);
    ui->rbSIN  ->installEventFilter(this);
    ui->rbTRIA ->installEventFilter(this);

    serialPortState = EClosed;
    updateCtrls();
    fillPortsInfo();

    connect(ui->btnOpenClose, SIGNAL(clicked()), SLOT(openPort()));
    connect(ui->rbRAMP, SIGNAL(clicked()), SLOT(setWaveform()));
    connect(ui->rbSIN, SIGNAL(clicked()), SLOT(setWaveform()));
    connect(ui->rbSAW, SIGNAL(clicked()), SLOT(setWaveform()));
    connect(ui->rbTRIA, SIGNAL(clicked()), SLOT(setWaveform()));

    connect(&serial, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(serialError(QSerialPort::SerialPortError)));
    connect(&serial, SIGNAL(readyRead()), SLOT(portRead()));

    ui->e10000000->setFocus();
    ui->e10000000->selectAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillPortsInfo()
{
    ui->lbSerialPortInfo->clear();
    static const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->lbSerialPortInfo->addItem(list.first(), list);
    }
}

void MainWindow::serialError(QSerialPort::SerialPortError err)
{
    if (err!=0) {
        qDebug() << "Serial error" << err;
        serialPortState = EClosed;
        updateCtrls();
        QMessageBox messageBox;
        messageBox.critical(0,"Error","COM port error!");
        messageBox.setFixedSize(500,200);
    }
}

void MainWindow::portRead()
{
    QByteArray readedData = serial.readAll();

    qDebug() << nowStr() << "portRead" << readedData.toHex();

}

void MainWindow::calculate()
{
    qDebug() << freq << QString("%1").arg(freq, 8, 10, QChar('0'));
    QString freq_str = QString("%1").arg(freq, 8, 10, QChar('0'));
    ui->e10000000->setText(freq_str.at(0));
    ui->e1000000->setText(freq_str.at(1));
    ui->e100000->setText(freq_str.at(2));
    ui->e10000->setText(freq_str.at(3));
    ui->e1000->setText(freq_str.at(4));
    ui->e100->setText(freq_str.at(5));
    ui->e10->setText(freq_str.at(6));
    ui->e1->setText(freq_str.at(7));

    quint64 ddsFreq = ui->edDDSFreq->text().toInt();
    qreal   adder = (qreal)freq / (qreal)ddsFreq  / 1.0 * 4294967296;
    adder32 = qRound(adder);
    waveform = 0;
    if (ui->rbSAW->isChecked()) waveform = 1;
    if (ui->rbRAMP ->isChecked()) waveform = 2;
    if (ui->rbTRIA->isChecked()) waveform = 3;

    ui->edAdder->setText(QString::number(adder, 'g', 16));
    ui->edAdderInt->setText(QString::number(adder32));
    ui->edRealFreq->setText(QString::number((qreal)(adder32*ddsFreq*1)/4294967296, 'g', 16));
    qreal err = (((((qreal)(adder32*ddsFreq*1)/4294967296) / (qreal)freq * 100) - 100));
    ui->edError->setText(QString::number( (qreal)((qint64)(err*1000))/1000));

}

void MainWindow::setFreq()
{
    QByteArray source;
    QDataStream stream(&source, QIODevice::ReadWrite);

    calculate();

    stream << (qint8)(01); // set freq msg
    stream << waveform;
    stream << adder32;
    qDebug() <<  nowStr() << "setFreq" << adder32 << source.toHex();

    serial.write(source);
}

void MainWindow::openPort()
{
    if (serialPortState==EClosed) {
        serialPortState = EWork;
        serial.setPortName(ui->lbSerialPortInfo->currentText());
        serial.setBaudRate(QSerialPort::Baud115200);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        serial.open(QIODevice::ReadWrite);
        qDebug() << nowStr() << "openPort";
    } else if (serialPortState==EWork) {
        serialPortState = EClosed;
        serial.close();
        qDebug() << nowStr() << "closePort";
    }
    updateCtrls();
}

void MainWindow::updateCtrls()
{
    if (serialPortState==EClosed) {
        ui->btnOpenClose->setText("Open");
    } else if (serialPortState==EWork) {
        ui->btnOpenClose->setText("Close");
    }
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    //qDebug() << target << event;

    if((target==ui->e10000000)||
       (target==ui->e1000000)||
       (target==ui->e100000)||
       (target==ui->e10000)||
       (target==ui->e1000)||
       (target==ui->e100)||
       (target==ui->e10)||
       (target==ui->e1))
    {
        if(event->type()==QEvent::MouseButtonPress) {
            dynamic_cast<QLineEdit*>(target)->selectAll();
            return true;
        }

        if(event->type()==QEvent::KeyPress) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent->key()==Qt::Key_Tab) focusNextChild();
            if (keyEvent->key()==Qt::Key_Backtab) focusPreviousChild();
            if ((keyEvent->key()>='0')&&(keyEvent->key()<='9')) {
                focusNextChild();
                dynamic_cast<QLineEdit*>(target)->setText((QString)keyEvent->key());
                QString freq_str = ui->e10000000->text()+
                        ui->e1000000->text() +
                        ui->e100000->text() +
                        ui->e10000->text() +
                        ui->e1000->text() +
                        ui->e100->text() +
                        ui->e10->text() +
                        ui->e1->text();
                freq = freq_str.toInt();
                calculate();
                setFreq();
            }
            return true;
        }

    }

    if ((target->objectName().left(3)=="pbM")||(target->objectName().left(3)=="pbP")) {
        if(event->type()==QEvent::MouseButtonPress||event->type()==QEvent::MouseButtonDblClick) {
            qDebug() << target->objectName().mid(3).toInt() << target->objectName().left(3);
            if(target->objectName().left(3)=="pbM") {
                freq = freq - target->objectName().mid(3).toInt();
            } else if(target->objectName().left(3)=="pbP") {
                freq = freq + target->objectName().mid(3).toInt();
            }
            if(freq>99999999) freq=99999999;
            if(freq<0) freq=0;
            calculate();
            setFreq();
            return true;
        }
    }

    return QMainWindow::eventFilter(target, event);
}

void MainWindow::setWaveform()
{
    calculate();
    setFreq();
}
