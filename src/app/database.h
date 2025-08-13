#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <vector>
#include <string>
#include <sqlite3.h>

class ClientDatabaseManager : public QObject 
{
    Q_OBJECT

public:
    explicit ClientDatabaseManager(QObject *parent = nullptr);
    ~ClientDatabaseManager();

    bool hasFriend(QString name);

    // inserts a friend with the given name into the friends database
    void insertFriend(QString name);

    // inserts a message with the given text, originality, and attributed friend into the messages database
    void insertMessage(QString message, bool original, QString friend_name = "");

    // requests that a vector of the names of all currently known friends
    // be retrieved and sent by signal outputFriendList
    void queryFriends();

    // requests that the last *count* messages from the friend chosen
    // with ID less than before (or the last ten at all, if before is zero)
    // be retrieved and send by outputMessageQuery
    void queryFriendMessages(QString friend_name);
    void queryMessages(QString friend_name, int count = 10, int before = 0);
    
signals:
    void outputFriendList(std::vector<QString> friends);
    void outputMessageQuery(std::vector<std::tuple<QString, bool>> messages, int first_message_id);
    void reportIncomingMessage(QString message, bool original);
    void reportOutgoingMessage(QString message, QString recipient);
    void reportNewFriend(int id, QString name, int status);

private:
    sqlite3 * database;
    QString queried_friend = "";
};

#endif DATABASE_H