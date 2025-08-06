#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QLabel>

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr, QTcpSocket * s = nullptr);
    ~LoginWidget();
    QTcpSocket *sock;
    void handlePacket(unsigned char * packet);
    QLineEdit *usernameEdit;
signals:
    void logged_in(QString username);
private slots:
    void swapLoginPurpose();
    void login();
private:
    QLineEdit *passwordEdit;
    QPushButton *createAccountInstead;
    bool creating_account;
    QPushButton *loginButton;
    QLabel *message;
};

#endif