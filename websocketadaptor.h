#ifndef WEBSOCKETADAPTOR_H
#define WEBSOCKETADAPTOR_H

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

 class WebSocketListener
{
public:
    virtual void onConnected() = 0;
    virtual void onMessage(std::string msg) = 0;
};

class WebSocketAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketAdaptor();
    void Connect();
    WebSocketListener *listener;

Q_SIGNALS:
    void closed();
    void MessageToSend(QString msg);

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void SendMessage(QString msg);

private:
    QWebSocket m_webSocket;


};

#endif // WEBSOCKETADAPTOR_H
