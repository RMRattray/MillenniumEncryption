#include <sqlite3.h>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <memory>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
    #define WSAGetLastError() errno
#endif

#include "packet.h"
#include "database.h"

class MillenniumServer{

    public:
    MillenniumServer();
    ~MillenniumServer();
    void spin();

    private:
    void handleClient(socket_t, std::string, long long int);
    void sendOutPacket(std::string to, std::shared_ptr<packetFromServer> f);

    MillenniumServerDatabaseManager dbm;
    
    const static int PORT = 1999;
    bool serverRunning = true;
    socket_t serverSocket;
    sockaddr_in service;
    
    std::mutex clientMutex; // Mutex for accessing the below
    std::vector<std::thread> clientThreads; // list of threads
    std::unordered_map<long long int, socket_t> clientSockets; // list of sockets by connection ID
    std::unordered_multimap<std::string, long long int> clientIDs; // list of IDs by user name
    std::unordered_map<long long int, std::shared_ptr<std::mutex>> socketMutexes; // mutexes for each socket, by ID
};