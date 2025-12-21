#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"


#include <QDebug>
#include <QKeyEvent>
#include <QSettings>
#include <QDateTime>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>


const QString VERSION = "V 1.3.4";


#define DEBUG_BYTES 0

// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::MainWindow
/// \param parent
///
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    Ui(new Ui::MainWindow)
{
    Ui->setupUi(this);


    SettingsForm =new QDialog(this);
    SettingsForm->setModal(true);
    UiSettings.setupUi(SettingsForm);

    setWindowTitle("Systa-Bus  ("+VERSION+")");

    setupCustomPlot(Ui->customPlot);

    Filenumber = 1;
    QString te = QString("_%1.CSV").arg(Filenumber);
    Ui->labelFilename->setText(te);


    IniFilename = QCoreApplication::applicationDirPath();
    IniFilename.append("/");
    IniFilename.append(QCoreApplication::applicationName());
    IniFilename.append(".ini");

    UiSettings.cb_Comport->installEventFilter(this);

    PortTimeoutTimer = new QTimer(this);
    PortTimeoutTimer->setInterval(3000);
    connect (PortTimeoutTimer, SIGNAL(timeout()), this, SLOT(portTimeoutHandler()));

    connect (UiSettings.cb_Comport, SIGNAL(activated(QString)), this, SLOT(openPort(QString)));
    connect (Ui->pb_Start, SIGNAL(clicked()), this, SLOT(start()));
    connect (Ui->pb_Stop, SIGNAL(clicked()), this, SLOT(stop()));

    connect (Ui->cbDebug, SIGNAL(clicked(bool)), this, SLOT(debugOutputChecked(bool)));

    connect (Ui->pb_XFit, SIGNAL(clicked()), this, SLOT(xFit()));
    connect (Ui->pb_YFit, SIGNAL(clicked()), this, SLOT(yFit()));

    connect (&ComPort, SIGNAL(readyRead()), this, SLOT(readPort()));
    connect (&ComPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(portError(QSerialPort::SerialPortError)));

    connect (Ui->actionZeige_Plotfenster, SIGNAL(triggered(bool)), Ui->widget_Plot, SLOT(setVisible(bool)));
    connect (Ui->actionZeige_Log_Einstellungen, SIGNAL(triggered(bool)), Ui->groupBox_Log, SLOT(setVisible(bool)));

    connect( Ui->menuEinstellungen, SIGNAL(aboutToShow()), SettingsForm, SLOT(show()));

    AboutDialog *about = new AboutDialog(this, VERSION);
    connect(Ui->actionAbout, SIGNAL(triggered(bool)), about, SLOT(show()));
    connect(Ui->action_ber_Qt, &QAction::triggered, this, [this]() { QMessageBox::aboutQt(this, tr("Über Qt")); });


    connect( UiSettings.cb_ServerAktiv, SIGNAL(clicked(bool)), this, SLOT(serverStart(bool)));
    connect (&Server, SIGNAL(clientConnected(bool)), Ui->cb_ClientConnected, SLOT(setChecked(bool)));


    Settings = new QSettings(IniFilename, QSettings::IniFormat);
    enumeratePorts();
    QString name = Settings->value("System/COMPort", "COM1").toString();
    int index = UiSettings.cb_Comport->findText(name);
    if ( index != -1 )// -1 for not found
    {
        UiSettings.cb_Comport->setCurrentIndex(index);
        openPort(name);
    }


    move(QPoint(Settings->value("Window/PosX",0).toInt(), Settings->value("Window/PosY",0).toInt()));
    resize(QSize(Settings->value("Window/Width",700).toInt(), Settings->value("Window/Height",400).toInt() ));

    Ui->actionZeige_Plotfenster->setChecked(Settings->value("View/Plot",true).toBool());
    Ui->actionZeige_Log_Einstellungen->setChecked(Settings->value("View/Log",true).toBool());
    UiSettings.cb_ServerAktiv->setChecked(Settings->value("Server/Activ",false).toBool());
    UiSettings.te_ServerIp->setText(Settings->value("Server/IP", "127.0.0.1").toString());
    UiSettings.te_ServerPort->setText(Settings->value("Server/Port", "5001").toString());
    UiSettings.te_ServerInterval->setText(Settings->value("Server/Interval", "10").toString());

    serverStart(UiSettings.cb_ServerAktiv->isChecked());
    Ui->widget_Plot->setVisible(Ui->actionZeige_Plotfenster->isChecked());
    Ui->groupBox_Log->setVisible(Ui->actionZeige_Log_Einstellungen->isChecked());

    Timer.start();

    ComCheckTimer.setInterval(5000);

    Ui->pb_Stop->setEnabled(false);

    StartTime = QDateTime::currentDateTime().toTime_t();
    StopTime = StartTime + 10;
    xFit();

    MaxTemp = -100;
    MinTemp = 200;


    Ui->te_Data->setToolTip("TSA\t: Kollektortemperatur\n"\
                            "TW\t:  Speicher\n"\
                            "TSV\t: Solarvorlauf\n"\
                            "TAM\t: Außentemperatur\n"\
                            "TSE\t: Solarrücklauf\n"\
                            );
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::~MainWindow
///
MainWindow::~MainWindow()
{
    ComCheckTimer.stop();
    PortTimeoutTimer->stop();

    if(ComPort.isOpen())
    {
        ComPort.close();
    }

    if(CsvFile.isOpen())
    {
        CsvFile.close();
    }


    Settings->setValue("System/COMPort", UiSettings.cb_Comport->currentText());

    Settings->setValue("View/Plot",Ui->actionZeige_Plotfenster->isChecked());
    Settings->setValue("View/Log",Ui->actionZeige_Log_Einstellungen->isChecked());

    Settings->setValue("Server/Activ", UiSettings.cb_ServerAktiv->isChecked());
    Settings->setValue("Server/IP", UiSettings.te_ServerIp->text());
    Settings->setValue("Server/Port", UiSettings.te_ServerPort->text());
    Settings->setValue("Server/Interval", UiSettings.te_ServerInterval->text());

    Settings->setValue("Window/PosX", pos().x());
    Settings->setValue("Window/PosY", pos().y());
    Settings->setValue("Window/Width", size().width());
    Settings->setValue("Window/Height", size().height());

    Settings->sync();

    if( Settings != Q_NULLPTR )
    {
        Settings->deleteLater();
    }

    delete Ui;
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::serverStart
/// \param startStop
///
void MainWindow::serverStart(bool startStop)
{
    if(startStop)
    {
        Server.listen(QHostAddress(UiSettings.te_ServerIp->text()), UiSettings.te_ServerPort->text().toInt());
    }
    else
    {
        Server.close();
    }
}

// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::setupCustomPlot
/// \param customPlot
///
void MainWindow::setupCustomPlot(QCustomPlot *customPlot)
{
    QPen pen;


    customPlot->addGraph();
    customPlot->graph()->setName("TSA");
    pen.setColor(QColor(Qt::red));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->clearData();

    customPlot->addGraph();
    customPlot->graph()->setName("TW");
    pen.setColor(QColor(Qt::blue));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->clearData();

    customPlot->addGraph();
    customPlot->graph()->setName("TSE");
    pen.setColor(QColor(Qt::yellow));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->clearData();

    customPlot->addGraph();
    customPlot->graph()->setName("TMA");
    pen.setColor(QColor(Qt::black));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->clearData();

    customPlot->addGraph();
    customPlot->graph()->setName("TSE");
    pen.setColor(QColor(Qt::green));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->clearData();


    // configure bottom axis to show date and time instead of number:
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("dd.MM.yyyy\nhh:mm:ss");
    // set a more compact font size for bottom and left axis tick labels:
    customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

    customPlot->xAxis->setAutoTickStep(true);
    customPlot->xAxis->setSubTickCount(10);

    customPlot->yAxis->setAutoTickStep(true);
    customPlot->yAxis->setSubTickCount(10);

    // set axis labels:
    customPlot->xAxis->setLabel("Zeit");
    customPlot->yAxis->setLabel("°C");

    // set axis ranges to show all data:
    //  customPlot->xAxis->setRange(startTime, stopTime);
    customPlot->yAxis->setRange(0, 15);

    // show legend:
    customPlot->legend->setVisible(true);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::eventFilter
/// \param object
/// \param event
/// \return
///
bool MainWindow::eventFilter(QObject * object, QEvent * event)
{
    bool result = false;

    if (object == UiSettings.cb_Comport)
    {
        if (event && event->type() == QEvent::MouseButtonPress)
        {
            enumeratePorts();
        }
    }

    return result;
}

// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::enumeratePorts
///
void MainWindow::enumeratePorts()
{
    UiSettings.cb_Comport->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        UiSettings.cb_Comport->addItem(info.portName());
        /*
        QString s = QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";

        qDebug() << s;
*/
    }
}

// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::openPort
/// \param name
///
void MainWindow::openPort(QString name)
{
    Ui->cb_ComportOk->setChecked(false);

    //    if (ComPort.isOpen())
    {
        ComPort.close();
        ComPort.clearError();
    }

    ComPort.setPortName(name);

    Ui->te_Data->clear();

    if (ComPort.open(QIODevice::ReadWrite))
    {
        ComPort.setBaudRate(9600);
        ComPort.setDataBits(QSerialPort::Data8);
        ComPort.setStopBits(QSerialPort::OneStop);
        ComPort.setParity(QSerialPort::NoParity);
        ComPort.setBreakEnabled(false);
        ComPort.setFlowControl(QSerialPort::NoFlowControl);
        ComPort.setReadBufferSize(10000);
        ComPort.setRequestToSend(true);
        ComPort.setDataTerminalReady(true);

        ComPort.clear(QSerialPort::AllDirections);
        Ui->te_Data->append("Port opened");
        Ui->cb_ComportOk->setChecked(true);
    }
    else
    {
        Ui->te_Data->append("Port not available");
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::xFit
///
void MainWindow::xFit(void)
{
    Ui->customPlot->xAxis->setRange(StartTime, StopTime);
    Ui->customPlot->replot();
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::yFit
///
void MainWindow::yFit(void)
{
    Ui->customPlot->yAxis->setRange(MinTemp, MaxTemp);
    Ui->customPlot->replot();
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::start
///
void MainWindow::start(void)
{
    Ui->pb_Start->setEnabled(false);
    Ui->pb_Stop->setEnabled(true);


    if( CsvFile.isOpen())
    {
        CsvFile.close();
    }


    QString path = QCoreApplication::applicationDirPath();

    QString te = QString("_%1.CSV").arg(Filenumber);
    Ui->labelFilename->setText(te);
    path.append(QString("/%1%2.CSV").arg(Ui->FilenameLineEdit->text()).arg(te));
    CsvFile.setFileName(path);

    if (!CsvFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        Ui->te_Data->setText("Error opening file");
    }
    else
    {
        CsvFile.write(QString("Zeit;TSA (°C);TW (°C);TSV (°C);TAM (°C);TSE (°C);Fluss (l/min);PWM (%);Tagesleistung (kWh);Gesamtleistung (kWh);Status\n").toLatin1());
        Filenumber++;
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::stop
///
void MainWindow::stop(void)
{
    Ui->pb_Start->setEnabled(true);
    Ui->pb_Stop->setEnabled(false);

    if (CsvFile.isOpen())
    {
        CsvFile.close();
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::readPort
///
void MainWindow::readPort(void)
{
#define WAIT_FOR_SYNC   0
#define SYNC1           0xFC3E
#define SYNC2           0xFC1F
#define SYNC3           0x2302
#define SYNC4           0xFD05

    static char buffer;
    static int ablauf = WAIT_FOR_SYNC;
    static quint16 sync = 0;
    static QByteArray data;
    static int count = 0;
    static double logTime = 0;
    static double logTimeServer = 0;

    static quint64 countPakets[4] = {0,0,0,0};
    static quint64 countErrors[4] = {0,0,0,0};

    while (!ComPort.atEnd())
    {
        ComPort.read(&buffer, 1);

        sync <<= 8;
        sync |= buffer;

        if( sync == SYNC1)
        {
            ablauf = sync;
            data.clear();
            count = 0;
        }
        else
            if( sync == SYNC2)
            {
                ablauf = sync;
                data.clear();
                count = 0;
            }
            else
                if( sync == SYNC3)
                {
                    ablauf = sync;
                    data.clear();
                    count = 0;
                }
                else
                    if( sync == SYNC4)
                    {
                        ablauf = sync;
                        data.clear();
                        count = 0;
                    }
                    else
                    {
                        if( ablauf == SYNC1)
                        {
                            data.append(buffer);
                            count++;
                            if( count > (SYNC1&0xFF))
                            {
                                int checksum = 0;
                                checksum += (SYNC1>>8);
                                checksum += (SYNC1&0xFF);
                                for(int i=0; i <= (SYNC1&0xFF); i++)
                                {
                                    checksum += data.at(i);
                                }
                                if( (checksum & 0xFF) != 0)
                                {
                                    //qDebug() << "Checksum Error Sync1\n";
                                    countErrors[0]++;
                                }
                                else
                                {
                                    countPakets[0]++;
                                    double tsa = double((qint16)((data.at(2) & 0xFF) | ((data.at(3) & 0xFF) << 8))) / 10.0;
                                    double tw = double((qint16)((data.at(4) & 0xFF) | ((data.at(5) & 0xFF) << 8))) / 10.0;
                                    double tsv = double((qint16)((data.at(6) & 0xFF) | ((data.at(7) & 0xFF) << 8))) / 10.0;
                                    double tam = double((qint16)((data.at(8) & 0xFF) | ((data.at(9) & 0xFF) << 8))) / 10.0;
                                    double tse = double((qint16)((data.at(12) & 0xFF) | ((data.at(13) & 0xFF) << 8))) / 10.0;
                                    double flow = double((qint16)((data.at(14) & 0xFF) | ((data.at(15) & 0xFF) << 8))) / 10.0;
                                    int pwm = (data.at(16) & 0xFF);
                                    int status = (data.at(20) & 0xFF);
                                    int energyDay = (qint16)((data.at(31) & 0xFF) | (data.at(32) & 0xFF) << 8);
                                    int energyTotal = (qint32)((data.at(35) & 0xFF) | (data.at(36) & 0xFF) << 8 | (data.at(37) & 0xFF) << 16 | (data.at(38) & 0xFF) << 24);
                                    QString dayTime = QString("%1:%2  %3.%4.20%5").arg(data.at(24) & 0xFF).arg(data.at(25) & 0xFF, 2, 10, QLatin1Char( '0' )).arg(data.at(26) & 0xFF).arg(data.at(27) & 0xFF).arg(data.at(28) & 0xFF);

                                    double oldMax = MaxTemp;
                                    double oldMin = MinTemp;
                                    if(MaxTemp < tsa)
                                    {
                                        MaxTemp = tsa+5;
                                    }
                                    if(MinTemp > tsa)
                                    {
                                        MinTemp = tsa-5;
                                    }
                                    if( (oldMax != MaxTemp) || (oldMin != MinTemp))
                                    {
                                        yFit();
                                    }

                                    double zeit = double(QDateTime::currentDateTime().currentMSecsSinceEpoch()) / 1000.0;

                                    if( zeit >= logTime)
                                    {
                                        logTime = zeit + 1;

                                        Ui->te_Data->setText(QString("TSA: %1 °C").arg(tsa));
                                        Ui->te_Data->append(QString("TW:  %1 °C").arg(tw));
                                        Ui->te_Data->append(QString("TSV: %1 °C").arg(tsv));
                                        Ui->te_Data->append(QString("TAM: %1 °C").arg(tam));
                                        Ui->te_Data->append(QString("TSE: %1 °C").arg(tse));
                                        Ui->te_Data->append(QString("Fluss: %1 l/min").arg(flow));
                                        Ui->te_Data->append(QString("PWM: %1").arg(pwm));
                                        Ui->te_Data->append(QString("Tag: %1 kWh").arg(energyDay));
                                        Ui->te_Data->append(QString("Gesamt: %1 kWh").arg(energyTotal));
                                        Ui->te_Data->append(QString("Zeit: %1").arg(dayTime));
                                        Ui->te_Data->append(QString("Status: %1").arg(getStatus(status)));
                                        Ui->te_Data->append(QString("ErrorCode: %1").arg(status));

                                        if( Ui->actionZeige_Plotfenster->isChecked() )
                                        {
                                            Ui->customPlot->graph(0)->addData( zeit, tsa);
                                            Ui->customPlot->graph(1)->addData( zeit, tw);
                                            Ui->customPlot->graph(2)->addData( zeit, tsv);
                                            Ui->customPlot->graph(3)->addData( zeit, tam);
                                            Ui->customPlot->graph(4)->addData( zeit, tse);
                                        }

                                        if( CsvFile.isOpen() )
                                        {
                                            CsvFile.write(QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;\n").arg(QDateTime::currentDateTime().toString()).arg(tsa).arg(tw).arg(tsv).arg(tam).arg(tse).arg(flow).arg(pwm).arg(energyDay).arg(energyTotal).arg(getStatus(status)).toLatin1());
                                        }

                                        Ui->te_Data->append(QString("\nPakete: %1 Ok / %2 Fehler").arg(countPakets[0]).arg(countErrors[0]));
                                        // for Debug only
#if 0
                                        QString str;
                                        for(int i=0; i < 4; i++)
                                        {
                                            str += QString("%1:%2    ").arg(countPakets[i]).arg(countErrors[i]);
                                        }
                                        qDebug() << str;
#endif
                                    }

                                    if( zeit >= logTimeServer)
                                    {
                                        logTimeServer = zeit + UiSettings.te_ServerInterval->text().toInt();

                                        if( Server.hasConnections() )
                                        {
                                            Server.send(QString("TSA:%1;TW:%2;TSV:%3;TAM:%4;TSE:%5;FLOW:%6;PWM:%7;EnergyDay:%8;EnergyTotal:%9;Status:%10;ErrorCode:%11;").arg(tsa).arg(tw).arg(tsv).arg(tam).arg(tse).arg(flow).arg(pwm).arg(energyDay).arg(energyTotal).arg(getStatus(status)).arg(status));
                                        }
                                    }

                                    if( zeit > StopTime)
                                    {
                                        StopTime += 10;
                                        if( Ui->actionZeige_Plotfenster->isChecked() )
                                        {
                                            xFit();
                                        }
                                    }
                                    else
                                    {
                                        if( Ui->actionZeige_Plotfenster->isChecked() )
                                        {
                                            Ui->customPlot->replot();
                                        }
                                    }

                                    debugOutput(1, data, count);
                                }
                                ablauf = WAIT_FOR_SYNC;
                            }
                        }
                        else
                            if( ablauf == SYNC2)
                            {
                                data.append(buffer);
                                count++;
                                if( count > (SYNC2&0xFF))
                                {
                                    int checksum = 0;
                                    checksum += (SYNC2>>8);
                                    checksum += (SYNC2&0xFF);
                                    for(int i=0; i <= (SYNC2&0xFF); i++)
                                    {
                                        checksum += data.at(i);
                                    }
                                    if( (checksum & 0xFF) != 0)
                                    {
                                        //qDebug() << "Checksum Error Sync2\n";
                                        countErrors[1]++;
                                    }
                                    else
                                    {
                                        countPakets[1]++;

                                        debugOutput(2, data, count);
                                    }
                                    ablauf = WAIT_FOR_SYNC;
                                }
                            }
                            else
                                if( ablauf == SYNC3)
                                {
                                    data.append(buffer);
                                    count++;
                                    if( count > (SYNC3&0xFF))
                                    {
                                        int checksum = 0;
                                        checksum += (SYNC3>>8);
                                        checksum += (SYNC3&0xFF);
                                        for(int i=0; i <= (SYNC3&0xFF); i++)
                                        {
                                            checksum += data.at(i);
                                        }
                                        if( (checksum & 0xFF) != 0)
                                        {
                                            //qDebug() << "Checksum Error Sync3\n";
                                            countErrors[2]++;
                                        }
                                        else
                                        {
                                            countPakets[2]++;

                                            debugOutput(4, data, count);
                                        }
                                        ablauf = WAIT_FOR_SYNC;
                                    }
                                }
                                else
                                    if( ablauf == SYNC4)
                                    {
                                        data.append(buffer);
                                        count++;
                                        if( count > (SYNC4&0xFF))
                                        {
                                            int checksum = 0;
                                            checksum += (SYNC4>>8);
                                            checksum += (SYNC4&0xFF);
                                            for(int i=0; i <= (SYNC4&0xFF); i++)
                                            {
                                                checksum += data.at(i);
                                            }
                                            if( (checksum & 0xFF) != 0)
                                            {
                                                //qDebug() << "Checksum Error Sync4\n";
                                                countErrors[3]++;
                                            }
                                            else
                                            {
                                                countPakets[3]++;

                                                debugOutput(4, data, count);
                                            }
                                            ablauf = WAIT_FOR_SYNC;
                                        }
                                    }
                    }
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::portError
/// \param error
///
void MainWindow::portError(QSerialPort::SerialPortError error)
{
    if( error != QSerialPort::SerialPortError::NoError)
    {
        PortTimeoutTimer->start(3000);
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::portTimeoutHandler
///
void MainWindow::portTimeoutHandler(void)
{
    PortTimeoutTimer->stop();

    enumeratePorts();
    QString name = Settings->value("System/COMPort", "COM1").toString();
    int index = UiSettings.cb_Comport->findText(name);
    if ( index != -1 )// -1 for not found
    {
        UiSettings.cb_Comport->setCurrentIndex(index);
        openPort(name);
    }

}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::debugOutputChecked
/// \param checked
///
void MainWindow::debugOutputChecked(bool checked)
{
    if( checked )
    {
        QString path = QCoreApplication::applicationDirPath();

        path.append("/");
        path.append("Debug.txt");
        DebugFile.setFileName(path);

        if (!DebugFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            Ui->te_Data->setText("Error opening file");
        }
        else
        {
        }
    }
    else
    {
        DebugFile.close();
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::debugOutput
/// \param msg
///
void MainWindow::debugOutput(int index, QByteArray data, int count)
{
    if (Ui->cbDebug->isChecked())
    {
        if( DebugFile.isOpen())
        {
            QString str;

            str = QString("Sync%1 (%2 msec):").arg(index).arg(Timer.restart());

            for(int i=0; i < count; i++)
            {
                if((i%10) == 0)
                {
                    DebugFile.write(str.toLatin1());
                    DebugFile.write("\r\n");
                    str = "";
                }
                str.append(QString("%1 ").arg((int)(data.at(i)& 0xFF), 2, 16, QLatin1Char( '0' )));
            }
            DebugFile.write(str.toLatin1());
            DebugFile.write("\r\n");
        }
    }

}


// ------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::debugOutput
/// \param msg
///
QString MainWindow::getStatus(int code)
{
    QString msg;

    switch(code)
    {
        case 0:
            msg = "Ok";
            break;
        case 1:
            msg = "Durchfluss im Solarkreis blockiert oder Pumpe defekt";
            break;
        case 2:
            msg = "Luft in der Anlage";
            break;
        case 3:
            msg = "Kein Volumenstrom im Frostschutz";
            break;
        case 4:
            msg = "Vorlauf- und Rücklauf des Kollektors vertauscht";
            break;
        case 5:
            msg = "Rückschlagklappe undicht (Fehlzirkulation gegen Pumprichtung)";
            break;
        case 6:
            msg = "Falsche Uhrzeit";
            break;
        case 7:
            msg = "Druckabfall in der Anlage";
            break;
        case 8:
            msg = "Volumenstrom zu hoch";
            break;
        case 9:
            msg = "Hydraulischer Anschluss fehlerhaft (Fehlzirkulation in Pumprichtung)";
            break;
        case 10:
            msg = "Anlage nicht frostsicher";
            break;
        case 11:
            msg = "Keine permanente Spannungsversorgung";
            break;
        case 12:
            msg = "Speicherfühler falsch gesetzt, ULV defekt oder Wärmetauscher verkalkt";
            break;
        case 13:
            msg = "Volumenstrom zu niedrig";
            break;
        case 14:
            msg = "Speicher unterkühlt";
            break;
        case 22:
            msg = "Ausfall Kollektorfühler (TSA)";
            break;
        case 23:
            msg = "Ausfall Fühler TSE";
            break;
        case 24:
            msg = "Ausfall Fühler TWU";
            break;
        case 26:
            msg = "Ausfall Fühler TW2";
            break;
        case 27:
            msg = "Ausfall Außensensor (TWA)";
            break;
        case 34:
            msg = "Speicher überhitzt";
            break;
        case 35:
            msg = "Speicher 2 überhitzt";
            break;
        case 50:
            msg = "Frostgefahr";
            break;

        default:
            msg = QString("Fehler unbekannt (%1)").arg(code);
            break;
    }

    return (msg);
}
