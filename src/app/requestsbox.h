#ifndef REQUESTSBOX_H
#define REQUESTSBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <sqlite3.h>

class RequestBox;
class QTcpSocket;

class RequestsBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestsBox(sqlite3 *db, QTcpSocket *socket, QWidget *parent = nullptr);
    ~RequestsBox();

    void handlePacket(unsigned char *packet);
    QString my_name;

private slots:
    void createFriendRequest();

private:
    sqlite3 *database;
    QTcpSocket *socket;
    QVBoxLayout *layout;
    QMap<QString, RequestBox*> requestWidgets;
    
    void addRequest(const QString &from, bool hasButtons);
    void removeRequest(const QString &from);
    void sendFriendRequestAcknowledge(const QString &to, const QString &from, int response);
    void sendFriendRequestSend(const QString &target);
};

class RequestBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestBox(const QString &username, bool hasButtons, QWidget *parent = nullptr);
    void handlePacket(unsigned char *packet);

signals:
    void acceptRequest(const QString &username);
    void rejectRequest(const QString &username);
    void hideRequest(const QString &username);

private slots:
    void onAcceptClicked();
    void onRejectClicked();
    void onHideClicked();

private:
    QString username;
    QLabel *statusLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;
    QPushButton *hideButton;
};

class FriendRequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FriendRequestDialog(QWidget *parent = nullptr);
    QString getUsername() const;

private slots:
    void onSendClicked();
    void onCancelClicked();

private:
    QLineEdit *usernameEdit;
    QString username;
};

#endif // REQUESTSBOX_H 