#include <iostream>
#include <sqlite3.h>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <packet.h>
#include <server.h>

MillenniumServer::MillenniumServer() {
    
    // Open database
    if (sqlite3_open("SCRAPE.db", &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char * zErrMsg;
    if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (user_name TEXT, pass_hash TEXT, hash_count INTEGER);", NULL, NULL, &zErrMsg) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS friends (ip_less TEXT, ip_more TEXT);", NULL, NULL, &zErrMsg))
    {
        fprintf(stderr, "Error occurred: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // Set up winsock
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr) {
        std::cout << "Winsock dll not found!\n";
        return;
    }

    // Set up server socket
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Bind socket to service
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(PORT);
    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Error listening on socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Millennium Encryption Server started on port " << PORT << std::endl;
    std::cout << "Waiting for client connections..." << std::endl;
}

void MillenniumServer::spin() {
    serverRunning = true;
    while (serverRunning) {
        SOCKET clientSocket = INVALID_SOCKET;
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        std::cout << "About to run the 'accept' function:\n";
        clientSocket = accept(serverSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrLen);
        std::cout << "I've run the 'accept' function\n";
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Get client IP address
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::string clientIPStr(clientIP);

        // Add client to map
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            clientSockets[clientIPStr] = clientSocket;
        }

        // Create a new thread to handle this client
        clientThreads.emplace_back(&MillenniumServer::handleClient, this, clientSocket, clientIPStr);

        std::cout << "Total connected clients: " << clientSockets.size() << std::endl;
    }
}

MillenniumServer::~MillenniumServer() {
    std::cout << "Shutting down server..." << std::endl;

    {
        std::lock_guard<std::mutex> lock(clientMutex);
        for (auto& client : clientSockets) {
            closesocket(client.second);
        }
        clientSockets.clear();
    }

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    sqlite3_close(db);

    std::cout << "Server shutdown complete" << std::endl;
    return;
}

int countCallback(void *data, int argc, char **argv, char **azColName) {
    if (argc > 0 && argv[0]) {
        *(int*)data = atoi(argv[0]);
    }
    return 0;
}

void MillenniumServer::handleClient(SOCKET clientSocket, std::string clientIP) {
    std::cout << "New client connected from: " << clientIP << std::endl;

    unsigned char receiveBuffer[PACKET_BUFFER_SIZE + 1];
    receiveBuffer[PACKET_BUFFER_SIZE] = 0;

    unsigned char sendBuffer[PACKET_BUFFER_SIZE + 1];
    sendBuffer[PACKET_BUFFER_SIZE] = 0;

    int db_response;
    std::string db_statement;
    char * zErrMsg;

    while (serverRunning) {
        // Receive data from the client
        int rbyteCount = recv(clientSocket, (char *)receiveBuffer, sizeof(receiveBuffer) - 1, 0);
        if (rbyteCount <= 0) {
            if (rbyteCount == 0) {
                std::cout << "Client " << clientIP << " disconnected gracefully" << std::endl;
            } else {
                std::cout << "Error receiving from client " << clientIP << ": " << WSAGetLastError() << std::endl;
            }
            break;
        }

        // Action depends on the first byte
        switch (receiveBuffer[0]) {
            case PacketToServer::CREATE_ACCOUNT:
                std::cout << "Received a packet requesting the creation of an account ";
                createAccountRequest * req;
                req = new createAccountRequest(receiveBuffer);
                std::cout << "with the user name:  '" << req->user_name << "' and the password:  '" << req->password << "'.\n";
                // (db, "CREATE TABLE IF NOT EXISTS users (user_name TEXT, pass_hash TEXT, hash_count INTEGER);", NULL, NULL, &zErrMsg)
                db_statement = "SELECT COUNT (*) FROM users WHERE user_name = '" + req->user_name + "';"; // Danger - user name better not contain apostrophes
                int taken;
                createAccountResponse * resp;
                if (sqlite3_exec(db, db_statement.c_str(), countCallback, &taken, &zErrMsg) != SQLITE_OK) {
                    std::cout << "Error in SQLite:  " << std::string(zErrMsg);
                    sqlite3_free(zErrMsg);
                    resp = new createAccountResponse(false, "Internal database issue");
                }
                else {
                    if (taken) {
                    std::cout << "User name was taken";
                        resp = new createAccountResponse(false, "User name was taken");
                    }
                    else { // TODO: actually hash the password
                        db_statement = "INSERT INTO users (user_name, pass_hash, hash_count) VALUES ('" + req->user_name + "','" + req->password + "', 0)";
                        if (sqlite3_exec(db, db_statement.c_str(), NULL, NULL, &zErrMsg) != SQLITE_OK) {
                            std::cout << "Error in SQLite:  " << std::string(zErrMsg);
                            sqlite3_free(zErrMsg);
                            resp = new createAccountResponse(false, "Internal database issue");
                        }
                        else {
                            resp = new createAccountResponse(true, "Database worked and didn't contain username");
                        }
                    }
                }
                resp->write_to_packet(sendBuffer);
                free(resp);
            break;
            default:
                std::cout << "Invalid packet received from IP " << clientIP << std::endl;
                goto close;
        }
        
        // receiveBuffer[rbyteCount] = '\0'; // Null terminate the received data
        // std::cout << "Received from " << clientIP << ": " << receiveBuffer << std::endl;
        
        // Send a response back to the client
        // std::string response = "Server received: " + std::string((char *)receiveBuffer);

        std::cout << "All good, about to send packet\n";

        int sbyteCount = send(clientSocket, (char *)sendBuffer, PACKET_BUFFER_SIZE, 0);
        if (sbyteCount == SOCKET_ERROR) {
            std::cout << "Error sending to client " << clientIP << ": " << WSAGetLastError() << std::endl;
            break;
        }
    }

    close:
    
    // Clean up client connection
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        clientSockets.erase(clientIP);
    }
    closesocket(clientSocket);
    std::cout << "Client " << clientIP << " connection closed" << std::endl;
}