#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "login.h"
#include "friendsbox.h"
#include "requestsbox.h"
#include "messagesbox.h"
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

private:
    // Login screen widgets
    LoginWidget *loginWidget;

    // Main central widget and its components
    QWidget *mainCentralWidget;
    QFrame *leftFrame;
    QFrame *rightFrame;
    FriendsBox * friendsBox;
    RequestsBox *requestsBox;
    MessagesBox *messagesBox;
    QLabel *codebookLabel;
    QPushButton *viewCodebooksButton;
    QFrame *rightEmptyFrame;
    QLineEdit *rightTextBox;
    QPushButton *sendButton;

    QTcpSocket *sock;
    sqlite3 *database;

    void showLoginWidget();
    void showMainCentralWidget(QString);
    void handlePacket();
};

#endif