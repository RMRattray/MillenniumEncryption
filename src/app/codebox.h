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
    explicit CodeBox(QTcpSocket *socket, QWidget *parent = nullptr);
    ~CodeBox();
    
    void encryptAndSendMessage(QString message, QString target);

private:
    QTcpSocket *sock;
    QVBoxLayout *layout;
    QMap<QString, std::shared_ptr<FullCodebook>> codebooks;
    QPushButton *addCodebookPushButton;
    std::shared_ptr<FullCodebook> current_codebook;
    QComboBox *codebookComboBox;

    void addNewCodebook();
};

#endif // CODEBOX_H