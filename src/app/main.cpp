#include <QApplication>
#include "mainwindow.h"
#include "packet.h"
#include <sqlite3.h>
#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Create SQLite database
    sqlite3 *db;
    int rc = sqlite3_open("PIT.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // Create friends table if it doesn't exist
    const char *create_friends_table = R"(
        CREATE TABLE IF NOT EXISTS friends (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            friend_name TEXT NOT NULL,
            status INTEGER NOT NULL
        )
    )";
    
    rc = sqlite3_exec(db, create_friends_table, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating friends table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    // Create messages table if it doesn't exist
    const char *create_messages_table = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            message TEXT NOT NULL,
            friend_id INTEGER NOT NULL,
            original BOOLEAN NOT NULL,
            FOREIGN KEY (friend_id) REFERENCES friends (id)
        )
    )";
    
    rc = sqlite3_exec(db, create_messages_table, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating messages table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    MainWindow window(db);
    window.show();

    int result = app.exec();
    
    // Close database
    sqlite3_close(db);
    
    return result;
}