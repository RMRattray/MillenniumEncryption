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
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS friends (left TEXT, right TEXT);", NULL, NULL, &zErrMsg) ||
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS requests (recipient TEXT, sender TEXT);", NULL, NULL, &zErrMsg))
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

        // Add client to map, create new thread to handle the client
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            clientSockets[clientIPStr] = clientSocket;
            socketMutexes[clientIPStr] = std::make_shared<std::mutex>();
            clientThreads.emplace_back(&MillenniumServer::handleClient, this, clientSocket, clientIPStr);
        }

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

int passwordCallback(void *data, int argc, char **argv, char **azColName) {
    if (argc > 0 && argv[0]) {
        *(std::string*)data = std::string(argv[0]);
    }
    return 0;
}

int gatherStringsCallback(void *data, int argc, char **argv, char **azColName) {
    if (argc > 0 && argv[0]) {
        ((std::vector<std::string>*)data)->push_back(std::string(argv[0]));
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
    int taken, userExists, targetExists;
    std::vector<std::string> names;
    bool recipientOnline;

    packetToServer * req;
    packetFromServer * resp;

    std::string connectedUser = "";

    while (serverRunning) {
        // Receive data from the client
        int rbyteCount;
        {
            std::lock_guard<std::mutex> myLock(*(socketMutexes[clientIP]));
            rbyteCount = recv(clientSocket, (char *)receiveBuffer, sizeof(receiveBuffer) - 1, 0);
        }
        if (rbyteCount <= 0) {
            if (rbyteCount == 0) {
                std::cout << "Client " << clientIP << " disconnected gracefully" << std::endl;
            } else {
                std::cout << "Error receiving from client " << clientIP << ": " << WSAGetLastError() << std::endl;
            }
            break;
        }

        resp = NULL;
        req = NULL;
        createAccountRequest * car = NULL;
        loginRequest * lr = NULL;
        friendRequestSend * frs = NULL;
        friendRequestAcknowledge * fra = NULL;
        messageSend * ms = NULL;

        std::shared_ptr<friendRequestForward> frf;
        std::shared_ptr<friendStatusUpdate> fsu;
        std::shared_ptr<friendRequestResponse> frr;
        std::shared_ptr<messageForward> mfr;

        // Action depends on the first byte
        switch (receiveBuffer[0]) {
            case PacketToServerType::CREATE_ACCOUNT:
                std::cout << "Received a packet requesting the creation of an account ";
                req = new createAccountRequest(receiveBuffer);
                car = dynamic_cast<createAccountRequest *>(req);
                std::cout << "with the user name:  '" << car->user_name << "' and the password:  '" << car->password << "'.\n";
                // (db, "CREATE TABLE IF NOT EXISTS users (user_name TEXT, pass_hash TEXT, hash_count INTEGER);", NULL, NULL, &zErrMsg)
                db_statement = "SELECT COUNT (*) FROM users WHERE user_name = '" + car->user_name + "';"; // Danger - user name better not contain apostrophes
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
                        db_statement = "INSERT INTO users (user_name, pass_hash, hash_count) VALUES ('" + car->user_name + "','" + car->password + "', 0)";
                        if (sqlite3_exec(db, db_statement.c_str(), NULL, NULL, &zErrMsg) != SQLITE_OK) {
                            std::cout << "Error in SQLite:  " << std::string(zErrMsg);
                            sqlite3_free(zErrMsg);
                            resp = new createAccountResponse(false, "Internal database issue");
                        }
                        else {
                            resp = new createAccountResponse(true, "Database worked and didn't contain username");
                            connectedUser = car->user_name;
                        }
                    }
                }
            break;
            
            case PacketToServerType::LOGIN_REQUEST:
                std::cout << "Received a login request packet ";
                req = new loginRequest(receiveBuffer);
                lr = dynamic_cast<loginRequest *>(req);
                std::cout << "with username: '" << lr->username << "' and password: '" << lr->password << "'.\n";
                
                // Check if username exists in users table
                db_statement = "SELECT COUNT(*) FROM users WHERE user_name = '" + lr->username + "';";
                if (sqlite3_exec(db, db_statement.c_str(), countCallback, &userExists, &zErrMsg) != SQLITE_OK) {
                    std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                    sqlite3_free(zErrMsg);
                    resp = new loginResult(false, "Internal database issue");
                }
                else {
                    if (userExists == 0) {
                        resp = new loginResult(false, "User does not exist");
                    }
                    else {
                        // Get the stored password for this user
                        db_statement = "SELECT pass_hash FROM users WHERE user_name = '" + lr->username + "';";
                        std::string storedPassword;
                        if (sqlite3_exec(db, db_statement.c_str(), passwordCallback, &storedPassword, &zErrMsg) != SQLITE_OK) {
                            std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                            sqlite3_free(zErrMsg);
                            resp = new loginResult(false, "Internal database issue");
                        }
                        else {
                            // TODO:  implement password hashing
                            if (lr->password == storedPassword) {
                                resp = new loginResult(true, "Login successful");
                                connectedUser = lr->username;
                                // Add this username and IP to clientIPs
                                {
                                    std::lock_guard<std::mutex> lock(clientMutex);
                                    clientIPs[lr->username] = clientIP;
                                }

                                // If login is successful, give them information

                                // get incoming friend requests
                                names.clear();
                                db_statement = "SELECT sender FROM requests WHERE recipient = '" + connectedUser + "';";
                                if (sqlite3_exec(db, db_statement.c_str(), gatherStringsCallback, &names, &zErrMsg) != SQLITE_OK) {
                                    std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                                    sqlite3_free(zErrMsg);
                                } 
                                for (auto &name : names) {
                                    frf = std::make_shared<friendRequestForward>(connectedUser, name);
                                    sendOutPacket(connectedUser, frf);
                                }

                                // get pending friend requests
                                names.clear();
                                db_statement = "SELECT recipient FROM requests WHERE sender = '" + connectedUser + "';";
                                if (sqlite3_exec(db, db_statement.c_str(), gatherStringsCallback, &names, &zErrMsg) != SQLITE_OK) {
                                    std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                                    sqlite3_free(zErrMsg);
                                } 
                                for (auto &name : names) {
                                    frr = std::make_shared<friendRequestResponse>(name, connectedUser, FriendRequestResponse::PENDING);
                                    sendOutPacket(connectedUser, frr);
                                }

                                // get friend statuses and send friend statuses
                                names.clear();
                                db_statement = "SELECT right FROM friends WHERE left = '" + connectedUser + "';";
                                if (sqlite3_exec(db, db_statement.c_str(), gatherStringsCallback, &names, &zErrMsg) != SQLITE_OK) {
                                    std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                                    sqlite3_free(zErrMsg);
                                }
                                for (auto &name : names) {
                                    std::cout << "Checking status of: " << name << std::endl;
                                    std::unique_lock<std::mutex> lock(clientMutex);
                                    // bool has_name = clientIPs.contains(name);
                                    bool has_name = (clientIPs.find(name) != clientIPs.end());
                                    lock.unlock();
                                    if (has_name) {
                                        fsu = std::make_shared<friendStatusUpdate>(name, FriendStatus::ONLINE);
                                        sendOutPacket(connectedUser, fsu);
                                        fsu = std::make_shared<friendStatusUpdate>(connectedUser, FriendStatus::ONLINE);
                                        sendOutPacket(name, fsu);
                                    }
                                    else {
                                        fsu = std::make_shared<friendStatusUpdate>(name, FriendStatus::OFFLINE);
                                        sendOutPacket(connectedUser, fsu);
                                    }
                                }
                            }
                            else {
                                resp = new loginResult(false, "Invalid password");
                            }
                        }
                    }
                }
            break;
            
            case PacketToServerType::FRIEND_REQUEST_SEND:
                std::cout << "Received a friend request send packet ";
                req = new friendRequestSend(receiveBuffer);
                frs = dynamic_cast<friendRequestSend *>(req);
                std::cout << "from user " << connectedUser << " targeting user: '" << frs->target_name << "'.\n";
                
                // Check if the target username exists in users table
                db_statement = "SELECT COUNT(*) FROM users WHERE user_name = '" + frs->target_name + "';";
                if (sqlite3_exec(db, db_statement.c_str(), countCallback, &targetExists, &zErrMsg) != SQLITE_OK) {
                    std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                    sqlite3_free(zErrMsg);
                    resp = new friendRequestResponse(frs->target_name, connectedUser, FriendRequestResponse::DATABASE_ERROR);
                }
                else {
                    if (targetExists == 0) {
                        std::cout << "That target does not exist.\n";
                        resp = new friendRequestResponse(frs->target_name, connectedUser, FriendRequestResponse::DOES_NOT_EXIST);
                    }
                    else {
                        // Target user exists, insert the request into the requests database
                        db_statement = "INSERT INTO requests (recipient, sender) VALUES ('" + frs->target_name + "', '" + connectedUser + "');";
                        if (sqlite3_exec(db, db_statement.c_str(), NULL, NULL, &zErrMsg) != SQLITE_OK) {
                            std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                            sqlite3_free(zErrMsg);
                            resp = new friendRequestResponse(frs->target_name, connectedUser, FriendRequestResponse::DATABASE_ERROR);
                        }
                        else {
                            resp = new friendRequestResponse(frs->target_name, connectedUser, FriendRequestResponse::PENDING);

                            // Also, if that user is online, send them the request now
                            frf = std::make_shared<friendRequestForward>(frs->target_name, connectedUser);
                            sendOutPacket(frs->target_name, frf);
                        }
                    }
                }
            break;
            
            case PacketToServerType::FRIEND_REQUEST_ACKNOWLEDGE:
                std::cout << "Received a friend request acknowledge packet ";
                req = new friendRequestAcknowledge(receiveBuffer);
                fra = dynamic_cast<friendRequestAcknowledge *>(req);
                std::cout << "with response: " << static_cast<int>(fra->response) << ".\n";
                
                switch (fra->response) {
                    case FriendRequestResponse::ACCEPT:
                    db_statement = "INSERT INTO friends (left, right) VALUES ('" + connectedUser + "', '" + fra->from + "'); "
                                + "INSERT INTO friends (left, right) VALUES ('" + fra->from + "', '" + connectedUser + "')";
                    std::cout << "It's an accepted request - should be running the query: " << db_statement << ";\n";
                    if (sqlite3_exec(db, db_statement.c_str(), NULL, NULL, &zErrMsg) != SQLITE_OK) { 
                        std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                        sqlite3_free(zErrMsg);
                    }

                    // announce the status of this new friend
                    {
                        std::cout << "Checking status of: " << fra->from << std::endl;
                        std::unique_lock<std::mutex> lock(clientMutex);
                        // bool has_name = clientIPs.contains(name);
                        bool has_name = (clientIPs.find(fra->from) != clientIPs.end());
                        lock.unlock();
                        if (has_name) {
                            fsu = std::make_shared<friendStatusUpdate>(fra->from, FriendStatus::ONLINE);
                            sendOutPacket(connectedUser, fsu);
                        }
                        else {
                            fsu = std::make_shared<friendStatusUpdate>(fra->from, FriendStatus::OFFLINE);
                            sendOutPacket(connectedUser, fsu);
                        }
                    }
                    
                    case FriendRequestResponse::REJECT:
                    frr = std::make_shared<friendRequestResponse>(connectedUser, fra->from, fra->response);
                        std::cout << "Passing on that information to " << fra->to;
                    sendOutPacket(fra->to, frr);
                    case FriendRequestResponse::HIDE:
                        std::cout << "\tand should be removing it from the database" << std::endl;
                    db_statement = "DELETE FROM requests WHERE recipient = '" + connectedUser + "' AND sender = '" + fra->from + "';";
                    if (sqlite3_exec(db, db_statement.c_str(), NULL, NULL, &zErrMsg) != SQLITE_OK) { 
                        std::cout << "Error in SQLite: " << std::string(zErrMsg) << std::endl;
                        sqlite3_free(zErrMsg);
                    }
                    break;
                    default:
                    std::cout << "Which is invalid.\n";
                }
                
            break;
            
            case PacketToServerType::MESSAGE_SEND:
                std::cout << "Received a message send packet ";
                req = new messageSend(receiveBuffer);
                ms = dynamic_cast<messageSend *>(req);
                std::cout << "with message: '" << ms->message << "' to recipient: '" << ms->recipient << "'.\n";
                
                // Check if the recipient is online (in clientIPs)
                recipientOnline = false;
                {
                    std::lock_guard<std::mutex> lock(clientMutex);
                    recipientOnline = (clientIPs.find(ms->recipient) != clientIPs.end());
                }
                
                resp = new messageResponse(recipientOnline);
                if (recipientOnline) {
                    mfr = std::make_shared<messageForward>(ms->message, connectedUser);
                    sendOutPacket(ms->recipient, mfr);
                }
            break;
            
            default:
                std::cout << "Invalid packet received from IP " << clientIP << std::endl;
                goto close;
        }

        delete req;
        
        // receiveBuffer[rbyteCount] = '\0'; // Null terminate the received data
        // std::cout << "Received from " << clientIP << ": " << receiveBuffer << std::endl;
        
        // Send a response back to the client
        // std::string response = "Server received: " + std::string((char *)receiveBuffer);

        if (resp) {
            std::cout << "To the connected user, sending a packet of type ";
            std::lock_guard<std::mutex> myLock(*(socketMutexes[clientIP]));

            switch (resp->type) {
                case (PacketFromServerType::ACCOUNT_RESULT):
                    std::cout << "account result"; break;
                case (PacketFromServerType::LOGIN_RESULT):
                    std::cout << "login result"; break;
                case (PacketFromServerType::FRIEND_STATUS_UPDATE):
                    std::cout << "friend status update"; break;
                case (PacketFromServerType::FRIEND_REQUEST_FORWARD):
                    std::cout << "friend request forward"; break;
                case (PacketFromServerType::FRIEND_REQUEST_RESPONSE):
                    std::cout << "friend request response"; break;
                case (PacketFromServerType::MESSAGE_FORWARD):
                    std::cout << "message forward"; break;
                case (PacketFromServerType::MESSAGE_RESPONSE):
                    std::cout << "message response"; break;
                default:
                    std::cout << "invalid";
            }

            std::cout << "\n";


            resp->write_to_packet(sendBuffer); // all response packets are short
            int sbyteCount = send(clientSocket, (char *)sendBuffer, PACKET_BUFFER_SIZE, 0);
            if (sbyteCount == SOCKET_ERROR) {
                std::cout << "Error sending to client " << clientIP << ": " << WSAGetLastError() << std::endl;
                delete resp;
                goto close;
            }

            delete resp;
            resp = NULL;
        }
    }

    close:
    
    // Clean up client connection
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        clientSockets.erase(clientIP);
        socketMutexes.erase(clientIP);
        clientIPs.erase(connectedUser);
    }
    closesocket(clientSocket);
    std::cout << "Client " << clientIP << " connection closed" << std::endl;
}

