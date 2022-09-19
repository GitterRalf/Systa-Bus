#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class QTcpServer;
class QTcpSocket;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void listen(QHostAddress ip, const quint16 port);

    bool hasConnections(void);

    bool send(QString string);


signals:
    void clientConnected(bool connect);


public slots:
    void on_newConnection();
    void on_readyRead();
    void on_disconnected();

private:
    QTcpServer* server;
    QTcpSocket* socket;

    bool clientConnectState;

};

#endif // SERVER_H
