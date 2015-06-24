#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QEvent>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject *target, QEvent *event);

public slots:
    void calculate();
    void setFreq();
    void setWaveform();
    void openPort();
    void updateCtrls();

private slots:
    void serialError(QSerialPort::SerialPortError err);
    void portRead();


private:
    enum EState {
        EClosed,
        EWork
    } serialPortState;


    void fillPortsInfo();
    Ui::MainWindow *ui;
    qint64 freq;
    qint32 adder32;
    qint8  waveform; //0-sin; 1-saw; 2-ramp; 3-tria

    QSerialPort serial;
};

#endif // MAINWINDOW_H
