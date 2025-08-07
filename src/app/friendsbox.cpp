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
    : QWidget(parent), database(db)
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

void FriendsBox::loadFriends()
{
    const char *sql = "SELECT id, friend_name, status FROM friends ORDER BY friend_name";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(database);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int status = sqlite3_column_int(stmt, 2);

        qDebug() << "We have a friend named: " << name;
        
        addFriend(id, QString::fromUtf8(name), status);
    }
    
    sqlite3_finalize(stmt);
}

void FriendsBox::addFriend(int id, const QString &name, int status)
{
    FriendBox *friendBox = new FriendBox(id, name, status, this);
    friendWidgets[id] = friendBox;
    friendNameToId[name] = id;
    layout->addWidget(friendBox);
    
    connect(friendBox, &FriendBox::friendClicked, this, [this](int friendId) {
        qDebug() << "Friend clicked:" << friendId;
        emit friendSelected(friendId);
    });
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
    if (!friendNameToId.contains(username)) addNewFriend(username, status);
    else {
        int id = friendNameToId[username];
        if (friendWidgets.contains(id)) {
            friendWidgets[id]->updateStatus(status);
            
            // Update database
            const char *sql = "UPDATE friends SET status = ? WHERE id = ?";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
            if (rc == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, status);
                sqlite3_bind_int(stmt, 2, id);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }
}

void FriendsBox::addNewFriend(const QString &username, int status)
{
    // Check if friend already exists
    if (friendNameToId.contains(username)) {
        return;
    }
    
    int id = insertFriendToDatabase(username, status);
    if (id > 0) {
        addFriend(id, username, status);
    }
}

int FriendsBox::insertFriendToDatabase(const QString &name, int status)
{
    std::string namechars = name.toStdString();

    const char *sql_chk = "SELECT COUNT(*) FROM friends WHERE friend_name = ? ;";
    sqlite3_stmt *stmt_chk;
    int rc = sqlite3_prepare_v2(database, sql_chk, -1, &stmt_chk, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(database);
        return -1;
    }
    sqlite3_bind_text(stmt_chk, 1, namechars.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt_chk);
    int ans = sqlite3_column_int(stmt_chk, 0);
    sqlite3_finalize(stmt_chk);

    if (ans) {
        qDebug() << "Caution!  Attempted to add duplicate friend " << name << "to database.";
        return -1;
    }

    const char *sql = "INSERT INTO friends (friend_name, status) VALUES (?, ?)";
    sqlite3_stmt *stmt;
    
    rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(database);
        return -1;
    }

    qDebug() << "Name of friend added to database: " << name;
    
    

    qDebug() << "Bound as: " << QString::fromStdString(std::string(namechars.c_str()));

    sqlite3_bind_text(stmt, 1, namechars.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, status);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        return sqlite3_last_insert_rowid(database);
    }
    
    return -1;
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