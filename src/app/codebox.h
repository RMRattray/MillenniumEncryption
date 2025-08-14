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
    void requestMessageSend(QString recipient, QString message);
    void reportDecryptedMessage(QString message, bool original, QString sender);
public slots:
    void encryptAndSendMessage(QString message, QString recipient);
    void decryptAndReceiveMessage(QString message, QString sender);

private:
    QVBoxLayout *layout;
    QMap<QString, std::shared_ptr<FullCodebook>> codebooks;
    QPushButton *addCodebookPushButton;
    std::shared_ptr<FullCodebook> current_codebook;
    QComboBox *codebookComboBox;

    void addNewCodebook();
};

#endif // CODEBOX_H