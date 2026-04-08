#include "codebox.h"
#include "codebookwindow.h"
#include "../server/packet.h"
#include "../crypt/encrypt.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <string>
#include <sstream>

CodeBox::CodeBox(QWidget *parent)
    : QWidget(parent), current_codebook(nullptr)
{
    layout = new QVBoxLayout(this);
    
    // Create combo box for codebook selection
    QHBoxLayout *comboLayout = new QHBoxLayout();
    QLabel *codebookLabel = new QLabel("Codebook:", this);
    codebookComboBox = new QComboBox(this);
    comboLayout->addWidget(codebookLabel);
    comboLayout->addWidget(codebookComboBox);
    layout->addLayout(comboLayout);
    
    // Create buttons to add, save new codebook
    addCodebookPushButton = new QPushButton("Add Codebook", this);
    layout->addWidget(addCodebookPushButton);
    saveCodebookPushButton = new QPushButton("Save Codebook To File", this);
    layout->addWidget(saveCodebookPushButton);
    
    // Connect buttons to functions
    connect(addCodebookPushButton, &QPushButton::clicked, this, &CodeBox::openCodebookWindow);
    connect(saveCodebookPushButton, &QPushButton::clicked, this, &CodeBox::saveCurrentCodebook);
    
    // Connect combo box selection change
    connect(codebookComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [this](const QString &text) {
                if (codebooks.contains(text)) {
                    current_codebook = codebooks[text];
                } else {
                    current_codebook = nullptr;
                }
            });
}

CodeBox::~CodeBox()
{
    // Qt will handle cleanup of child widgets
}

void CodeBox::encryptAndSendMessage(QString recipient, QString message)
{
    qDebug() << "Encrypting the message \"" << message << "\" for " << recipient;
    if (!current_codebook) {
        qDebug() << "No codebook selected for encryption";
        requestMessageSend(recipient, message);
        return;
    }
    
    // Convert QString to std::string for encryption
    std::string messageStr = message.toStdString();
    
    // Encrypt the message using the current codebook
    std::string encryptedMessage;
    
    std::istringstream messageStream(messageStr);
    std::ostringstream cipherStream(encryptedMessage);
    encrypt(messageStream, cipherStream, *current_codebook);
    
    requestMessageSend(recipient, QString::fromStdString(encryptedMessage));
}

void CodeBox::decryptAndReceiveMessage(QString sender, QString message)
{
    if (!current_codebook) {
        qDebug() << "No codebook selected for decryption";
        reportDecryptedMessage(message, false, sender);
        return;
    }
    
    // Convert QString to std::string for encryption
    std::string messageStr = message.toStdString();
    
    // Encrypt the message using the current codebook
    std::string decryptedMessage;
    
    std::istringstream cipherStream(messageStr);
    std::ostringstream plainStream(decryptedMessage);
    decrypt(cipherStream, plainStream, *current_codebook);
    
    reportDecryptedMessage(QString::fromStdString(decryptedMessage), false, sender);
}

void CodeBox::openCodebookWindow()
{
    CodebookWindow * cbw = new CodebookWindow(nullptr);
    connect(cbw, &CodebookWindow::reportNewCodebook, this, &CodeBox::addNewCodebook);
    cbw->show();
}

void CodeBox::addNewCodebook(QString codebookName, std::shared_ptr<FullCodebook> codebook)
{
    // Check if codebook with this name already exists
    if (codebooks.contains(codebookName)) {
        QMessageBox::warning(this, "Codebook Exists", 
                           "A codebook with this name already exists.");
        return;
    }
    
    // Add to map
    codebooks[codebookName] = codebook;
    
    // Add to combo box and select it
    codebookComboBox->addItem(codebookName);
    codebookComboBox->setCurrentText(codebookName);
    current_codebook = codebooks[codebookName];
                
    qDebug() << "Successfully loaded codebook:" << codebookName;
}

void CodeBox::saveCurrentCodebook() {
    if (codebookComboBox->count() > 0) {
        // File dialog to place codebook file
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Locate Codebook File",
            codebookComboBox->currentText() + ".cbk",
            "Codebook Files (*.cbk);;All Files (*)"
        );
        
        if (fileName.isEmpty()) {
            return; // User cancelled
        }

        current_codebook->write_to_file(fileName.toStdString());
    } else {
        QMessageBox::warning(this, "Error", "No codebook loaded");
    }
}
