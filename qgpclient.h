/************************************************************************************
**                                                                                 **
** QGPClient: simple TCP communication client for Gazepoint GP3 HD gaze tracker    **
** for use within Qt framework.                                                    **
**                                                                                 **
** MIT License                                                                     **
**                                                                                 **
** Copyright (c) 2018 stribor14                                                    **
**                                                                                 **
** Permission is hereby granted, free of charge, to any person obtaining a copy    **
** of this software and associated documentation files (the "Software"), to deal   **
** in the Software without restriction, including without limitation the rights    **
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       **
** copies of the Software, and to permit persons to whom the Software is           **
** furnished to do so, subject to the following conditions:                        **
**                                                                                 **
** The above copyright notice and this permission notice shall be included in all  **
** copies or substantial portions of the Software.                                 **
**                                                                                 **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        **
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     **
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          **
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   **
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   **
** SOFTWARE.                                                                       **
**                                                                                 **
*************************************************************************************
**           Author: Mirko Kokot                                                   **
**          Contact: mirko.kokot@fer.hr                                            **
**             Date: 29.04.18                                                      **
**          Version: 1.0                                                           **
*************************************************************************************/

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

    enum dataCommands{
        SEND_DATA,
        COUNTER,
        TIME,
        TIME_TICK,
        POG_FIX,
        POG_LEFT,
        POG_RIGHT,
        POG_BEST,
        PUPIL_LEFT,
        PUPIL_RIGHT,
        EYE_LEFT,
        EYE_RIGHT,
        CURSOR,
        USER_DATA
    };

private:
    QTcpSocket *tcpSocket;

    unsigned int ipPort;
    QString ipAddress;

    QList<QByteArray> buffer;
    int bufferSize;
    QMutex bufferMutex;

    notificationFlags notificationSetting;

    static const QMap<dataCommands, QByteArray> cmdMap;

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

    void sendCommand(QByteArray cmd);
    void sendCommand(dataCommands cmd, bool state = true);
    void sendCommand(const QList<dataCommands> &cmd, bool state = true);

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
