#ifndef MESSAGESBOX_H
#define MESSAGESBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <sqlite3.h>
#include <vector>
#include <tuple>

class MessageBox;

class MessagesBox : public QWidget
{
    Q_OBJECT

public:
    explicit MessagesBox(QWidget *parent = nullptr);
    ~MessagesBox();

public slots:
    void processMessages(std::vector<std::tuple<QString, bool>> messages, int id);
    void addMessage(QString message, bool original);

private:
    QVBoxLayout *layout;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QLabel *noSelectionLabel;
    int currentMinMessageId;
    
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