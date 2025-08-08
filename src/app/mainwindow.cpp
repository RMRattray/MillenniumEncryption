#include "mainwindow.h"
#include "login.h"
#include "packet.h"
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

MainWindow::MainWindow(sqlite3 *db, QWidget *parent)
    : QMainWindow(parent), database(db)
{

    // Set up connection to server
    sock = new QTcpSocket();
    QAbstractSocket::SocketState s = sock->state();
    qDebug() << "About to attempt to connect";
    sock->connectToHost("127.0.0.1", 1999);
    if (sock->waitForConnected(3000))
        qDebug() << "Connected to server!\n";
    else {
        qDebug() << "Failed to connect to server\n";
    }

    // Login widget takes up the entire screen
    loginWidget = new LoginWidget(this, sock);

    mainCentralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainCentralWidget);
    // Left frame
    leftFrame = new QFrame(mainCentralWidget);
    leftFrame->setFrameShape(QFrame::StyledPanel);
    leftFrame->setFixedWidth(200);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    
    // Add FriendsBox and RequestsBox
    friendsBox = new FriendsBox(database, leftFrame);
    leftLayout->addWidget(friendsBox);
    
    requestsBox = new RequestsBox(database, sock, leftFrame);
    leftLayout->addWidget(requestsBox);
    
    leftLayout->addSpacing(10);

    // To be replaced by CodeBox:
    codebookLabel = new QLabel("Codebook: NONE", leftFrame);
    codebookLabel->setFixedHeight(30);
    leftLayout->addWidget(codebookLabel);
    viewCodebooksButton = new QPushButton("View codebooks", leftFrame);
    viewCodebooksButton->setFixedHeight(30);
    leftLayout->addWidget(viewCodebooksButton);


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
    messagesBox = new MessagesBox(database, rightFrame);
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
    // Connect friend selection to messages box
    connect(friendsBox, &FriendsBox::friendSelected, messagesBox, &MessagesBox::selectFriend);

    showLoginWidget();
    connect(loginWidget, &LoginWidget::logged_in, this, &MainWindow::showMainCentralWidget);
    connect(sock, &QTcpSocket::readyRead, this, &MainWindow::handlePacket);
}

void MainWindow::showLoginWidget()
{
    setCentralWidget(loginWidget);
}

void MainWindow::showMainCentralWidget(QString new_username)
{
    qDebug() << "Should be showing the main central widget now";
    requestsBox->my_name = new_username;
    qDebug() << "Successfully set a name property";
    setCentralWidget(mainCentralWidget);
    qDebug() << "And the central widget";
}

void MainWindow::handlePacket() {
    qDebug() << "Received a packet";
    unsigned char packet[PACKET_BUFFER_SIZE + 1];
    packet[PACKET_BUFFER_SIZE] = 0;
    if (sock->read((char *)packet, PACKET_BUFFER_SIZE) < 1) {
        qDebug() << "An error occurred";
    }
    else {
        switch (*packet) {
            case PacketFromServerType::ACCOUNT_RESULT:
                qDebug() << "It's an account result packet";
            case PacketFromServerType::LOGIN_RESULT:
                qDebug() << "It's a login result packet";
                loginWidget->handlePacket(packet);
                break;
            case PacketFromServerType::FRIEND_STATUS_UPDATE:
                qDebug() << "It's a friend status update packet";
                friendsBox->handlePacket(packet);
            case PacketFromServerType::FRIEND_REQUEST_RESPONSE:
                qDebug() << "It's a friend request response packet";
                requestsBox->handlePacket(packet);
                break;
            case PacketFromServerType::FRIEND_REQUEST_FORWARD:
                qDebug() << "It's a friend request forwarding packet";
                requestsBox->handlePacket(packet);
                break;
            default:
                qDebug() << "Received invalid packet";
        }
    }
}

MainWindow::~MainWindow()
{
    // Qt will delete child widgets automatically
}