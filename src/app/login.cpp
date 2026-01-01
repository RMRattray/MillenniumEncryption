#include "login.h"
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QString>

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
    message = new QLabel("", this);

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
    if (creating_account) emit requestAccount(usernameEdit->text(), passwordEdit->text());
    else emit requestLogin(usernameEdit->text(), passwordEdit->text());
}

void LoginWidget::handleLoginResult(bool result, QString reason) {
    if (result) reportLoginSuccess(usernameEdit->text()); // should this be stored on login attempt in case user types before login result?
    else if (creating_account) message->setText("Failed to create account: " + reason);
    else message->setText("Failed to log in: " + reason);
}