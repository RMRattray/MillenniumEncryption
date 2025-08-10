#include "database.h"
#include <QDebug>

ClientDatabaseManager::ClientDatabaseManager(QObject *parent)
    : QObject(parent)
    , database(nullptr)
{
    // Initialize SQLite database
    int rc = sqlite3_open("client.db", &database);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to open database:" << sqlite3_errmsg(database);
        return;
    }

    // Create friends table (without status field)
    const char* createFriendsTable = 
        "CREATE TABLE IF NOT EXISTS friends ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "friend_name TEXT NOT NULL)";

    rc = sqlite3_exec(database, createFriendsTable, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to create friends table:" << sqlite3_errmsg(database);
        return;
    }

    // Create messages table
    const char* createMessagesTable = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "friend_id INTEGER NOT NULL, "
        "message TEXT NOT NULL, "
        "original BOOLEAN NOT NULL, "
        "FOREIGN KEY (friend_id) REFERENCES friends (id))";

    rc = sqlite3_exec(database, createMessagesTable, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to create messages table:" << sqlite3_errmsg(database);
        return;
    }

    qDebug() << "Database initialized successfully";
}

ClientDatabaseManager::~ClientDatabaseManager()
{
    if (database) {
        sqlite3_close(database);
        database = nullptr;
    }
}

void ClientDatabaseManager::insertFriend(QString name)
{
    if (!database) return;

    const char* sql = "INSERT INTO friends (friend_name) VALUES (?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare insert friend statement:" << sqlite3_errmsg(database);
        return;
    }

    sqlite3_bind_text(stmt, 1, name.toUtf8().constData(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qDebug() << "Failed to insert friend:" << sqlite3_errmsg(database);
    }

    sqlite3_finalize(stmt);
}

void ClientDatabaseManager::insertMessage(QString message, bool original, QString friend)
{
    if (!database) return;

    // First get the friend_id from the friends table
    const char* getFriendIdSql = "SELECT id FROM friends WHERE friend_name = ?";
    sqlite3_stmt* getFriendStmt;
    
    int rc = sqlite3_prepare_v2(database, getFriendIdSql, -1, &getFriendStmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare get friend id statement:" << sqlite3_errmsg(database);
        return;
    }

    sqlite3_bind_text(getFriendStmt, 1, friend.toUtf8().constData(), -1, SQLITE_STATIC);
    
    int friendId = -1;
    if (sqlite3_step(getFriendStmt) == SQLITE_ROW) {
        friendId = sqlite3_column_int(getFriendStmt, 0);
    }
    
    sqlite3_finalize(getFriendStmt);
    
    if (friendId == -1) {
        qDebug() << "Friend not found:" << friend;
        return;
    }

    // Insert the message
    const char* insertMessageSql = "INSERT INTO messages (friend_id, message, original) VALUES (?, ?, ?)";
    sqlite3_stmt* insertStmt;
    
    rc = sqlite3_prepare_v2(database, insertMessageSql, -1, &insertStmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare insert message statement:" << sqlite3_errmsg(database);
        return;
    }

    sqlite3_bind_int(insertStmt, 1, friendId);
    sqlite3_bind_text(insertStmt, 2, message.toUtf8().constData(), -1, SQLITE_STATIC);
    sqlite3_bind_int(insertStmt, 3, original ? 1 : 0);
    
    rc = sqlite3_step(insertStmt);
    if (rc != SQLITE_DONE) {
        qDebug() << "Failed to insert message:" << sqlite3_errmsg(database);
    }

    sqlite3_finalize(insertStmt);
}

void ClientDatabaseManager::queryFriends()
{
    if (!database) return;

    std::vector<QString> friends;
    const char* sql = "SELECT friend_name FROM friends ORDER BY friend_name";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare query friends statement:" << sqlite3_errmsg(database);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* friendName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        friends.push_back(QString::fromUtf8(friendName));
    }

    sqlite3_finalize(stmt);
    
    emit outputFriendList(friends);
}

void ClientDatabaseManager::queryMessages(QString friend, int count, int before)
{
    if (!database) return;

    // First get the friend_id
    const char* getFriendIdSql = "SELECT id FROM friends WHERE friend_name = ?";
    sqlite3_stmt* getFriendStmt;
    
    int rc = sqlite3_prepare_v2(database, getFriendIdSql, -1, &getFriendStmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare get friend id statement:" << sqlite3_errmsg(database);
        return;
    }

    sqlite3_bind_text(getFriendStmt, 1, friend.toUtf8().constData(), -1, SQLITE_STATIC);
    
    int friendId = -1;
    if (sqlite3_step(getFriendStmt) == SQLITE_ROW) {
        friendId = sqlite3_column_int(getFriendStmt, 0);
    }
    
    sqlite3_finalize(getFriendStmt);
    
    if (friendId == -1) {
        qDebug() << "Friend not found:" << friend;
        return;
    }

    // Query messages
    std::vector<std::tuple<QString, bool>> messages;
    int firstMessageId = 0;
    
    const char* sql;
    if (before > 0) {
        sql = "SELECT id, message, original FROM messages WHERE friend_id = ? AND id < ? ORDER BY id DESC LIMIT ?";
    } else {
        sql = "SELECT id, message, original FROM messages WHERE friend_id = ? ORDER BY id DESC LIMIT ?";
    }
    
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Failed to prepare query messages statement:" << sqlite3_errmsg(database);
        return;
    }

    if (before > 0) {
        sqlite3_bind_int(stmt, 1, friendId);
        sqlite3_bind_int(stmt, 2, before);
        sqlite3_bind_int(stmt, 3, count);
    } else {
        sqlite3_bind_int(stmt, 1, friendId);
        sqlite3_bind_int(stmt, 2, count);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool original = sqlite3_column_int(stmt, 2) != 0;
        
        if (firstMessageId == 0) {
            firstMessageId = id;
        }
        
        messages.push_back(std::make_tuple(QString::fromUtf8(message), original));
    }

    sqlite3_finalize(stmt);
    
    emit outputMessageQuery(messages, firstMessageId);
}