void MillenniumServer::sendOutPacket(std::string to, std::shared_ptr<packetFromServer> f) {
    std::cout << "Sending a packet to " << to << std::endl;
    std::unique_lock infoLock(clientMutex);
    if (clientIPs.find(to) == clientIPs.end()) {
        std::cout << "That user is not online.\n";
        return;
    }
    std::string destIPStr = clientIPs[to];
    std::unique_lock socketLock(*(socketMutexes[destIPStr]));
    SOCKET destSocket = clientSockets[destIPStr];
    infoLock.unlock();

    unsigned char packet[PACKET_BUFFER_SIZE + 1];
    packet[PACKET_BUFFER_SIZE] = 0;

    int sbyteCount;

    while (f->write_to_packet(packet)) {
        sbyteCount = send(destSocket, (char *)packet, PACKET_BUFFER_SIZE, 0);
        if (sbyteCount == SOCKET_ERROR) {
            std::cout << "Error sending friend status to client " << destIPStr << ": " << WSAGetLastError() << std::endl;
        }
    }
    sbyteCount = send(destSocket, (char *)packet, PACKET_BUFFER_SIZE, 0);
    if (sbyteCount == SOCKET_ERROR) {
        std::cout << "Error sending friend status to client " << destIPStr << ": " << WSAGetLastError() << std::endl;
    }
}