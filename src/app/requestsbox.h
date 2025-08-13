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
#include "packet.h"

class RequestBox;
class QTcpSocket;

class RequestsBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestsBox(QWidget *parent = nullptr);
    ~RequestsBox();

    void handlePacket(unsigned char *packet);
    QString my_name;
signals:
    void requestFriendRequest(QString name);
    void requestFriendResponse(QString name, FriendRequestResponse resp);
    void announceNewFriend(QString name);

public slots:
    void processFriendRequest(QString name);
    void processFriendResponse(QString name, FriendRequestResponse resp);

private:
    QVBoxLayout *layout;
    QMap<QString, RequestBox*> requestWidgets;
    
    void addRequest(const QString &from, bool hasButtons);
    void removeRequest(const QString &from);
    void sendFriendRequestAcknowledge(const QString &to, const QString &from, int response);
    void sendFriendRequestSend(const QString &target);

private slots:
    void createFriendRequest();
};

class RequestBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestBox(const QString &username, bool hasButtons, QWidget *parent = nullptr);
    void handlePacket(unsigned char *packet);
    bool is_pending;

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