#ifndef FRIENDSBOX_H
#define FRIENDSBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <sqlite3.h>

class FriendBox;

class FriendsBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsBox(sqlite3 *db, QWidget *parent = nullptr);
    ~FriendsBox();

private:
    sqlite3 *database;
    QVBoxLayout *layout;
    QMap<int, FriendBox*> friendWidgets;
    
    void loadFriends();
    void addFriend(int id, const QString &name, int status);
};

class FriendBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendBox(int friendId, const QString &name, int status, QWidget *parent = nullptr);
    
signals:
    void friendClicked(int friendId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int friendId;
    QString friendName;
    int friendStatus;
};

#endif // FRIENDSBOX_H 