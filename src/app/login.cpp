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
    if (sock->state() != QAbstractSocket::SocketState::ConnectedState) {
        qDebug() << "Unable to log in - not connected\n";
        return;
    }
    unsigned char packet[PACKET_BUFFER_SIZE];
    packetToServer * req;
    if (creating_account) req = new createAccountRequest(usernameEdit->text().toStdString(), passwordEdit->text().toStdString());
    else req = new loginRequest(usernameEdit->text().toStdString(), passwordEdit->text().toStdString());    
    req->write_to_packet(packet);
    sock->write((char *)packet, PACKET_BUFFER_SIZE);
    qDebug() << "Should have sent packet";
    delete req;    
}

void LoginWidget::handlePacket(unsigned char * packet) {
    packetFromServer * resp;
    createAccountResponse * car = NULL;
    loginResult * lgr = NULL;
    qDebug() << "Login widget received a packet";
    switch (*packet) {
        case PacketFromServerType::ACCOUNT_RESULT:
            resp = new createAccountResponse(packet);
            car = dynamic_cast<createAccountResponse *>(resp);
            qDebug() << "Processing packet";
            if (car->success) logged_in();
            else message->setText(QString::fromStdString("Failed to create account: " + car->reason));
        break;
        case PacketFromServerType::LOGIN_RESULT:
            resp = new loginResult(packet);
            lgr = dynamic_cast<loginResult *>(resp);
            if (lgr->success) logged_in();
            else message->setText(QString::fromStdString("Login failed: " + lgr->reason));
        break;
        default:
        qDebug() << "Invalid packet sent to login widget";
    }
    delete resp;
}