#ifndef MLNM_PACKET_H
#define MLNM_PACKET_H

#define PACKET_BUFFER_SIZE 256

#include <string>

enum PacketToServerType {
    CREATE_ACCOUNT = 1,
    LOGIN_REQUEST,
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
    FRIEND_REQUEST_ACKNOWLEDGE_FORWARD,
    FRIEND_REQUEST_ACKNOWLEDGE_RESPONSE,
    MESSAGE_FORWARD,
    MESSAGE_RESPONSE
};

enum FriendRequestResponse {
    ACCEPT = 1,
    REJECT,
    HIDE
};

enum FriendStatus {
    OFFLINE = 1,
    ONLINE
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

// PacketToServer derivatives
class createAccountRequest : public packetToServer {
    public:
    createAccountRequest(std::string user_name, std::string password);
    createAccountRequest(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string user_name;
    std::string password;
};

class loginRequest : public packetToServer {
    public:
    loginRequest(std::string username, std::string password);
    loginRequest(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string username;
    std::string password;
};

class friendRequestSend : public packetToServer {
    public:
    friendRequestSend(std::string target_name);
    friendRequestSend(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string target_name;
};

class friendRequestAcknowledge : public packetToServer {
    public:
    friendRequestAcknowledge(FriendRequestResponse response);
    friendRequestAcknowledge(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    FriendRequestResponse response;
};

class messageSend : public packetToServer {
    public:
    messageSend(std::string message, std::string recipient);
    messageSend(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string message;
    std::string recipient;
};

class messageAcknowledge : public packetToServer {
    public:
    messageAcknowledge(std::string recipient);
    messageAcknowledge(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string recipient;
};

// PacketFromServer derivatives
class createAccountResponse : public packetFromServer {
    public:
    createAccountResponse(bool success, std::string reason);
    createAccountResponse(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    bool success;
    std::string reason;
};

class loginResult : public packetFromServer {
    public:
    loginResult(bool success, std::string reason);
    loginResult(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    bool success;
    std::string reason;
};

class friendStatusUpdate : public packetFromServer {
    public:
    friendStatusUpdate(std::string username, FriendStatus status);
    friendStatusUpdate(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string username;
    FriendStatus status;
};

class friendRequestForward : public packetFromServer {
    public:
    friendRequestForward(std::string username);
    friendRequestForward(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string username;
};

class friendRequestResponse : public packetFromServer {
    public:
    friendRequestResponse();
    friendRequestResponse(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
};

class friendRequestAcknowledgeForward : public packetFromServer {
    public:
    friendRequestAcknowledgeForward(FriendRequestResponse response);
    friendRequestAcknowledgeForward(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    FriendRequestResponse response;
};

class friendRequestAcknowledgeResponse : public packetFromServer {
    public:
    friendRequestAcknowledgeResponse();
    friendRequestAcknowledgeResponse(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
};

class messageForward : public packetFromServer {
    public:
    messageForward(std::string message, std::string sender);
    messageForward(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string message;
    std::string sender;
};

class messageResponse : public packetFromServer {
    public:
    messageResponse(std::string sender);
    messageResponse(unsigned char * buffer);
    int write_to_packet(unsigned char * buffer);
    std::string sender;
};

#endif