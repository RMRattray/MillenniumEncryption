#include "friendsbox.h"
#include "packet.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <iostream>

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
        
        addFriend(id, QString::fromUtf8(name), status);
    }
    
    sqlite3_finalize(stmt);
}

void FriendsBox::addFriend(int id, const QString &name, int status)
{
    FriendBox *friendBox = new FriendBox(id, name, status, this);
    friendWidgets[id] = friendBox;
    layout->addWidget(friendBox);
    
    connect(friendBox, &FriendBox::friendClicked, this, [this](int friendId) {
        qDebug() << "Friend clicked:" << friendId;
        // Emit signal or handle friend selection
    });
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
    
    QLabel *statusLabel = new QLabel(statusText, this);
    statusLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold;").arg(statusColor));
    layout->addWidget(statusLabel);
}

void FriendBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit friendClicked(friendId);
    }
    QWidget::mousePressEvent(event);
} 