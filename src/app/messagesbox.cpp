#include "messagesbox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QDebug>
#include <QSpacerItem>
#include <QTimer>
#include <QScrollBar>

MessagesBox::MessagesBox(sqlite3 *db, QWidget *parent)
    : QWidget(parent), database(db), currentFriendId(-1)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // Create scroll area for messages
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget for scroll area
    scrollContent = new QWidget(scrollArea);
    scrollContent->setLayout(new QVBoxLayout(scrollContent));
    scrollContent->layout()->setSpacing(5);
    scrollContent->layout()->setContentsMargins(5, 5, 5, 5);
    
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);
    
    // Create "no selection" label
    noSelectionLabel = new QLabel("Select a friend to view messages", this);
    noSelectionLabel->setAlignment(Qt::AlignCenter);
    noSelectionLabel->setStyleSheet("color: #666; font-size: 14px;");
    layout->addWidget(noSelectionLabel);
    
    // Initially show the no selection label
    scrollArea->hide();
    noSelectionLabel->show();
}

MessagesBox::~MessagesBox()
{
    // Qt will delete child widgets automatically
}

void MessagesBox::selectFriend(int friendId)
{
    if (friendId == currentFriendId) {
        return; // Already showing this friend's messages
    }
    
    currentFriendId = friendId;
    loadMessages(friendId);
}

void MessagesBox::loadMessages(int friendId)
{
    clearMessages();
    
    if (friendId <= 0) {
        scrollArea->hide();
        noSelectionLabel->show();
        return;
    }
    
    // Get friend name for display
    QString friendName;
    const char *nameSql = "SELECT friend_name FROM friends WHERE id = ?";
    sqlite3_stmt *nameStmt;
    int rc = sqlite3_prepare_v2(database, nameSql, -1, &nameStmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(nameStmt, 1, friendId);
        if (sqlite3_step(nameStmt) == SQLITE_ROW) {
            friendName = QString::fromUtf8((const char*)sqlite3_column_text(nameStmt, 0));
        }
    }
    sqlite3_finalize(nameStmt);
    
    // Load messages for this friend
    const char *sql = "SELECT message, original FROM messages WHERE friend_id = ? ORDER BY id";
    sqlite3_stmt *stmt;
    
    rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(database);
        return;
    }
    
    sqlite3_bind_int(stmt, 1, friendId);
    
    bool hasMessages = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *message = (const char*)sqlite3_column_text(stmt, 0);
        bool isOriginal = sqlite3_column_int(stmt, 1) != 0;
        
        MessageBox *messageBox = new MessageBox(QString::fromUtf8(message), isOriginal, scrollContent);
        scrollContent->layout()->addWidget(messageBox);
        hasMessages = true;
    }
    
    sqlite3_finalize(stmt);
    
    if (hasMessages) {
        // Show scroll area and hide no selection label
        scrollArea->show();
        noSelectionLabel->hide();
        
        // Scroll to the bottom to show latest messages
        QTimer::singleShot(100, [this]() {
            QScrollBar *scrollBar = scrollArea->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        });
    } else {
        // Show "no messages" label
        QLabel *noMessagesLabel = new QLabel("No messages yet", scrollContent);
        noMessagesLabel->setAlignment(Qt::AlignCenter);
        noMessagesLabel->setStyleSheet("color: #666; font-size: 14px;");
        scrollContent->layout()->addWidget(noMessagesLabel);
        
        scrollArea->show();
        noSelectionLabel->hide();
    }
}

void MessagesBox::clearMessages()
{
    // Remove all child widgets from scroll content
    QLayout *contentLayout = scrollContent->layout();
    QLayoutItem *item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

MessageBox::MessageBox(const QString &message, bool isOriginal, QWidget *parent)
    : QWidget(parent), messageText(message), isOriginalMessage(isOriginal)
{
    setFixedHeight(60);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    // Create message label
    messageLabel = new QLabel(message, this);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    // Style based on whether it's original or received
    if (isOriginal) {
        // Original message (sent by user) - align to right, blue background
        layout->addStretch();
        layout->addWidget(messageLabel);
        setStyleSheet("QWidget { background-color: #007AFF; border-radius: 10px; }");
        messageLabel->setStyleSheet("color: white; font-size: 12px; padding: 5px;");
    } else {
        // Received message - align to left, gray background
        layout->addWidget(messageLabel);
        layout->addStretch();
        setStyleSheet("QWidget { background-color: #E5E5EA; border-radius: 10px; }");
        messageLabel->setStyleSheet("color: black; font-size: 12px; padding: 5px;");
    }
} 