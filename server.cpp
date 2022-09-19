#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>

Server::Server(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()),
            this, SLOT(on_newConnection()));
}

// ------------------------------------------------------------------------------------------------
void Server::listen(QHostAddress ip, const quint16 port)
{
    server->listen(ip, port);
    clientConnectState = false;
}


// ------------------------------------------------------------------------------------------------
void Server::on_newConnection()
{
    socket = server->nextPendingConnection();
    if(socket->state() == QTcpSocket::ConnectedState)
    {
        //qDebug() << "New connection established.\n";
        clientConnectState = true;
        emit clientConnected(true);
    }
    connect(socket, SIGNAL(disconnected()),
            this, SLOT(on_disconnected()));
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(on_readyRead()));
}


// ------------------------------------------------------------------------------------------------
bool Server::hasConnections(void)
{
    return clientConnectState;
}


// ------------------------------------------------------------------------------------------------
bool Server::send(QString str)
{
    bool erg = false;

    if( hasConnections())
    {
        if (! socket->waitForBytesWritten())
        {
            socket->write(str.toLatin1());
            erg = true;
        }
    }

    return erg;
}

// ------------------------------------------------------------------------------------------------
void Server::on_readyRead()
{
    while(socket->canReadLine())
    {
        QByteArray ba = socket->readLine();
        if(strcmp(ba.constData(), "!exit\n") == 0)
        {
            socket->disconnectFromHost();
            break;
        }
        //printf(">> %s", ba.constData());
    }
}


// ------------------------------------------------------------------------------------------------
void Server::on_disconnected()
{
    //qDebug() << "Connection disconnected.\n";
    disconnect(socket, SIGNAL(disconnected()));
    disconnect(socket, SIGNAL(readyRead()));
    socket->deleteLater();
    clientConnectState = false;
    emit clientConnected(false);
}
