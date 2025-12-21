#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>

// ------------------------------------------------------------------------------------------------
///
/// \brief Server::Server
/// \param parent
///
Server::Server(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
}

// ------------------------------------------------------------------------------------------------
///
/// \brief Server::listen
/// \param ip
/// \param port
///
void Server::listen(QHostAddress ip, const quint16 port)
{
    server->listen(ip, port);
    clientConnectState = false;
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::close
///
void Server::close(void)
{
    server->close();
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::on_newConnection
///
void Server::on_newConnection()
{
    // Alte Verbindung sauber beenden
    if(socket != Q_NULLPTR)
    {
        disconnect(socket, nullptr, nullptr, nullptr);
        socket->deleteLater();
        socket = Q_NULLPTR;
    }

    socket = server->nextPendingConnection();
    if( socket != Q_NULLPTR )
    {
        if(socket->state() == QTcpSocket::ConnectedState)
        {
            connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
            connect(socket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));

            //qDebug() << "New connection established.\n";
            clientConnectState = true;
            emit clientConnected(true);
        }
    }
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::hasConnections
/// \return
///
bool Server::hasConnections(void)
{
    return clientConnectState;
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::send
/// \param str
/// \return
///
bool Server::send(QString str)
{
    bool erg = false;

    if( hasConnections())
    {
        if( socket != Q_NULLPTR )
        {
            socket->write(str.toLatin1());
            socket->waitForBytesWritten();
        }
        erg = true;
    }

    return erg;
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::on_readyRead
///
void Server::on_readyRead()
{
    if( socket != Q_NULLPTR )
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
}


// ------------------------------------------------------------------------------------------------
///
/// \brief Server::on_disconnected
///
void Server::on_disconnected()
{
    //qDebug() << "Connection disconnected.\n";
    if( socket != Q_NULLPTR )
    {
        disconnect(socket, SIGNAL(disconnected()));
        disconnect(socket, SIGNAL(readyRead()));
        socket->deleteLater();
    }
    clientConnectState = false;
    emit clientConnected(false);
}
