#include "friendsbox.h"
#include "packet.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <iostream>
#include <string>
#include <QString>

FriendsBox::FriendsBox(sqlite3 *db, QWidget *parent)
    : QWidget(parent), database(db), selectedFriendId(-1)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    
    loadFriends();
}

FriendsBox::~FriendsBox()
{
    // Qt will delete child widgets automatically
}

void FriendsBox::addFriend(int id, const QString &name, int status)
{
    if (friendWidgets.contains(id) || friendNameToId.contains(name)) {
        qDebug() << "Adding duplicate friend";
        return;
    }

    FriendBox *friendBox = new FriendBox(id, name, status, this);
    friendWidgets[id] = friendBox;
    friendNameToId[name] = id;
    layout->addWidget(friendBox);
    
    connect(friendBox, &FriendBox::friendClicked, this, [this](int friendId) {
        qDebug() << "Friend clicked:" << friendId;
        selectedFriendId = friendId;
        emit friendSelected(friendId);
    });
}

void FriendsBox::processFriendList(vector<QString> names) {
    int i = 0;
    while (i < names.size()) {
        addFriend(i, names[i], FriendStatus::OFFLINE);
    }
}


void FriendsBox::handlePacket(unsigned char *packet)
{
    std::cout << "Friends box received a packet of type: ";
    switch (*packet) {
        case PacketFromServerType::FRIEND_STATUS_UPDATE: {
            std::cout << "friend status update.\n";
            friendStatusUpdate statusUpdate(packet);
            QString username = QString::fromStdString(statusUpdate.username);
            qDebug() << "Status of: " << username;
            updateFriendStatus(username, static_cast<int>(statusUpdate.status));
            break;
        }
        case PacketFromServerType::FRIEND_REQUEST_RESPONSE: {
            std::cout << "friend request response.\n";
            friendRequestResponse response(packet);
            QString from = QString::fromStdString(response.from);
            if (response.response == FriendRequestResponse::ACCEPT) {
                addNewFriend(from, FriendStatus::ONLINE);
            }
            break;
        }
    }
}

void FriendsBox::updateFriendStatus(const QString &username, int status)
{   
    if (!friendNameToId.contains(username)) addFriend(friendNameToId.size(), username, status);
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
        emit friendClicked(friendId);
    }
    QWidget::mousePressEvent(event);
}

// FriendsBox methods for getting selected friend information
int FriendsBox::getSelectedFriendId() const
{
    return selectedFriendId;
}

QString FriendsBox::getSelectedFriendName() const
{
    if (selectedFriendId > 0 && friendWidgets.contains(selectedFriendId)) {
        return friendWidgets[selectedFriendId]->friendName;
    }
    return QString();
}

int FriendsBox::getSelectedFriendStatus() const
{
    if (selectedFriendId > 0 && friendWidgets.contains(selectedFriendId)) {
        return friendWidgets[selectedFriendId]->friendStatus;
    }
    return FriendStatus::OFFLINE;
}

bool FriendsBox::hasSelectedFriend() const
{
    return selectedFriendId > 0 && friendWidgets.contains(selectedFriendId);
}