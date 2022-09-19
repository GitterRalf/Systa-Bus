#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QFile>
#include <QElapsedTimer>
#include <QTimer>

#include "ui_setting.h"
#include "qcustomplot/qcustomplot.h"
#include "server.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual bool eventFilter(QObject * object, QEvent * event);

private:
    void debugOutput(int index, QByteArray data, int count);
    QString getStatus(int code);


private slots:

    void enumeratePorts();

    void openPort(QString name);

    void readPort(void);

    void start(void);

    void stop(void);

    void setupCustomPlot(QCustomPlot *customPlot);

    void xFit(void);

    void yFit(void);

    void serverStart(bool startStop);

    void portError(QSerialPort::SerialPortError error);

    void portTimeoutHandler(void);

    void debugOutputChecked(bool checked);


private:

    Ui::MainWindow *Ui;

    QDialog *SettingsForm;

    Ui::Setting UiSettings;

    QSettings *Settings;

    QSerialPort Port;

    QFile CsvFile;
    QFile DebugFile;

    QString IniFilename;

    QElapsedTimer Timer;

    QTimer ComCheckTimer;

    int Filenumber;

    double StartTime;
    double StopTime;

    double MinTemp;
    double MaxTemp;

    class Server Server;

    QTimer *PortTimeoutTimer;

};

#endif // MAINWINDOW_H
