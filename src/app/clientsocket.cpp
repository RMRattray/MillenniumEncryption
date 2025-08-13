#include "clientsocket.h"
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

ClientSocketManager::ClientSocketManager(QString server_address, QObject *parent)
    : QObject(parent)
    , sock(new QTcpSocket(this))
{
    // Connect socket signals
    connect(sock, &QTcpSocket::connected, this, [this]() {
        qDebug() << "Connected to server";
    });
    
    connect(sock, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "Disconnected from server";
    });
    
    connect(sock, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        qDebug() << "Socket error:" << error;
    });
    
    // Connect readyRead to handle incoming data
    connect(sock, &QTcpSocket::readyRead, this, [this]() {
        while (sock->bytesAvailable() >= PACKET_BUFFER_SIZE) {
            char buffer[PACKET_BUFFER_SIZE];
            qint64 bytesRead = sock->read(buffer, PACKET_BUFFER_SIZE);
            if (bytesRead == PACKET_BUFFER_SIZE) {
                handlePacket(buffer);
            }
        }
    });
    
    sock->connectToHost(server_address, 1999);
}

ClientSocketManager::~ClientSocketManager()
{
    if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->disconnectFromHost();
    }
}

// Slots for sending packets to server
void ClientSocketManager::sendAccountRequest(QString user_name, QString password)
{
    if (sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    createAccountRequest packet(user_name.toStdString(), password.toStdString());
    unsigned char buffer[PACKET_BUFFER_SIZE];
    int bytesWritten = packet.write_to_packet(buffer);
    
    if (bytesWritten > 0) {
        sock->write(reinterpret_cast<const char*>(buffer), bytesWritten);
    }
}

void ClientSocketManager::sendLoginRequest(QString user_name, QString password)
{
    if (sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    loginRequest packet(user_name.toStdString(), password.toStdString());
    unsigned char buffer[PACKET_BUFFER_SIZE];
    int bytesWritten = packet.write_to_packet(buffer);
    
    if (bytesWritten > 0) {
        sock->write(reinterpret_cast<const char*>(buffer), bytesWritten);
    }
}

void ClientSocketManager::sendFriendRequest(QString friend_name)
{
    if (sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    friendRequestSend packet(friend_name.toStdString());
    unsigned char buffer[PACKET_BUFFER_SIZE];
    int bytesWritten = packet.write_to_packet(buffer);
    
    if (bytesWritten > 0) {
        sock->write(reinterpret_cast<const char*>(buffer), bytesWritten);
    }
}

void ClientSocketManager::sendFriendRequestResponse(QString friend_name, FriendRequestResponse response)
{
    if (sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    // Note: This needs the 'to' and 'from' parameters from the packet.h
    // For now, using empty strings - you may need to modify this based on your needs
    friendRequestAcknowledge packet("", friend_name.toStdString(), response);
    unsigned char buffer[PACKET_BUFFER_SIZE];
    int bytesWritten = packet.write_to_packet(buffer);
    
    if (bytesWritten > 0) {
        sock->write(reinterpret_cast<const char*>(buffer), bytesWritten);
    }
}

void ClientSocketManager::sendMessage(QString friend_name, QString message)
{
    if (sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Not connected to server";
        return;
    }
    
    messageSend packet(message.toStdString(), friend_name.toStdString());
    unsigned char buffer[PACKET_BUFFER_SIZE];
    while (packet.write_to_packet(buffer)) sock->write(reinterpret_cast<const char*>(buffer), PACKET_BUFFER_SIZE);
    sock->write(reinterpret_cast<const char*>(buffer), PACKET_BUFFER_SIZE);
}

// Private method to handle incoming packets and emit signals
void ClientSocketManager::handlePacket(char *packet)
{
    // First byte contains the packet type
    PacketFromServerType packetType = static_cast<PacketFromServerType>(packet[0]);
    unsigned char buffer[PACKET_BUFFER_SIZE];

    switch (packetType) {
        case ACCOUNT_RESULT: {
            createAccountResponse response(reinterpret_cast<unsigned char*>(packet));
            if (response.success) emit mentionLoginSuccess();
            else emit mentionAccountResult(QString::fromStdString(response.reason));
            break;
        }
        
        case LOGIN_RESULT: {
            loginResult response(reinterpret_cast<unsigned char*>(packet));
            if (response.success) emit mentionLoginSuccess();
            else emit mentionLoginResult(QString::fromStdString(response.reason));
            break;
        }
        
        case FRIEND_STATUS_UPDATE: {
            friendStatusUpdate response(reinterpret_cast<unsigned char*>(packet));
            emit mentionFriendStatus(QString::fromStdString(response.username), response.status);
            break;
        }
        
        case FRIEND_REQUEST_FORWARD: {
            friendRequestForward response(reinterpret_cast<unsigned char*>(packet));
            emit mentionFriendRequest(QString::fromStdString(response.from));
            break;
        }
        
        case FRIEND_REQUEST_RESPONSE: {
            friendRequestResponse response(reinterpret_cast<unsigned char*>(packet));
            emit mentionFriendRequestResponse(QString::fromStdString(response.from), response.response);
            break;
        }
        
        case MESSAGE_FORWARD: {
            messageForward response(reinterpret_cast<unsigned char*>(packet));
            if (response.bytes_remaining) {
                sock->read((char *)buffer, PACKET_BUFFER_SIZE);
                while(response.read_from_packet(buffer)) sock->read((char *)buffer, PACKET_BUFFER_SIZE);
            }
            emit mentionMessage(QString::fromStdString(response.sender), QString::fromStdString(response.message));
            break;
        }
        
        case MESSAGE_RESPONSE: {
            messageResponse response(reinterpret_cast<unsigned char*>(packet));
            // Note: This packet doesn't have a corresponding signal in the header
            // You may want to add a signal for message delivery confirmation
            qDebug() << "Message response received, sent:" << response.sent;
            break;
        }
        
        default:
            qDebug() << "Unknown packet type received:" << packetType;
            break;
    }
}
