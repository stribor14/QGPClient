#include "qgpclient.h"

const QMap<QGPClient::dataCommands, QByteArray> QGPClient::cmdMap = {
       { QGPClient::SEND_DATA, "ENABLE_SEND_DATA" },
       { QGPClient::COUNTER, "ENABLE_SEND_COUNTER" },
       { QGPClient::TIME, "ENABLE_SEND_TIME" },
       { QGPClient::TIME_TICK, "ENABLE_SEND_TIME_TICK" },
       { QGPClient::POG_FIX, "ENABLE_SEND_POG_FIX" },
       { QGPClient::POG_LEFT, "ENABLE_SEND_POG_LEFT" },
       { QGPClient::POG_RIGHT, "ENABLE_SEND_POG_RIGHT" },
       { QGPClient::POG_BEST, "ENABLE_SEND_POG_BEST" },
       { QGPClient::PUPIL_LEFT, "ENABLE_SEND_PUPIL_LEFT" },
       { QGPClient::PUPIL_RIGHT, "ENABLE_SEND_PUPIL_RIGHT" },
       { QGPClient::EYE_LEFT, "ENABLE_SEND_EYE_LEFT" },
       { QGPClient::EYE_RIGHT, "ENABLE_SEND_EYE_RIGHT" },
       { QGPClient::CURSOR, "ENABLE_SEND_CURSOR" },
       { QGPClient::USER_DATA, "ENABLE_SEND_USER_DATA" }
   };

void QGPClient::receiveData()
{
    QByteArray data = tcpSocket->readAll();
    int delimiter_index = data.indexOf("\r\n", 0);
    while (data.size())
    {
        bufferMutex.lock();
            buffer.append(data.mid(0, delimiter_index));
            if (notificationSetting.testFlag(QGPClient::notifyEveryMsg))
                emit msgReceived(buffer.last());
            if (buffer.size() >= bufferSize){
                if(notificationSetting.testFlag(QGPClient::notifyFullBuffer))
                    emit msgReceived(buffer.last());
                while (buffer.size() > bufferSize)
                        buffer.removeFirst();
            }
        bufferMutex.unlock();

        data.remove(0, delimiter_index+2);
        delimiter_index = data.indexOf("\r\n", 0);
    }
    if(notificationSetting.testFlag(QGPClient::notifyEveryCycle))
        emit msgReceived(buffer.last());
}

QGPClient::QGPClient(QObject * parent) : QObject(parent),
    tcpSocket(new QTcpSocket())
{
    ipPort = 4242;
    ipAddress = "127.0.0.1";

    bufferSize = 60*60*3;
    connect(tcpSocket, &QTcpSocket::readyRead, this, &QGPClient::receiveData);

    notificationSetting.setFlag(QGPClient::notifyEveryCycle, true);

    //signal forwarding
    connect(tcpSocket, &QTcpSocket::hostFound, this, &QGPClient::hostFound);
    connect(tcpSocket, &QTcpSocket::connected, this, &QGPClient::connected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &QGPClient::disconnected);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &QGPClient::stateChanged);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &QGPClient::error);
}

QGPClient::~QGPClient()
{
    tcpSocket->abort();
    delete tcpSocket;
}

bool QGPClient::clientConnect()
{
    tcpSocket->connectToHost(ipAddress, ipPort);
    if (tcpSocket->waitForConnected(1000)) return true;
    return false;
}

bool QGPClient::clientDisconnect()
{
    tcpSocket->disconnectFromHost();
    if (tcpSocket->state() == QTcpSocket::UnconnectedState
            || tcpSocket->waitForDisconnected(1000))
        return true;
    return false;
}

void QGPClient::getMsgBuffer(QList<QByteArray> &data)
{
    bufferMutex.lock();
        data.clear();
        buffer.swap(data);
    bufferMutex.unlock();
}

QByteArray QGPClient::getLastMsg()
{
    QByteArray temp;

    bufferMutex.lock();
        if (buffer.isEmpty()) return temp;
        else {
            temp = buffer.takeLast();
        }
        buffer.clear();
    bufferMutex.unlock();

    return temp;
}

void QGPClient::sendCommand(QByteArray cmd)
{
    cmd = cmd + "\r\n";
    tcpSocket->write(cmd, cmd.size());
    tcpSocket->flush();
}

void QGPClient::sendCommand(QGPClient::dataCommands cmd, bool state)
{
    sendCommand(R"(<SET ID=")" + cmdMap.value(cmd) + R"(" STATE=")" + (state ? "1" : "0") + R"(" />)");
}

void QGPClient::sendCommand(const QList<QGPClient::dataCommands> &cmd, bool state)
{
    for(auto &&single : cmd) sendCommand(single, state);
}
