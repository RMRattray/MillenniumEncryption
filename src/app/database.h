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

    // inserts a friend with the given name into the friends database
    void insertFriend(QString name);

    // inserts a message with the given text, originality, and attributed friend into the messages database
    void insertMessage(QString message, bool original, QString friend);

    // requests that a vector of the names of all currently known friends
    // be retrieved and sent by signal outputFriendList
    void queryFriends();

    // requests that the last *count* messages from the friend chosen
    // with ID less than before (or the last ten at all, if before is zero)
    // be retrieved and send by outputMessageQuery
    void queryMessages(QString friend, int count, int before);
    
signals:
    void outputFriendList(std::vector<QString> friends);
    void outputMessageQuery(std::vector<std::tuple<QString, bool>> messages, int first_message_id);

private:
    sqlite3 * database;
};