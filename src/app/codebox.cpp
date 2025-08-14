#include "codebox.h"
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
    
    // Create button to add new codebook
    addCodebookPushButton = new QPushButton("Add Codebook", this);
    layout->addWidget(addCodebookPushButton);
    
    // Connect button to addNewCodebook function
    connect(addCodebookPushButton, &QPushButton::clicked, this, &CodeBox::addNewCodebook);
    
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

void CodeBox::encryptAndSendMessage(QString message, QString recipient)
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

void CodeBox::decryptAndReceiveMessage(QString message, QString sender)
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

void CodeBox::addNewCodebook()
{
    // Open file dialog to select codebook file
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Codebook File",
        "",
        "Codebook Files (*.cbk);;All Files (*)"
    );
    
    if (fileName.isEmpty()) {
        return; // User cancelled
    }
    
    // Extract just the filename without path for the map key
    QFileInfo fileInfo(fileName);
    QString codebookName = fileInfo.fileName();
    
    // Check if codebook with this name already exists
    if (codebooks.contains(codebookName)) {
        QMessageBox::warning(this, "Codebook Exists", 
                           "A codebook with this name already exists.");
        return;
    }
    
    // Create a new FullCodebook and try to read from file
    try {
        std::shared_ptr<FullCodebook> newCodebook = std::make_shared<FullCodebook>(""); // Empty keyword, will be loaded from file
        std::string filePath = fileName.toStdString();
        
        if (newCodebook->read_from_file(filePath)) {
            // Verify the codebook is valid
            if (newCodebook->verify()) {
                // Add to map
                codebooks[codebookName] = newCodebook;
                
                // Add to combo box
                codebookComboBox->addItem(codebookName);
                
                // If this is the first codebook, select it
                if (codebookComboBox->count() == 1) {
                    codebookComboBox->setCurrentText(codebookName);
                    current_codebook = codebooks[codebookName];
                }
                
                qDebug() << "Successfully loaded codebook:" << codebookName;
            } else {
                QMessageBox::warning(this, "Invalid Codebook", 
                                   "The selected file is not a valid codebook.");
            }
        } else {
            QMessageBox::warning(this, "File Error", 
                               "Failed to read the codebook file.");
        }
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", 
                            QString("Error loading codebook: %1").arg(e.what()));
    }
}
