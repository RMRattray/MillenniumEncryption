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
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();
    void handlePacket(unsigned char * packet);
signals:
    void requestAccount(QString username, QString password);
    void requestLogin(QString username, QString password);
    void reportLoginSuccess(QString username);
public slots:
    void handleLoginResult(bool result, QString reason);
private slots:
    void swapLoginPurpose();
    void login();
private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *createAccountInstead;
    bool creating_account;
    QPushButton *loginButton;
    QLabel *message;
};

#endif