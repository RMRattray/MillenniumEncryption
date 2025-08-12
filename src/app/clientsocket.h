#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include "packet.h"

class ClientSocketManager : public QObject 
{
    Q_OBJECT 

public:
    explicit ClientSocketManager(QString server_address, QObject *parent = nullptr);
    ~ClientSocketManager();

public slots:
    void sendAccountRequest(QString user_name, QString password);
    void sendLoginRequest(QString user_name, QString password);
    void sendFriendRequest(QString friend);
    void sendFriendRequestResponse(QString friend, FriendRequestResponse response);
    void sendMessage(QString friend, QString message);

signals:
    void mentionAccountResult(QString reason);
    void mentionLoginResult(QString reason);
    void mentionLoginSuccess();
    void mentionFriendStatus(QString friend, FriendStatus status);
    void mentionFriendRequest(QString friend);
    void mentionFriendRequestResponse(QString friend, FriendRequestResponse response);
    void mentionMessage(QString friend, QString message);

private:
    void handlePacket(char * packet);
    QTcpSocket * sock;
    QString connectedUser;
};

#endif 