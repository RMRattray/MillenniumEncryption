#include <sqlite3.h>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

class MillenniumServer{
    public:
    MillenniumServer();
    ~MillenniumServer();
    void spin();
    private:
    void handleClient(SOCKET, std::string);
    sqlite3 *db;
    const static int PORT = 1999;
    bool serverRunning = true;
    std::vector<std::thread> clientThreads;
    SOCKET serverSocket;
    sockaddr_in service;
    std::map<std::string, SOCKET> clientSockets;
    std::mutex clientMutex;
    std::map<std::string, std::string> clientIPs;
};