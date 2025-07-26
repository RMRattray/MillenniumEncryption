#include <string>
#include <stdexcept>
#include "packet.h"

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
friendRequestAcknowledge::friendRequestAcknowledge(FriendRequestResponse resp) {
    response = resp;
    type = PacketToServerType::FRIEND_REQUEST_ACKNOWLEDGE;
}

friendRequestAcknowledge::friendRequestAcknowledge(unsigned char * buffer) {
    type = PacketToServerType::FRIEND_REQUEST_ACKNOWLEDGE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request acknowledge from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    response = static_cast<FriendRequestResponse>(*(buffer + 1));
}

int friendRequestAcknowledge::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    *(buffer + 1) = static_cast<unsigned char>(response);
    return 0;
}

// MessageSend implementations
messageSend::messageSend(std::string msg, std::string recip) {
    if (msg.size() + recip.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Message or recipient is too long");
    message = msg;
    recipient = recip;
    type = PacketToServerType::MESSAGE_SEND;
}

messageSend::messageSend(unsigned char * buffer) {
    type = PacketToServerType::MESSAGE_SEND;
    if (*buffer != type) throw std::runtime_error("Attempting to read message send from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    message = std::string((char *)buffer + 1);
    if (message.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("Message is too long");
    recipient = std::string((char *)buffer + 2 + message.size());
}

int messageSend::write_to_packet(unsigned char * buffer) {
    if (message.size() + recipient.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Message or recipient is too long");
    *buffer = type;
    message.copy((char *)buffer + 1, message.size());
    *(buffer + 1 + message.size()) = 0;
    recipient.copy((char *)buffer + 2 + message.size(), recipient.size());
    *(buffer + 2 + message.size() + recipient.size()) = 0;
    return 0;
}

// MessageAcknowledge implementations
messageAcknowledge::messageAcknowledge(std::string recip) {
    if (recip.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Recipient is too long");
    recipient = recip;
    type = PacketToServerType::MESSAGE_ACKNOWLEDGE;
}

messageAcknowledge::messageAcknowledge(unsigned char * buffer) {
    type = PacketToServerType::MESSAGE_ACKNOWLEDGE;
    if (*buffer != type) throw std::runtime_error("Attempting to read message acknowledge from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    recipient = std::string((char *)buffer + 1);
}

int messageAcknowledge::write_to_packet(unsigned char * buffer) {
    if (recipient.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Recipient is too long");
    *buffer = type;
    recipient.copy((char *)buffer + 1, recipient.size());
    *(buffer + 1 + recipient.size()) = 0;
    return 0;
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
friendRequestForward::friendRequestForward(std::string uname) {
    if (uname.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Username is too long");
    username = uname;
    type = PacketFromServerType::FRIEND_REQUEST_FORWARD;
}

friendRequestForward::friendRequestForward(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_FORWARD;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request forward from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    username = std::string((char *)buffer + 1);
}

int friendRequestForward::write_to_packet(unsigned char * buffer) {
    if (username.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Username is too long");
    *buffer = type;
    username.copy((char *)buffer + 1, username.size());
    *(buffer + 1 + username.size()) = 0;
    return 0;
}

// FriendRequestResponse implementations
friendRequestResponse::friendRequestResponse() {
    type = PacketFromServerType::FRIEND_REQUEST_RESPONSE;
}

friendRequestResponse::friendRequestResponse(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_RESPONSE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
}

int friendRequestResponse::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    return 0;
}

// FriendRequestAcknowledgeForward implementations
friendRequestAcknowledgeForward::friendRequestAcknowledgeForward(FriendRequestResponse resp) {
    response = resp;
    type = PacketFromServerType::FRIEND_REQUEST_ACKNOWLEDGE_FORWARD;
}

friendRequestAcknowledgeForward::friendRequestAcknowledgeForward(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_ACKNOWLEDGE_FORWARD;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request acknowledge forward from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    response = static_cast<FriendRequestResponse>(*(buffer + 1));
}

int friendRequestAcknowledgeForward::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    *(buffer + 1) = static_cast<unsigned char>(response);
    return 0;
}

// FriendRequestAcknowledgeResponse implementations
friendRequestAcknowledgeResponse::friendRequestAcknowledgeResponse() {
    type = PacketFromServerType::FRIEND_REQUEST_ACKNOWLEDGE_RESPONSE;
}

friendRequestAcknowledgeResponse::friendRequestAcknowledgeResponse(unsigned char * buffer) {
    type = PacketFromServerType::FRIEND_REQUEST_ACKNOWLEDGE_RESPONSE;
    if (*buffer != type) throw std::runtime_error("Attempting to read friend request acknowledge response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
}

int friendRequestAcknowledgeResponse::write_to_packet(unsigned char * buffer) {
    *buffer = type;
    return 0;
}

// MessageForward implementations
messageForward::messageForward(std::string msg, std::string send) {
    if (msg.size() + send.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Message or sender is too long");
    message = msg;
    sender = send;
    type = PacketFromServerType::MESSAGE_FORWARD;
}

messageForward::messageForward(unsigned char * buffer) {
    type = PacketFromServerType::MESSAGE_FORWARD;
    if (*buffer != type) throw std::runtime_error("Attempting to read message forward from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    message = std::string((char *)buffer + 1);
    if (message.size() > PACKET_BUFFER_SIZE - 4) throw std::runtime_error("Message is too long");
    sender = std::string((char *)buffer + 2 + message.size());
}

int messageForward::write_to_packet(unsigned char * buffer) {
    if (message.size() + sender.size() > PACKET_BUFFER_SIZE - 3) throw std::runtime_error("Message or sender is too long");
    *buffer = type;
    message.copy((char *)buffer + 1, message.size());
    *(buffer + 1 + message.size()) = 0;
    sender.copy((char *)buffer + 2 + message.size(), sender.size());
    *(buffer + 2 + message.size() + sender.size()) = 0;
    return 0;
}

// MessageResponse implementations
messageResponse::messageResponse(std::string send) {
    if (send.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Sender is too long");
    sender = send;
    type = PacketFromServerType::MESSAGE_RESPONSE;
}

messageResponse::messageResponse(unsigned char * buffer) {
    type = PacketFromServerType::MESSAGE_RESPONSE;
    if (*buffer != type) throw std::runtime_error("Attempting to read message response from wrong sort of packet");
    if (buffer[PACKET_BUFFER_SIZE] != 0) throw std::runtime_error("Buffer not safely terminated");
    sender = std::string((char *)buffer + 1);
}

int messageResponse::write_to_packet(unsigned char * buffer) {
    if (sender.size() > PACKET_BUFFER_SIZE - 2) throw std::runtime_error("Sender is too long");
    *buffer = type;
    sender.copy((char *)buffer + 1, sender.size());
    *(buffer + 1 + sender.size()) = 0;
    return 0;
}

