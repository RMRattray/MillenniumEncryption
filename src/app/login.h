#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();
    QTcpSocket *sock;    
private slots:
    void swapLoginPurpose();
    void login();
private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *createAccountInstead;
    bool creating_account;
    QPushButton *loginButton;
};

#endif