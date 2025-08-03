#ifndef MESSAGESBOX_H
#define MESSAGESBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <sqlite3.h>

class MessageBox;

class MessagesBox : public QWidget
{
    Q_OBJECT

public:
    explicit MessagesBox(sqlite3 *db, QWidget *parent = nullptr);
    ~MessagesBox();

    void selectFriend(int friendId);

private:
    sqlite3 *database;
    QVBoxLayout *layout;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QLabel *noSelectionLabel;
    int currentFriendId;
    
    void loadMessages(int friendId);
    void clearMessages();
};

class MessageBox : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBox(const QString &message, bool isOriginal, QWidget *parent = nullptr);

private:
    QString messageText;
    bool isOriginalMessage;
    QLabel *messageLabel;
};

#endif // MESSAGESBOX_H 