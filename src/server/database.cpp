#include <database.h>
#include <string>
#include <vector>
#include <iostream>

MillenniumServerDatabaseManager::MillenniumServerDatabaseManager() {
    // Open database
    if (sqlite3_open("SCRAPE.db", &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char * zErrMsg;
    if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (user_name TEXT, pass_hash TEXT, hash_count INTEGER);", NULL, NULL, &zErrMsg) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS friends (left TEXT, right TEXT);", NULL, NULL, &zErrMsg) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS requests (recipient TEXT, sender TEXT, hidden INTEGER);", NULL, NULL, &zErrMsg))
    {
        fprintf(stderr, "Error occurred: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

MillenniumServerDatabaseManager::~MillenniumServerDatabaseManager() {
    sqlite3_close(db);
}

int MillenniumServerDatabaseManager::checkIfUsernameExists(std::string user_name) {

    const char * query_tmplt = "SELECT COUNT(*) FROM users WHERE user_name = ?;";
    sqlite3_stmt * query_stmt;
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to check if user name is in use:" << sqlite3_errmsg(db);
        return -1;
    }

    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(query_stmt) != SQLITE_ROW) {
        std::cout << "Failed to step in statement to check if user name is in use" << sqlite3_errmsg(db);
        return -1;
    }
    
    int taken = sqlite3_column_int(query_stmt, 0);
    sqlite3_finalize(query_stmt);
    return taken;
}

// Retrieve stored password hash, also hash count if requested
// Will return only the first password hash for a user with that name - should higher-level logic
// ensure no duplicate usernames?
std::string MillenniumServerDatabaseManager::getPassHashFromUsername(std::string user_name, int* hash_count) {
    
    const char * query_tmplt = "SELECT pass_hash, hash_count FROM users WHERE user_name = ?;";
    sqlite3_stmt * query_stmt;
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepared statement to check for password corresponding to user name: " << sqlite3_errmsg(db);
        return "";
    }

    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(query_stmt);
    if (rc != SQLITE_ROW) {
        if (rc == SQLITE_DONE) sqlite3_finalize(query_stmt); // There were no rows in the table with this user-name
        return "";
    }

    std::string pass_hash = std::string((const char *)sqlite3_column_text(query_stmt, 0));
    if (hash_count) *hash_count = sqlite3_column_int(query_stmt, 1);

    sqlite3_finalize(query_stmt);

    return pass_hash;
}

// Insert a new user into the table, with the given user name, password hash, and count.  Return -1 if fail, 0 if success
int MillenniumServerDatabaseManager::insertNewUser(std::string user_name, std::string pass_hash, int hash_count) {
    
    const char * query_tmplt = "INSERT INTO users (user_name, pass_hash, hash_count) VALUES ( ?1 , ?2, ?3 );";
    sqlite3_stmt * query_stmt;
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to add new user";
        return -1;
    }

    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, pass_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(query_stmt, 3, hash_count);
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to step statement to add user name" << sqlite3_errmsg(db);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    return 0;
}

std::vector<std::string> MillenniumServerDatabaseManager::getFriendList(std::string user_name) {
    const char * query_tmplt = "SELECT right FROM friends WHERE left = ?;";
    sqlite3_stmt * query_stmt;
    
    std::vector<std::string> friends;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to get friend list: " << sqlite3_errmsg(db) << std::endl;
        return friends;
    }
    
    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(query_stmt) == SQLITE_ROW) {
        const char * friend_name = (const char *)sqlite3_column_text(query_stmt, 0);
        if (friend_name) {
            friends.push_back(std::string(friend_name));
        }
    }
    
    sqlite3_finalize(query_stmt);
    return friends;
}

std::vector<std::string> MillenniumServerDatabaseManager::getIncomingFriendRequests(std::string user_name) {
    const char * query_tmplt = "SELECT sender FROM requests WHERE recipient = ? AND hidden = FALSE;";
    sqlite3_stmt * query_stmt;

    std::vector<std::string> requests;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to get incoming friend requests: " << sqlite3_errmsg(db) << std::endl;
        return requests;
    }
    
    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(query_stmt) == SQLITE_ROW) {
        const char * sender = (const char *)sqlite3_column_text(query_stmt, 0);
        if (sender) {
            requests.push_back(std::string(sender));
        }
    }
    
    sqlite3_finalize(query_stmt);
    return requests;
}

