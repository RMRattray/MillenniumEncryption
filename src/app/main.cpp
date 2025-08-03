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

    // Set up database if it doesn't exist    
    if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS friends (id INTEGER PRIMARY KEY AUTOINCREMENT, friend_name TEXT NOT NULL, status INTEGER NOT NULL)", nullptr, nullptr, nullptr) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS messages (id INTEGER PRIMARY KEY AUTOINCREMENT, friend_id INTEGER NOT NULL, message TEXT NOT NULL, original BOOLEAN NOT NULL, FOREIGN KEY (friend_id) REFERENCES friends (id))", nullptr, nullptr, nullptr) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS requests (friend_name TEXT NOT NULL, status INTEGER NOT NULL)", nullptr, nullptr, nullptr)) {
        std::cerr << "SQL error creating tables: " << sqlite3_errmsg(db) << std::endl;
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