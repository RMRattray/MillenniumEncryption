#include "login.h"
#include "packet.h"
#include <iostream>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QLabel>

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent) {
    // Login widgets
    QVBoxLayout *loginLayout = new QVBoxLayout(this);
    QLabel *userLabel = new QLabel("Username", this);
    usernameEdit = new QLineEdit(this);
    QLabel *passLabel = new QLabel("Password", this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Login", this);
    createAccountInstead = new QPushButton("Create new account", this);
    createAccountInstead->setFlat(true);
    creating_account = false;
    sock = new QTcpSocket(parent); // do not delete if this widget is deleted

    // Layout is one big column for now
    loginLayout->addStretch();
    loginLayout->addWidget(userLabel);
    loginLayout->addWidget(usernameEdit);
    loginLayout->addSpacing(5);
    loginLayout->addWidget(passLabel);
    loginLayout->addWidget(passwordEdit);
    loginLayout->addSpacing(5);
    loginLayout->addWidget(loginButton);
    loginLayout->addSpacing(5);
    loginLayout->addWidget(createAccountInstead);
    loginLayout->addStretch();

    connect(createAccountInstead, SIGNAL(clicked()), this, SLOT(swapLoginPurpose()));
    connect(loginButton, SIGNAL(clicked()), this, SLOT(login()));
};

LoginWidget::~LoginWidget() {

}

void LoginWidget::swapLoginPurpose() {
    if (creating_account) {
        loginButton->setText("Login");
        createAccountInstead->setText("Create new account");
    }
    else {
        loginButton->setText("Create account");
        createAccountInstead->setText("Return to login");
    }
}

void LoginWidget::login() {
    QAbstractSocket::SocketState s = sock->state();
    // if connecting from another click to the login button, return
    if (s == QAbstractSocket::SocketState::ConnectingState || QAbstractSocket::SocketState::HostLookupState) return;
    // if not connected, connect!
    if (s == QAbstractSocket::SocketState::UnconnectedState) {
        sock->connectToHost("127.0.01", 1999);
        if (sock->waitForConnected(3000))
            std::cout << "Connected to server!\n";
        else {
            std::cout << "Failed to connect to server\n";
        }
    }
    if (sock->state() != QAbstractSocket::SocketState::ConnectedState) {
        std::cout << "Somehow not connected\n";
        return;
    }

    createAccountRequest req(usernameEdit->text().toStdString(), passwordEdit->text().toStdString());
    unsigned char packet[PACKET_BUFFER_SIZE];
    req.write_to_packet(packet);
    sock->write((char *)packet, PACKET_BUFFER_SIZE);
}