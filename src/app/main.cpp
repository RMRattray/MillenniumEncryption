#include <QApplication>
#include "mainwindow.h"
#include "packet.h"
#include <sqlite3.h>
#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ClientSocketManager sock();
    ClientDatabaseManager db();
    MainWindow window();

    connect(sock, &ClientSocketManager::mentionLoginSuccess, window, &MainWindow::showMainCentralWidget);
    connect(sock, &ClientSocketManager::mentionAccountResult, window->loginWidget, &LoginWidget::handleFailure);
    connect(sock, &ClientSocketManager::mentionLoginResult, window->loginWidget, &LoginWidget::handleFailure);
    
    connect(window->loginWidget, &LoginWidget::requestAccount, sock, &ClientSocketManager::sendAccountRequest);
    connect(window->loginWidget, &LoginWidget::requestLogin, sock, &ClientSocketManager::sendLoginRequest);

    connect(sock, &ClientSocketManager::mentionLoginSuccess, db, &ClientDatabaseManager::queryFriends);
    connect(db, &ClientDatabaseManager::outputFriendList, window->friendsBox, &FriendsBox::processFriendList);
    
    connect(sock, &ClientSocketManager::mentionFriendStatus, window->friendsBox, &FriendsBox::updateFriendStatus);
    connect(window->requestsBox, &RequestsBox::requestFriendRequest, sock, &ClientSocketManager::sendFriendRequest);
    connect(window->requestsBox, &RequestsBox::requestFriendResponse, sock, &ClientSocketManager::sendFriendRequest);
    connect(sock, &ClientSocketManager::mentionFriendRequest, window->requestsBox, &RequestsBox::processFriendRequest);
    connect(sock, &ClientSocketManager::mentionFriendRequestResponse, window->requestsBox, &RequestsBox::processFriendResponse);
    connect(window->requestsBox, &RequestsBox::announceNewFriend, window->friendsBox, &FriendsBox::addNewFriend);

    connect(sock, &ClientSocketManager::mentionMessage, window->codeBox, &CodeBox::decryptAndReceiveMessage);
    connect(window->codeBox, &CodeBox::requestMessageSend, sock, &ClientSocketManager::sendMessage);

    connect(db, &ClientDatabaseManager::outputMessageQuery, window->messagesBox, &MessagesBox::processMessages);
    connect(window->codeBox, &CodeBox::reportDecryptedMessage, db, &ClientDatabaseManager::insertMessage);
    connect(db, &ClientDatabaseManager::reportIncomingMessage, window->messagesBox, &MessagesBox::addMessage);
    connect(window->friendsBox, &FriendsBox::friendSelected, db, &ClientDatabaseManager::queryMessages);

    connect(window->sendButton, &QPushButton::clicked, app, [db, window](){ db->insertMessage(window->rightTextBox->text(), true); } );
    connect(db, &ClientDatabaseManager::reportOutgoingMessage, window->codeBox, &CodeBox::encryptAndSendMessage);

    window.show();

    int result = app.exec();
    
    return result;
}