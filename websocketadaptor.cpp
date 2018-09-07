#include "websocketadaptor.h"
#include "stdio.h"
#include <iostream>
#include <thread>
#include "settings.h"


QT_USE_NAMESPACE

//QObject::connect(&client, &EchoClient::closed, &a, &QCoreApplication::quit);

WebSocketAdaptor::WebSocketAdaptor()
{

}

void WebSocketAdaptor::Connect()
{
     connect(this, &WebSocketAdaptor::MessageToSend, this, &WebSocketAdaptor::SendMessage);
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketAdaptor::onConnected);

    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketAdaptor::onTextMessageReceived);
    //connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketAdaptor::closed);
    const char *adr = Settings::webSockAdr.c_str();
    m_webSocket.open(QUrl(QString(adr)));
}

void WebSocketAdaptor::onConnected()
{
    //connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketAdaptor::onTextMessageReceived);
    listener->onConnected();

}

void WebSocketAdaptor::onTextMessageReceived(QString message)
{
    listener->onMessage(message.toStdString().c_str());
}


void WebSocketAdaptor::SendMessage(QString msg)
{
    m_webSocket.sendTextMessage(msg);
}







