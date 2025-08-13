#include "mainwindow.h"
#include "clientsocket.h"
#include "database.h"
#include "codebox.h"
#include "friendsbox.h"
#include "messagesbox.h"
#include "requestsbox.h"
#include "login.h"
#include "packet.h"
#include "../server/packet.h"
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QSpacerItem>
#include <QTcpSocket>

MainWindow::MainWindow(QString server_address, QWidget *parent)
    : QMainWindow(parent)
{
    // Login widget takes up the entire screen
    loginWidget = new LoginWidget(this);

    mainCentralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainCentralWidget);
    // Left frame
    leftFrame = new QFrame(mainCentralWidget);
    leftFrame->setFrameShape(QFrame::StyledPanel);
    leftFrame->setFixedWidth(200);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    
    // Add FriendsBox and RequestsBox
    friendsBox = new FriendsBox(leftFrame);
    leftLayout->addWidget(friendsBox);
    
    requestsBox = new RequestsBox(leftFrame);
    leftLayout->addWidget(requestsBox);
    
    leftLayout->addSpacing(10);

    // Add CodeBox
    codeBox = new CodeBox(leftFrame);
    leftLayout->addWidget(codeBox);


    leftLayout->setStretch(0, 1);
    leftLayout->setStretch(1, 0);
    leftLayout->setStretch(2, 0);
    leftLayout->setStretch(3, 0);
    leftLayout->setStretch(4, 0);

    // Right frame
    rightFrame = new QFrame(mainCentralWidget);
    rightFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    
    // Create MessagesBox to replace the empty frame
    messagesBox = new MessagesBox(rightFrame);
    rightLayout->addWidget(messagesBox);
    QHBoxLayout *bottomRow = new QHBoxLayout();
    rightTextBox = new QLineEdit(rightFrame);
    bottomRow->addWidget(rightTextBox);
    sendButton = new QPushButton(rightFrame);
    sendButton->setFixedWidth(50);
    sendButton->setIcon(QIcon::fromTheme("send"));
    bottomRow->addWidget(sendButton);
    rightLayout->addLayout(bottomRow);
    rightLayout->setStretch(0, 1);
    rightLayout->setStretch(1, 0);
    mainLayout->addWidget(leftFrame);
    mainLayout->addWidget(rightFrame);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    db = new ClientDatabaseManager(this);
    sock = new ClientSocketManager(server_address, this);

    // // Connect friend selection to messages box
    // connect(friendsBox, &FriendsBox::friendSelected, messagesBox, &MessagesBox::selectFriend);
    
    // // Connect send button to handler
    // connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);

    
    connect(sock, &ClientSocketManager::mentionLoginSuccess, this, &MainWindow::showMainCentralWidget);
    connect(sock, &ClientSocketManager::mentionAccountResult, loginWidget, &LoginWidget::handleFailure);
    connect(sock, &ClientSocketManager::mentionLoginResult, loginWidget, &LoginWidget::handleFailure);
    
    connect(loginWidget, &LoginWidget::requestAccount, sock, &ClientSocketManager::sendAccountRequest);
    connect(loginWidget, &LoginWidget::requestLogin, sock, &ClientSocketManager::sendLoginRequest);

    connect(sock, &ClientSocketManager::mentionLoginSuccess, db, &ClientDatabaseManager::queryFriends);
    connect(db, &ClientDatabaseManager::outputFriendList, friendsBox, &FriendsBox::processFriendList);
    
    connect(sock, &ClientSocketManager::mentionFriendStatus, friendsBox, &FriendsBox::updateFriendStatus);
    connect(requestsBox, &RequestsBox::requestFriendRequest, sock, &ClientSocketManager::sendFriendRequest);
    connect(requestsBox, &RequestsBox::requestFriendResponse, sock, &ClientSocketManager::sendFriendResponse);
    connect(sock, &ClientSocketManager::mentionFriendRequest, requestsBox, &RequestsBox::processFriendRequest);
    connect(sock, &ClientSocketManager::mentionFriendResponse, requestsBox, &RequestsBox::processFriendResponse);
    connect(requestsBox, &RequestsBox::announceNewFriend, db, &ClientDatabaseManager::insertFriend);
    connect(db, &ClientDatabaseManager::reportNewFriend, friendsBox, &FriendsBox::addNewFriend);

    connect(sock, &ClientSocketManager::mentionMessage, codeBox, &CodeBox::decryptAndReceiveMessage);
    connect(codeBox, &CodeBox::requestMessageSend, sock, &ClientSocketManager::sendMessage);

    connect(db, &ClientDatabaseManager::outputMessageQuery, messagesBox, &MessagesBox::processMessages);
    connect(codeBox, &CodeBox::reportDecryptedMessage, db, &ClientDatabaseManager::insertMessage);
    connect(db, &ClientDatabaseManager::reportIncomingMessage, messagesBox, &MessagesBox::addMessage);
    connect(friendsBox, &FriendsBox::friendSelected, db, &ClientDatabaseManager::queryFriendMessages);

    connect(sendButton, &QPushButton::clicked, this, [this](){ this->db->insertMessage(this->rightTextBox->text(), true); } );
    connect(db, &ClientDatabaseManager::reportOutgoingMessage, codeBox, &CodeBox::encryptAndSendMessage);

    showLoginWidget();
}

void MainWindow::showLoginWidget()
{
    setCentralWidget(loginWidget);
}

void MainWindow::showMainCentralWidget() {
    setCentralWidget(mainCentralWidget);
}

MainWindow::~MainWindow()
{
    // Qt will delete child widgets automatically
}