#include "login.h"
#include "packet.h"
#include <iostream>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QString>

LoginWidget::LoginWidget(QWidget *parent, QTcpSocket *s) : QWidget(parent) {
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
    message = new QLabel("", this);
    sock = s; // do not delete if this widget is deleted

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
    loginLayout->addWidget(message);
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
        creating_account = false;
    }
    else {
        loginButton->setText("Create account");
        createAccountInstead->setText("Return to login");
        creating_account = true;
    }
}

void LoginWidget::login() {
    loginButton->setText("Logging in");
    qDebug() << "Logging in";
    
    if (sock->state() != QAbstractSocket::SocketState::ConnectedState) {
        qDebug() << "Somehow not connected\n";
        return;
    }
    qDebug() << "Made it this far";
    createAccountRequest req(usernameEdit->text().toStdString(), passwordEdit->text().toStdString());
    unsigned char packet[PACKET_BUFFER_SIZE];
    req.write_to_packet(packet);
    sock->write((char *)packet, PACKET_BUFFER_SIZE);
    qDebug() << "Should be writing";    
}

void LoginWidget::handlePacket(unsigned char * packet) {
    qDebug() << "Login widget received a packet";
    switch (*packet) {
        case PacketFromServer::ACCOUNT_RESULT:
            createAccountResponse * resp;
            resp = new createAccountResponse(packet);
            qDebug() << "Processing packet";
            if (resp->success) {
                qDebug() << "Should be logging in";
                logged_in();
            }
            else {
                message->setText(QString::fromStdString("Failed to create account: " + resp->reason));
            }
        break;
        default:
        qDebug() << "Invalid packet sent to login widget";
    }
}