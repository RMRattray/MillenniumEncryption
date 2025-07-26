#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "login.h"
#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Login screen widgets
    LoginWidget *loginWidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;

    // Main central widget and its components
    QWidget *mainCentralWidget;
    QFrame *leftFrame;
    QFrame *rightFrame;
    QLabel *codebookLabel;
    QPushButton *viewCodebooksButton;
    QFrame *leftEmptyFrame;
    QFrame *rightEmptyFrame;
    QLineEdit *rightTextBox;
    QPushButton *sendButton;

    QTcpSocket *sock;

    void showLoginWidget();
    void showMainCentralWidget();
    void handlePacket();
};

#endif