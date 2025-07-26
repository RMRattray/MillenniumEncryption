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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    sock = new QTcpSocket();
    QAbstractSocket::SocketState s = sock->state();
    qDebug() << "About to attempt to connect";
    sock->connectToHost("127.0.0.1", 1999);
    if (sock->waitForConnected(3000))
        qDebug() << "Connected to server!\n";
    else {
        qDebug() << "Failed to connect to server\n";
    }
    showLoginWidget();
    connect(sock, &QTcpSocket::readyRead, this, &MainWindow::handlePacket);
}

void MainWindow::showLoginWidget()
{
    loginWidget = new LoginWidget(this, sock);
    setCentralWidget(loginWidget);
    connect(loginWidget, &LoginWidget::logged_in, this, &MainWindow::showMainCentralWidget);
    // connect(loginButton, &QPushButton::clicked, this, [this]() {
    //     if (usernameEdit->text() == "name" && passwordEdit->text() == "word") {
    //         showMainCentralWidget();
    //     } else {
    //         loginButton->setText("Try again");
    //     }
    // });
}

void MainWindow::showMainCentralWidget()
{
    mainCentralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainCentralWidget);
    // Left frame
    leftFrame = new QFrame(mainCentralWidget);
    leftFrame->setFrameShape(QFrame::StyledPanel);
    leftFrame->setFixedWidth(200);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftEmptyFrame = new QFrame(leftFrame);
    leftEmptyFrame->setFrameShape(QFrame::NoFrame);
    leftLayout->addWidget(leftEmptyFrame);
    leftLayout->addSpacing(10);
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
    // Right frame
    rightFrame = new QFrame(mainCentralWidget);
    rightFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightEmptyFrame = new QFrame(rightFrame);
    rightEmptyFrame->setFrameShape(QFrame::NoFrame);
    rightLayout->addWidget(rightEmptyFrame);
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
    setCentralWidget(mainCentralWidget);
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
            loginWidget->handlePacket(packet);
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