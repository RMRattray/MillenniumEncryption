#include <string>
#include <stdexcept>
#include "packet.h"

createAccountRequest::createAccountRequest(std::string name, std::string pwd) {
    if (name.size() + pwd.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    user_name = name;
    password = pwd;
}

createAccountRequest::createAccountRequest(unsigned char * buffer) {
    if (*buffer != PacketToServer::CREATE_ACCOUNT) throw std::runtime_error("Attempting to create account from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    user_name = std::string((char *)buffer + 1);
    if (user_name.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("User name is too long");
    password = std::string((char *)buffer + 2 + user_name.size());
}

void createAccountRequest::write_to_packet(unsigned char * buffer) {
    if (user_name.size() + password.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    *buffer = PacketToServer::CREATE_ACCOUNT;
    user_name.copy((char *)buffer + 1, user_name.size());
    *(buffer + 1 + user_name.size()) = 0;
    password.copy((char *)buffer + 2 + user_name.size(), password.size());
    *(buffer + 2 + user_name.size() + password.size()) = 0;
}

