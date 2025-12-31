#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

class MillenniumServerDatabaseManager {
    public:
    MillenniumServerDatabaseManager();
    ~MillenniumServerDatabaseManager();
    
    int checkIfUsernameExists(std::string user_name);
    std::string getPassHashFromUsername(std::string user_name, int* hash_count);
    int insertNewUser(std::string user_name, std::string pass_hash, int hash_count);

    std::vector<std::string> getFriendList(std::string user_name);
    std::vector<std::string> getIncomingFriendRequests(std::string user_name);
    std::vector<std::string> getOutgoingFriendRequests(std::string user_name);

    bool checkIfFriendRequest(std::string sender, std::string recipient);
    bool checkIfFriend(std::string sender, std::string recipient);

    int insertFriendRequest(std::string sender, std::string recipient);
    int hideFriendRequest(std::string sender, std::string recipient);
    int removeFriendRequest(std::string sender, std::string recipient);
    int insertFriend(std::string sender, std::string recipient);

    private:
    sqlite3 *db;
};

#endif