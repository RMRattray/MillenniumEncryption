#ifndef MLNM_PACKET_H
#define MLNM_PACKET_H

#define PACKET_BUFFER_SIZE 256

#include <string>

enum PacketToServer {
    CREATE_ACCOUNT = 1,
    LOGIN,
    FRIEND_REQUEST_SEND,
    FRIEND_REQUEST_ACKNOWLEDGE,
    MESSAGE_SEND,
    MESSAGE_ACKNOWLEDGE
};

enum PacketFromServer {
    ACCOUNT_RESULT = 1,
    LOGIN_RESULT,
    FRIEND_STATUS_UPDATE,
    FRIEND_REQUEST_FORWARD,
    FRIEND_REQUEST_RESPONSE,
    MESSAGE_FORWARD,
    MESSAGE_RESPONSE
};

class createAccountRequest {
    public:
    createAccountRequest(std::string user_name, std::string password);
    createAccountRequest(unsigned char * buffer);
    void write_to_packet(unsigned char * buffer);
    private:
    std::string user_name;
    std::string password;
};

class createAccountResponse {
    public:
    bool success;
    std::string reason;
};

#endif