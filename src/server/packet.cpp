#include <string>
#include <packet.h>

createAccountRequest::createAccountRequest(std::string user_name, std::string password) {
    if (this.user_name.size() + this.password.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    this.user_name = user_name;
    this.password = password;
}

createAccountRequest::createAccountRequest(unsigned char * buffer) {
    if (*buffer != PacketToServer::CREATE_ACCOUNT) throw std::runtime_error("Attempting to create account from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    this.user_name = std::string((char *)buffer + 1);
    if (this.user_name.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("User name is too long");
    this.password = std::string((char *)buffer + 2 + user_name.size());
}

void createAccountRequest::write_to_packet(unsigned char * buffer) {
    if (this.user_name.size() + this.password.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    *buffer = PacketToServer::CREATE_ACCOUNT;
    this.user_name.copy(buffer + 1);
    *(buffer + 1 + this.user_name.size()) = 0;
    this.password.copy(buffer + 2 + this.user_name.size());
}

