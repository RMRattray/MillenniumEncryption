#include <sqlite3.h>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "packet.h"

class MillenniumServer{

    public:
    MillenniumServer();
    ~MillenniumServer();
    void spin();

    private:
    void handleClient(SOCKET, std::string);
    void sendOutPacket(std::string to, std::shared_ptr<packetFromServer> f);

    sqlite3 *db;
    
    const static int PORT = 1999;
    bool serverRunning = true;
    SOCKET serverSocket;
    sockaddr_in service;
    
    std::mutex clientMutex; // Mutex for accessing the below
    std::vector<std::thread> clientThreads; // list of threads
    std::map<std::string, SOCKET> clientSockets; // list of sockets by IP address
    std::map<std::string, std::string> clientIPs; // list of IP addresses by user name
    std::map<std::string, std::shared_ptr<std::mutex>> socketMutexes; // mutexes for each socket, by IP address
};