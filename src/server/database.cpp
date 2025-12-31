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

std::string MillenniumServerDatabaseManager::getPassHashFromUsername(std::string user_name, int* hash_count) {
    
    const char * query_tmplt = "SELECT pass_hash FROM users WHERE user_name = ?;";
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

// std::vector<std::string> MillenniumServerDatabaseManager::getFriendList(std::string user_name);
// std::vector<std::string> MillenniumServerDatabaseManager::getIncomingFriendRequests(std::string user_name);
// std::vector<std::string> MillenniumServerDatabaseManager::getOutgoingFriendRequests(std::string user_name);

// bool MillenniumServerDatabaseManager::checkIfFriendRequest(std::string sender, std::string recipient);
// bool MillenniumServerDatabaseManager::checkIfFriend(std::string sender, std::string recipient);

// int MillenniumServerDatabaseManager::insertFriendRequest(std::string sender, std::string recipient);
// int MillenniumServerDatabaseManager::hideFriendRequest(std::string sender, std::string recipient);
// int MillenniumServerDatabaseManager::removeFriendRequest(std::string sender, std::string recipient);
// int MillenniumServerDatabaseManager::insertFriend(std::string sender, std::string recipient);