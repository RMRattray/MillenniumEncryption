#include <QApplication>
#include "mainwindow.h"
#include "packet.h"
#include <sqlite3.h>
#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Get server address if one is present in command-line arguments
    QString server_address = "127.0.0.1";
    int i = 1;
    while (i < argc) {
        if (!strcmp(argv[i], "-ip")) {
            ++i;
            if (i == argc) {
                std::cerr << "No IP address after -ip flag\n";
                return 1;
            }
            server_address = QString(argv[i]);
        }
        ++i;
    }

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

    // Check if friends table is empty and insert sample data
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM friends", -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count == 0) {
                // Insert sample friends
                const char *insert_friends[] = {
                    "INSERT INTO friends (friend_name, status) VALUES ('Alice', 1)",
                    "INSERT INTO friends (friend_name, status) VALUES ('Bob', 2)",
                    "INSERT INTO friends (friend_name, status) VALUES ('Charlie', 1)",
                    "INSERT INTO friends (friend_name, status) VALUES ('Diana', 2)",
                    "INSERT INTO friends (friend_name, status) VALUES ('Eve', 1)"
                };
                
                for (const char *sql : insert_friends) {
                    rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cerr << "SQL error inserting friend: " << sqlite3_errmsg(db) << std::endl;
                    }
                }
                
                // Insert sample messages
                const char *insert_messages[] = {
                    "INSERT INTO messages (message, friend_id, original) VALUES ('Hey Alice!', 1, 1)",
                    "INSERT INTO messages (message, friend_id, original) VALUES ('How are you?', 1, 0)",
                    "INSERT INTO messages (message, friend_id, original) VALUES ('Hi Bob', 2, 1)",
                    "INSERT INTO messages (message, friend_id, original) VALUES ('Good morning Charlie', 3, 1)",
                    "INSERT INTO messages (message, friend_id, original) VALUES ('See you later!', 3, 0)"
                };
                
                for (const char *sql : insert_messages) {
                    rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cerr << "SQL error inserting message: " << sqlite3_errmsg(db) << std::endl;
                    }
                }
                
                std::cout << "Inserted sample data into database" << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);

    MainWindow window(db, server_address);
    window.show();

    int result = app.exec();
    
    // Close database
    sqlite3_close(db);
    
    return result;
}