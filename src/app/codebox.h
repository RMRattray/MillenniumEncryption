#ifndef CODEBOX_H
#define CODEBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QComboBox>
#include <QTcpSocket>
#include "../crypt/codebook.h"
#include <memory>

class CodeBox : public QWidget
{
    Q_OBJECT

public:
    explicit CodeBox(QWidget *parent = nullptr);
    ~CodeBox();
    
signals:
    void requestMessageSend(QString recipient, QByteArray messageEncrypted);
    void reportDecryptedMessage(QString message, bool original, QString sender); 
    // Sender is last in above signal to link to ClientDatabaseManager::insertMessage, in which friend_name is null for outgoing messages

public slots:
    void encryptAndSendMessage(QString recipient, QString message);
    void decryptAndReceiveMessage(QString sender, QByteArray messageEncrypted);

private:
    QVBoxLayout *layout;
    QMap<QString, std::shared_ptr<FullCodebook>> codebooks;
    QPushButton *addCodebookPushButton;
    QPushButton *saveCodebookPushButton;
    std::shared_ptr<FullCodebook> current_codebook;
    QComboBox *codebookComboBox;

    void openCodebookWindow();
    void saveCurrentCodebook();

private slots:
    void addNewCodebook(QString codebookName, std::shared_ptr<FullCodebook> codebook);
};

#endif // CODEBOX_H