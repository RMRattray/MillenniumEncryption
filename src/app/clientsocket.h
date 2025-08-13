#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include "packet.h"
#include <QTcpSocket>

class ClientSocketManager : public QObject 
{
    Q_OBJECT 

public:
    explicit ClientSocketManager(QString server_address, QObject *parent = nullptr);
    ~ClientSocketManager();
    
signals:
    void mentionAccountResult(QString reason);
    void mentionLoginResult(QString reason);
    void mentionLoginSuccess();
    void mentionFriendStatus(QString friend_name, FriendStatus status);
    void mentionFriendRequest(QString friend_name);
    void mentionFriendRequestResponse(QString friend_name, FriendRequestResponse response);
    void mentionMessage(QString friend_name, QString message);

public slots:
    void sendAccountRequest(QString user_name, QString password);
    void sendLoginRequest(QString user_name, QString password);
    void sendFriendRequest(QString friend_name);
    void sendFriendRequestResponse(QString friend_name, FriendRequestResponse response);
    void sendMessage(QString friend_name, QString message);

private:
    void handlePacket(char * packet);
    QTcpSocket * sock;
    QString connectedUser;
};

#endif 