std::vector<std::string> MillenniumServerDatabaseManager::getOutgoingFriendRequests(std::string user_name) {
    const char * query_tmplt = "SELECT recipient FROM requests WHERE sender = ?;";
    sqlite3_stmt * query_stmt;

    std::vector<std::string> requests;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to get outgoing friend requests: " << sqlite3_errmsg(db) << std::endl;
        return requests;
    }
    
    sqlite3_bind_text(query_stmt, 1, user_name.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(query_stmt) == SQLITE_ROW) {
        const char * recipient = (const char *)sqlite3_column_text(query_stmt, 0);
        if (recipient) {
            requests.push_back(std::string(recipient));
        }
    }
    
    sqlite3_finalize(query_stmt);
    return requests;
}

int MillenniumServerDatabaseManager::checkIfFriendRequest(std::string sender, std::string recipient) {
    const char * query_tmplt = "SELECT COUNT(*) FROM requests WHERE sender = ? AND recipient = ?;";
    sqlite3_stmt * query_stmt;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to check if friend request exists: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(query_stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(query_stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(query_stmt, 0);
    }
    
    sqlite3_finalize(query_stmt);
    return count;
}

int MillenniumServerDatabaseManager::checkIfFriend(std::string sender, std::string recipient) {
    const char * query_tmplt = "SELECT COUNT(*) FROM friends WHERE left = ? AND right = ?;";
    sqlite3_stmt * query_stmt;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to check if friend exists: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(query_stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(query_stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(query_stmt, 0);
    }
    
    sqlite3_finalize(query_stmt);
    return count;
}

// Functions below this point return -1 in case of SQLite3 error; 0 in case of no error
int MillenniumServerDatabaseManager::insertFriendRequest(std::string sender, std::string recipient) {
    const char * query_tmplt = "INSERT INTO requests (recipient, sender, hidden) VALUES (?, ?, 0);";
    sqlite3_stmt * query_stmt;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to insert friend request: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(query_stmt, 1, recipient.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, sender.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to insert friend request: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(query_stmt);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    return 0;
}

int MillenniumServerDatabaseManager::hideFriendRequest(std::string sender, std::string recipient) {
    const char * query_tmplt = "UPDATE requests SET hidden = 1 WHERE sender = ? AND recipient = ?;";
    sqlite3_stmt * query_stmt;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to hide friend request: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(query_stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to hide friend request: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(query_stmt);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    return 0;
}

int MillenniumServerDatabaseManager::removeFriendRequest(std::string sender, std::string recipient) {
    const char * query_tmplt = "DELETE FROM requests WHERE sender = ? AND recipient = ?;";
    sqlite3_stmt * query_stmt;
    
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to remove friend request: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(query_stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to remove friend request: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(query_stmt);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    return 0;
}

int MillenniumServerDatabaseManager::insertFriend(std::string sender, std::string recipient) {
    // Insert bidirectional friendship (both directions)
    const char * query_tmplt = "INSERT INTO friends (left, right) VALUES (?, ?);";
    sqlite3_stmt * query_stmt;
    
    // Insert first direction: sender -> recipient
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to insert friend (first direction): " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(query_stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to insert friend (first direction): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(query_stmt);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    
    // Insert second direction: recipient -> sender
    if (sqlite3_prepare_v2(db, query_tmplt, -1, &query_stmt, nullptr) != SQLITE_OK) {
        std::cout << "Failed to prepare statement to insert friend (second direction): " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(query_stmt, 1, recipient.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(query_stmt, 2, sender.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(query_stmt) != SQLITE_DONE) {
        std::cout << "Failed to insert friend (second direction): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(query_stmt);
        return -1;
    }
    
    sqlite3_finalize(query_stmt);
    return 0;
}
