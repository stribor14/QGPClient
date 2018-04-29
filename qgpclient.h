#ifndef QGPCLIENT_H
#define QGPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QFlags>

class QGPClient : public QObject
{
    Q_OBJECT
public:
    enum notificationOptions{
        notifyEveryMsg   = 0x1,
        notifyEveryCycle = 0x2,
        notifyFullBuffer = 0x4
    };
    Q_DECLARE_FLAGS(notificationFlags, notificationOptions)

private:
    QTcpSocket *tcpSocket;

    unsigned int ipPort;
    QString ipAddress;

    QList<QByteArray> buffer;
    int bufferSize;
    QMutex bufferMutex;

    notificationFlags notificationSetting;

private slots:
    void receiveData();

public:
    QGPClient(QObject * parent = NULL);
    ~QGPClient();

    void setAddress (QString address) {ipAddress = address;} // set server IP address
    void setPort (unsigned int port) {ipPort = port;} // set server IP port
    void setNotificationFlags(QGPClient::notificationOptions value, bool on = true){ notificationSetting.setFlag(value, on); }

    bool clientConnect();
    bool clientDisconnect();

    void sendCmd(QByteArray cmd);

    void getMsgBuffer(QList<QByteArray> &data);
    int getNotificationFlags(){ return notificationSetting; }
    void clearBuffer(){ buffer.clear(); }
    QByteArray getLastMsg(); // WARNING: clearing the buffer content

    QTcpSocket::SocketState getState(){return tcpSocket->state();}
    QTcpSocket::SocketError getLastError(){return tcpSocket->error();}

signals:
    void msgReceived(const QByteArray &msg);

    // forwarded from QTcpSocket
    void hostFound();
    void connected();
    void disconnected();
    void stateChanged(QTcpSocket::SocketState);
    void error(QTcpSocket::SocketError);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGPClient::notificationFlags)

#endif // QGPCLIENT_H
