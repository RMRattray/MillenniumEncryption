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
#include <QPixmap>
#include <QDebug>

MainWindow::MainWindow(QString server_address, QWidget *parent)
    : QMainWindow(parent)
{
    // Add a logo to the top of the main window!
    QLabel * logo = new QLabel(this);
    QPixmap pixmap(":/images/badlogo.png");
    logo->setPixmap(pixmap);
    qDebug() << pixmap.width() << " " << pixmap.height();
    logo->setAlignment(Qt::AlignCenter);

    // Logo, and focus screen (either login, main, or error) go in layout together
    QWidget * container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container); 
    layout->addWidget(logo);
    setCentralWidget(container);
    logo->show();

    // Login widget takes up the entire screen
    loginWidget = new LoginWidget(container);
    layout->addWidget(loginWidget);

    mainCentralWidget = new QWidget(container);
    layout->addWidget(mainCentralWidget);
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

    errorScreen = new QLabel(container);
    layout->addWidget(errorScreen);

    db = new ClientDatabaseManager(this);
    sock = new ClientSocketManager(server_address, this);

    // // Connect friend selection to messages box
    // connect(friendsBox, &FriendsBox::friendSelected, messagesBox, &MessagesBox::selectFriend);
    
    // // Connect send button to handler
    // connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(sock, &ClientSocketManager::mentionSocketError, this, &MainWindow::showError);
    
    // Login widget sets login or account creation requests to the socket manager
    connect(loginWidget, &LoginWidget::requestAccount, sock, &ClientSocketManager::sendAccountRequest);
    connect(loginWidget, &LoginWidget::requestLogin, sock, &ClientSocketManager::sendLoginRequest);

    // Socket manager sends the results to the login widget
    connect(sock, &ClientSocketManager::mentionAccountResult, loginWidget, &LoginWidget::handleLoginResult);
    connect(sock, &ClientSocketManager::mentionLoginResult, loginWidget, &LoginWidget::handleLoginResult);
    
    // If login is successful, login widget tells the main window and the database manager user's handle
    connect(loginWidget, &LoginWidget::reportLoginSuccess, this, &MainWindow::showMainCentralWidget);
    connect(loginWidget, &LoginWidget::reportLoginSuccess, db, &ClientDatabaseManager::onLogin);
    // And the database reports known friends of that handle to the friends box
    connect(db, &ClientDatabaseManager::outputFriendList, friendsBox, &FriendsBox::processFriendList);
    
    connect(sock, &ClientSocketManager::mentionFriendStatus, friendsBox, &FriendsBox::updateFriendStatus);
    connect(requestsBox, &RequestsBox::requestFriendRequest, sock, &ClientSocketManager::sendFriendRequest);
    connect(requestsBox, &RequestsBox::requestFriendResponse, sock, &ClientSocketManager::sendFriendResponse);
    connect(sock, &ClientSocketManager::mentionFriendRequest, requestsBox, &RequestsBox::processFriendRequest);
    connect(sock, &ClientSocketManager::mentionFriendResponse, requestsBox, &RequestsBox::processFriendResponse);
    connect(requestsBox, &RequestsBox::announceNewFriend, db, &ClientDatabaseManager::insertFriend);
    connect(db, &ClientDatabaseManager::reportNewFriend, friendsBox, &FriendsBox::addNewFriend);
    connect(friendsBox, &FriendsBox::reportNewFriend, db, &ClientDatabaseManager::insertFriend);

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
    loginWidget->show();
    errorScreen->hide();
    mainCentralWidget->hide();
}

void MainWindow::showMainCentralWidget(QString username) {
    // TODO:  Put the username on screen somewhere?
    mainCentralWidget->show();
    errorScreen->hide();
    loginWidget->hide();
}

void MainWindow::showError(QString message) {
    errorScreen->setText(message);
    errorScreen->show();
    loginWidget->hide();
    mainCentralWidget->hide();
}

MainWindow::~MainWindow()
{
    // Qt will delete child widgets automatically
}