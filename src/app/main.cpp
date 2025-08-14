#include <QApplication>
#include "clientsocket.h"
#include "database.h"
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

    MainWindow window(server_address);

    window.show();

    int result = app.exec();
    
    return result;
}