#ifndef CODEBOOK_WINDOW_H
#define CODEBOOK_WINDOW_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QMap>
#include <QDebug>
#include <QMainWindow>

#include <memory>
#include <map>

#include "../crypt/codebook.h"

class CodebookWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CodebookWindow(QWidget *parent = nullptr);
    ~CodebookWindow();

signals:
    void reportNewCodebook(QString bookName, std::shared_ptr<FullCodebook> book);

private:
    QVBoxLayout *layout;

    QLineEdit *codebookNameBox;
    QComboBox *codebookMethodBox;

    QWidget *fileSelectZone;
        QLabel *fileNameBox;
    QWidget *keywordWriteZone;
        QLineEdit *keywordBox;
    QWidget *manualStringZone;

    QPushButton *createButton;
};

#endif