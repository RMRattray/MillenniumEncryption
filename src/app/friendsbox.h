#ifndef FRIENDSBOX_H
#define FRIENDSBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <sqlite3.h>

class FriendBox;

class FriendsBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsBox(sqlite3 *db, QWidget *parent = nullptr);
    ~FriendsBox();

    void handlePacket(unsigned char *packet);
    void updateFriendStatus(const QString &username, int status);
    void addNewFriend(const QString &username, int status);

private:
    sqlite3 *database;
    QVBoxLayout *layout;
    QMap<int, FriendBox*> friendWidgets;
    QMap<QString, int> friendNameToId;
    
    void loadFriends();
    void addFriend(int id, const QString &name, int status);
    int insertFriendToDatabase(const QString &name, int status);
};

class FriendBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendBox(int friendId, const QString &name, int status, QWidget *parent = nullptr);
    void handlePacket(unsigned char *packet);
    void updateStatus(int status);
    
signals:
    void friendClicked(int friendId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int friendId;
    QString friendName;
    int friendStatus;
    QLabel *statusLabel;
};

#endif // FRIENDSBOX_H 