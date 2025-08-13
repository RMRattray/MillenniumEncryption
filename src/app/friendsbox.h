#ifndef FRIENDSBOX_H
#define FRIENDSBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <sqlite3.h>
#include <vector>

class FriendBox;

class FriendsBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsBox(QWidget *parent = nullptr);
    ~FriendsBox();

public slots:
    void updateFriendStatus(const QString &username, int status);
    void addNewFriend(int id, const QString &username, int status);
    void processFriendList(std::vector<QString> friend_names);

signals:
    void friendSelected(QString friend_name);

private:
    QVBoxLayout *layout;
    QMap<int, FriendBox*> friendWidgets;
    QMap<QString, int> friendNameToId;
    int selectedFriendId;
};

class FriendBox : public QWidget
{
    Q_OBJECT

public:
    explicit FriendBox(int friendId, const QString &name, int status, QWidget *parent = nullptr);
    void updateStatus(int status);
    
    QString friendName;
    int friendStatus;
signals:
    void friendClicked(QString friendName);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int friendId;
    QLabel *statusLabel;
};

#endif // FRIENDSBOX_H 