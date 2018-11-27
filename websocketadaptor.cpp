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
    QString url;
    if(Settings::isSequre) {
        url = QString("wss://")+QString(adr)+QString(":5443/WebRTCAppEE/websocket");
    }
    else {
        url = QString("ws://")+QString(adr)+QString(":5080/WebRTCAppEE/websocket");
    }
    std::cout<<"WebSocketOpen:"<<url.toStdString()<<std::endl;
    m_webSocket.open(url);
}

void WebSocketAdaptor::onConnected()
{
    printf("Web Socket Connected\n");
    //connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketAdaptor::onTextMessageReceived);
    listener->onConnected();

}

void WebSocketAdaptor::onTextMessageReceived(QString message)
{
    std::cout<<"Signalling Message:"<<message.toStdString().c_str()<<std::endl;
    listener->onMessage(message.toStdString().c_str());
}


void WebSocketAdaptor::SendMessage(QString msg)
{
    m_webSocket.sendTextMessage(msg);
}







