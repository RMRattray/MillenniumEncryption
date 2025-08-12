#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "login.h"
#include "friendsbox.h"
#include "requestsbox.h"
#include "messagesbox.h"
#include "codebox.h"
#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <sqlite3.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(sqlite3 *db, QWidget *parent = nullptr);
    ~MainWindow();

    LoginWidget *loginWidget;
private:
    // Login screen widgets

    // Main central widget and its components
    QWidget *mainCentralWidget;
    QFrame *leftFrame;
    QFrame *rightFrame;
    FriendsBox * friendsBox;
    RequestsBox *requestsBox;
    MessagesBox *messagesBox;
    CodeBox *codeBox;
    QFrame *rightEmptyFrame;
    QLineEdit *rightTextBox;
    QPushButton *sendButton;

    void showLoginWidget();
    void showMainCentralWidget(QString);
    void handlePacket();
    
private slots:
    void onSendButtonClicked();
};

#endif