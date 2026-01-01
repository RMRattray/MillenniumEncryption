#include "friendsbox.h"
#include "packet.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <iostream>
#include <string>
#include <QString>
#include "vector"
#include <QScrollArea>
#include <QFrame>

FriendsBox::FriendsBox(QWidget *parent)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    scroll = new QScrollArea(this);
    // scroll->setWidget(frame);

    QWidget* scrollContent = new QWidget(scroll);
    scrollContent->setLayout(new QVBoxLayout(scrollContent));
    scrollContent->layout()->setSpacing(5);
    scrollContent->layout()->setContentsMargins(5, 5, 5, 5);
    
    scroll->setWidget(scrollContent);
    layout->addWidget(scroll);
}

FriendsBox::~FriendsBox()
{
    // Qt will delete child widgets automatically
}

void FriendsBox::addNewFriend(int id, const QString &name, int status)
{
    if (friendWidgets.contains(id) || friendNameToId.contains(name)) {
        qDebug() << "Adding duplicate friend";
        return;
    }

    FriendBox *friendBox = new FriendBox(id, name, status, this);
    friendWidgets[id] = friendBox;
    friendNameToId[name] = id;
    layout->addWidget(friendBox);
    
    connect(friendBox, &FriendBox::friendClicked, this, [this](QString friendName) {
        qDebug() << "Friend clicked:" << friendName;
        // selectedFriendId = friendId;
        emit friendSelected(friendName);
    });
}

void FriendsBox::processFriendList(std::vector<QString> names) {
    int i = 0;
    while (i < names.size()) {
        addNewFriend(i, names[i], FriendStatus::OFFLINE);
        ++i;
    }
}

void FriendsBox::updateFriendStatus(const QString &username, int status)
{   
    if (!friendNameToId.contains(username)) {
        addNewFriend(friendNameToId.size(), username, status);
        reportNewFriend(username);
    }
    else {
        int id = friendNameToId[username];
        if (friendWidgets.contains(id)) {
            friendWidgets[id]->updateStatus(status);
        }
    }
}

FriendBox::FriendBox(int friendId, const QString &name, int status, QWidget *parent)
    : QWidget(parent), friendId(friendId), friendName(name), friendStatus(status)
{
    setFixedHeight(40);
    setStyleSheet("QWidget { background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 4px; }");
    setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    QLabel *nameLabel = new QLabel(name, this);
    nameLabel->setStyleSheet("font-weight: bold; color: #333;");
    layout->addWidget(nameLabel);
    
    layout->addStretch();
    
    QString statusText = (status == FriendStatus::ONLINE) ? "ONLINE" : "OFFLINE";
    QString statusColor = (status == FriendStatus::ONLINE) ? "#4CAF50" : "#9E9E9E";
    
    statusLabel = new QLabel(statusText, this);
    statusLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold;").arg(statusColor));
    layout->addWidget(statusLabel);
}

void FriendBox::updateStatus(int status)
{
    friendStatus = status;
    QString statusText = (status == FriendStatus::ONLINE) ? "ONLINE" : "OFFLINE";
    QString statusColor = (status == FriendStatus::ONLINE) ? "#4CAF50" : "#9E9E9E";
    
    statusLabel->setText(statusText);
    statusLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold;").arg(statusColor));
}

void FriendBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit friendClicked(friendName);
    }
    QWidget::mousePressEvent(event);
}