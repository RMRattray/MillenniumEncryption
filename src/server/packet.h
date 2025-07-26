#ifndef MLNM_PACKET_H
#define MLNM_PACKET_H

#define PACKET_BUFFER_SIZE 256

#include <string>

enum PacketToServerType {
    CREATE_ACCOUNT = 1,
    LOGIN,
    FRIEND_REQUEST_SEND,
    FRIEND_REQUEST_ACKNOWLEDGE,
    MESSAGE_SEND,
    MESSAGE_ACKNOWLEDGE
};

enum PacketFromServerType {
    ACCOUNT_RESULT = 1,
    LOGIN_RESULT,
    FRIEND_STATUS_UPDATE,
    FRIEND_REQUEST_FORWARD,
    FRIEND_REQUEST_RESPONSE,
    MESSAGE_FORWARD,
    MESSAGE_RESPONSE
};

class packetToServer {
    public:
    virtual int write_to_packet(unsigned char * buffer) { return 0; }
    ~packetToServer() { }
    protected:
    PacketToServerType type;
};

class packetFromServer {
    public:
    virtual int write_to_packet(unsigned char * buffer) { return 0; }
    ~packetFromServer() { }
    protected:
    PacketFromServerType type;
};

class createAccountRequest : public packetToServer {
    public:
    createAccountRequest(std::string user_name, std::string password);
    createAccountRequest(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string user_name;
    std::string password;
};

class createAccountResponse : public packetFromServer {
    public:
    createAccountResponse(bool success, std::string reason);
    createAccountResponse(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    bool success;
    std::string reason;
};

#endif