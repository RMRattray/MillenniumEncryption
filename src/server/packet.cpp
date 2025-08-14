#include <algorithm>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "packet.h"
#include <iostream>

createAccountRequest::createAccountRequest(std::string name, std::string pwd) {
    if (name.size() + pwd.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    user_name = name;
    password = pwd;
    type = PacketToServerType::CREATE_ACCOUNT;
}

createAccountRequest::createAccountRequest(unsigned char * buffer) {
    type = PacketToServerType::CREATE_ACCOUNT;
    if (*buffer != type) throw std::runtime_error("Attempting to create account from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    user_name = std::string((char *)buffer + 1);
    if (user_name.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("User name is too long");
    password = std::string((char *)buffer + 2 + user_name.size());
}

int createAccountRequest::write_to_packet(unsigned char * buffer) {
    if (user_name.size() + password.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    *buffer = type;
    user_name.copy((char *)buffer + 1, user_name.size());
    *(buffer + 1 + user_name.size()) = 0;
    password.copy((char *)buffer + 2 + user_name.size(), password.size());
    *(buffer + 2 + user_name.size() + password.size()) = 0;
    return 0;
}

// LoginRequest implementations
loginRequest::loginRequest(std::string uname, std::string pwd) {
    if (uname.size() + pwd.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    username = uname;
    password = pwd;
    type = PacketToServerType::LOGIN_REQUEST;
}

loginRequest::loginRequest(unsigned char * buffer) {
    type = PacketToServerType::LOGIN_REQUEST;
    if (*buffer != type) throw std::runtime_error("Attempting to read login request from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    username = std::string((char *)buffer + 1);
    if (username.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("Username is too long");
    password = std::string((char *)buffer + 2 + username.size());
}

int loginRequest::write_to_packet(unsigned char * buffer) {
    if (username.size() + password.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username or password is too long");
    *buffer = type;
    username.copy((char *)buffer + 1, username.size());
    *(buffer + 1 + username.size()) = 0;
    password.copy((char *)buffer + 2 + username.size(), password.size());
    *(buffer + 2 + username.size() + password.size()) = 0;
    return 0;
}

// FriendRequestSend implementations
friendRequestSend::friendRequestSend(std::string target) {
    if (target.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Target name is too long");
    target_name = target;
    type = PacketToServerType::FRIEND_REQUEST_SEND;
}

friendRequestSend::friendRequestSend(unsigned char * buffer) {
    type = PacketToServerType::FRIEND_REQUEST_SEND;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request send from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    target_name = std::string((char *)buffer + 1);
}

int friendRequestSend::write_to_packet(unsigned char * buffer) {
    if (target_name.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Target name is too long");
    *buffer = type;
    target_name.copy((char *)buffer + 1, target_name.size());
    *(buffer + 1 + target_name.size()) = 0;
    return 0;
}

// FriendRequestAcknowledge implementations
friendRequestAcknowledge::friendRequestAcknowledge(std::string to_, std::string from_, FriendRequestResponse r_) {
    to = to_;
    from = from_;
    response = r_;
    type = PacketToServerType::FRIEND_REQUEST_ACKNOWLEDGE;
}

friendRequestAcknowledge::friendRequestAcknowledge(unsigned char * buffer) {
    type = PacketToServerType::FRIEND_REQUEST_ACKNOWLEDGE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request acknowledge from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    to = std::string((char *)buffer + 1);
    if (to.size() > PACKET_BUFFER_SIZE - 6) throw std::runtime_error("To username is too long");
    from = std::string((char *)buffer + 2 + to.size());
    if (from.size() > PACKET_BUFFER_SIZE - 6 - to.size()) throw std::runtime_error("From username is too long");
    response = static_cast<FriendRequestResponse>(*(buffer + 3 + to.size() + from.size()));
}

int friendRequestAcknowledge::write_to_packet(unsigned char * buffer) {
    if (to.size() + from.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("To or from username is too long");
    *buffer = type;
    to.copy((char *)buffer + 1, to.size());
    *(buffer + 1 + to.size()) = 0;
    from.copy((char *)buffer + 2 + to.size(), from.size());
    *(buffer + 2 + to.size() + from.size()) = 0;
    *(buffer + 3 + to.size() + from.size()) = static_cast<unsigned char>(response);
    return 0;
}

// MessageSend implementations
messageSend::messageSend(std::string msg, std::string recip) {
    message = msg;
    recipient = recip;
    type = PacketToServerType::MESSAGE_SEND;
    bytes_remaining = msg.size();
}

messageSend::messageSend(unsigned char * buffer) {
    type = PacketToServerType::MESSAGE_SEND;
    if (*buffer != type) throw std::runtime_error("Attempting to read message send from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    
    bytes_remaining = *(uint32_t *)(buffer + 4);
    recipient = std::string((char *)buffer + 8);
    uint32_t readable = std::min((uint32_t)(PACKET_BUFFER_SIZE - 9 - recipient.size()), bytes_remaining);
    message = std::string((char *)buffer + 9 + recipient.size(), readable);
    bytes_remaining -= readable;
}

int messageSend::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    *((uint32_t *)(buffer + 4)) = bytes_remaining;

    uint32_t writeable;
    if (bytes_remaining == message.size()) {
        recipient.copy((char *)buffer + 8, recipient.size());
        *(buffer + 8 + recipient.size()) = 0;
        writeable = PACKET_BUFFER_SIZE - 9 - recipient.size();
    }
    else writeable = PACKET_BUFFER_SIZE - 8;

    if (writeable > bytes_remaining) writeable = bytes_remaining;
    message.substr(message.size() - bytes_remaining, writeable).copy((char *)buffer + PACKET_BUFFER_SIZE - writeable, writeable);
    bytes_remaining -= writeable;
    
    return bytes_remaining;
}

int messageSend::read_from_packet(unsigned char * buffer) {
    if (*buffer != type) throw std::runtime_error("Attempting to read message send from wrong sort of packet");
    if (bytes_remaining != *(uint32_t *)(buffer + 4)) throw std::runtime_error("Packet length inconsistent");
    uint32_t readable = std::min((uint32_t)(PACKET_BUFFER_SIZE - 8), bytes_remaining);
    message += std::string((char *)buffer + 8, readable);
    bytes_remaining -= readable;

    return bytes_remaining;
}

createAccountResponse::createAccountResponse(bool s, std::string r) {
    if (reason.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Reason is too long");
    success = s;
    reason = r;
    type = PacketFromServerType::ACCOUNT_RESULT;
}

createAccountResponse::createAccountResponse(unsigned char * buffer) {
    type = PacketFromServerType::ACCOUNT_RESULT;
    if (*buffer != type) throw std::runtime_error("Attempting to read account response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    success = (bool)(*(buffer + 1));
    reason = std::string((char *)buffer + 2);

}

int createAccountResponse::write_to_packet(unsigned char * buffer) {
    if (reason.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Reason is too long");
    *buffer = type;
    *(buffer + 1) = success;
    reason.copy((char *)buffer + 2, reason.size());
    *(buffer + 2 + reason.size()) = 0;
    return 0;
}

// LoginResult implementations
loginResult::loginResult(bool s, std::string r) {
    if (r.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Reason is too long");
    success = s;
    reason = r;
    type = PacketFromServerType::LOGIN_RESULT;
}

loginResult::loginResult(unsigned char * buffer) {
    type = PacketFromServerType::LOGIN_RESULT;
    if (*buffer != type) throw std::runtime_error("Attempting to read login result from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    success = (bool)(*(buffer + 1));
    reason = std::string((char *)buffer + 2);
}

int loginResult::write_to_packet(unsigned char * buffer) {
    if (reason.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Reason is too long");
    *buffer = type;
    *(buffer + 1) = success;
    reason.copy((char *)buffer + 2, reason.size());
    *(buffer + 2 + reason.size()) = 0;
    return 0;
}

// FriendStatusUpdate implementations
friendStatusUpdate::friendStatusUpdate(std::string uname, FriendStatus stat) {
    if (uname.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username is too long");
    username = uname;
    status = stat;
    type = PacketFromServerType::FRIEND_STATUS_UPDATE;
}

friendStatusUpdate::friendStatusUpdate(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_STATUS_UPDATE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend status update from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    username = std::string((char *)buffer + 1);
    if (username.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("Username is too long");
    status = static_cast<FriendStatus>(*(buffer + 2 + username.size()));
}

int friendStatusUpdate::write_to_packet(unsigned char * buffer) {
    if (username.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Username is too long");
    *buffer = type;
    username.copy((char *)buffer + 1, username.size());
    *(buffer + 1 + username.size()) = 0;
    *(buffer + 2 + username.size()) = static_cast<unsigned char>(status);
    return 0;
}

// FriendRequestForward implementations
friendRequestForward::friendRequestForward(std::string to_, std::string from_) {
    if (to_.size() + from_.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("To or from username is too long");
    to = to_;
    from = from_;
    type = PacketFromServerType::FRIEND_REQUEST_FORWARD;
}

friendRequestForward::friendRequestForward(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_FORWARD;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request forward from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    to = std::string((char *)buffer + 1);
    if (to.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("To username is too long");
    from = std::string((char *)buffer + 2 + to.size());
}

int friendRequestForward::write_to_packet(unsigned char * buffer) {
    if (to.size() + from.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("To or from username is too long");
    *buffer = type;
    to.copy((char *)buffer + 1, to.size());
    *(buffer + 1 + to.size()) = 0;
    from.copy((char *)buffer + 2 + to.size(), from.size());
    *(buffer + 2 + to.size() + from.size()) = 0;
    return 0;
}

// FriendRequestResponse implementations
friendRequestResponse::friendRequestResponse(std::string to_, std::string from_, FriendRequestResponse r_) {
    type = PacketFromServerType::FRIEND_REQUEST_RESPONSE;
    to = to_;
    from = from_;
    response = r_;
}

friendRequestResponse::friendRequestResponse(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_RESPONSE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    to = std::string((char *)buffer + 1);
    if (to.size() > PACKET_BUFFER_SIZE - 6) throw std::runtime_error("To username is too long");
    from = std::string((char *)buffer + 2 + to.size());
    if (from.size() > PACKET_BUFFER_SIZE - 6 - to.size()) throw std::runtime_error("From username is too long");
    response = static_cast<FriendRequestResponse>(*(buffer + 3 + to.size() + from.size()));
}

int friendRequestResponse::write_to_packet(unsigned char * buffer) {
    if (to.size() + from.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("To or from username is too long");
    *buffer = type;
    to.copy((char *)buffer + 1, to.size());
    *(buffer + 1 + to.size()) = 0;
    from.copy((char *)buffer + 2 + to.size(), from.size());
    *(buffer + 2 + to.size() + from.size()) = 0;
    *(buffer + 3 + to.size() + from.size()) = static_cast<unsigned char>(response);
    return 0;
}

// MessageForward implementations
messageForward::messageForward(std::string msg, std::string send) {
    message = msg;
    sender = send;
    type = PacketFromServerType::MESSAGE_FORWARD;
    bytes_remaining = msg.size();
}

messageForward::messageForward(unsigned char * buffer) {
    type = PacketFromServerType::MESSAGE_FORWARD;
    if (*buffer != type) throw std::runtime_error("Attempting to read message forward from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    
    bytes_remaining = *(uint32_t *)(buffer + 4);
    sender = std::string((char *)buffer + 8);
    uint32_t readable = std::min((uint32_t)(PACKET_BUFFER_SIZE - 9 - sender.size()), bytes_remaining);
    message = std::string((char *)buffer + 9 + sender.size(), readable);
    bytes_remaining -= readable;
}

int messageForward::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    *(uint32_t *)(buffer + 4) = bytes_remaining;

    uint32_t writeable;
    if (bytes_remaining == message.size()) {
        sender.copy((char *)buffer + 8, sender.size());
        *(buffer + 8 + sender.size()) = 0;
        writeable = PACKET_BUFFER_SIZE - 9 - sender.size();
    }
    else writeable = PACKET_BUFFER_SIZE - 8;

    if (writeable > message.size()) writeable = message.size();
    message.substr(message.size() - bytes_remaining, writeable).copy((char *)buffer + 9, writeable);
    bytes_remaining -= writeable;

    return bytes_remaining;
}

int messageForward::read_from_packet(unsigned char * buffer) {
    if (*buffer  != type) throw std::runtime_error("Attempting to read message forward from wrong type of packet");
    if (bytes_remaining != *(uint32_t *)(buffer + 4)) throw std::runtime_error("Packet length inconsistent");
    uint32_t readable = std::min((uint32_t)(PACKET_BUFFER_SIZE - 8), bytes_remaining);
    message += std::string((char *)buffer + 8, readable);
    bytes_remaining -= readable;

    return bytes_remaining;
}

// MessageResponse implementations
messageResponse::messageResponse(bool sent_) {
    sent = sent_;
    type = PacketFromServerType::MESSAGE_RESPONSE;
}

messageResponse::messageResponse(unsigned char * buffer) {
    type = PacketFromServerType::MESSAGE_RESPONSE;
    if (*buffer != type) throw std::runtime_error("Attempting to read message response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    sent = (bool)(*(buffer + 1));
}

int messageResponse::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    *(buffer + 1) = sent;
    return 0;
}

