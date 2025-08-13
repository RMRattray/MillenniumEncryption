#include "messagesbox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QDebug>
#include <QSpacerItem>
#include <QTimer>
#include <QScrollBar>
#include <vector>
#include <tuple>

MessagesBox::MessagesBox(QWidget *parent)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // Create scroll area for messages
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget for scroll area
    scrollContent = new QWidget(scrollArea);
    scrollContent->setLayout(new QVBoxLayout(scrollContent));
    scrollContent->layout()->setSpacing(5);
    scrollContent->layout()->setContentsMargins(5, 5, 5, 5);
    
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);
    
    // Create "no selection" label
    noSelectionLabel = new QLabel("Select a friend to view messages", this);
    noSelectionLabel->setAlignment(Qt::AlignCenter);
    noSelectionLabel->setStyleSheet("color: #666; font-size: 14px;");
    layout->addWidget(noSelectionLabel);
    
    // Initially show the no selection label
    scrollArea->hide();
    noSelectionLabel->show();
}

MessagesBox::~MessagesBox()
{
    // Qt will delete child widgets automatically
}

void MessagesBox::processMessages(std::vector<std::tuple<QString, bool>> messages, int id)
{
    clearMessages();

    currentMinMessageId = id;
    
    bool hasMessages = messages.size();
    
    if (hasMessages) {
        // Show scroll area and hide no selection label
        scrollArea->show();
        noSelectionLabel->hide();

        for (auto message : messages) {
            addMessage(std::get<0>(message), std::get<1>(message));
        }
        
        // Scroll to the bottom to show latest messages
        QTimer::singleShot(100, [this]() {
            QScrollBar *scrollBar = scrollArea->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        });
    } else {
        // Show "no messages" label
        QLabel *noMessagesLabel = new QLabel("No messages yet", scrollContent);
        noMessagesLabel->setAlignment(Qt::AlignCenter);
        noMessagesLabel->setStyleSheet("color: #666; font-size: 14px;");
        scrollContent->layout()->addWidget(noMessagesLabel);
        
        scrollArea->show();
        noSelectionLabel->hide();
    }
}

void MessagesBox::addMessage(QString message, bool original) {
    MessageBox *messageBox = new MessageBox(message, original, scrollContent);
    scrollContent->layout()->addWidget(messageBox);
}

void MessagesBox::clearMessages()
{
    // Remove all child widgets from scroll content
    QLayout *contentLayout = scrollContent->layout();
    QLayoutItem *item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

MessageBox::MessageBox(const QString &message, bool isOriginal, QWidget *parent)
    : QWidget(parent), messageText(message), isOriginalMessage(isOriginal)
{
    setFixedHeight(60);
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    // Create message label
    messageLabel = new QLabel(message, this);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    // Style based on whether it's original or received
    if (isOriginal) {
        // Original message (sent by user) - align to right, blue background
        layout->addStretch();
        layout->addWidget(messageLabel);
        setStyleSheet("QWidget { background-color: #007AFF; border-radius: 10px; }");
        messageLabel->setStyleSheet("color: white; font-size: 12px; padding: 5px;");
    } else {
        // Received message - align to left, gray background
        layout->addWidget(messageLabel);
        layout->addStretch();
        setStyleSheet("QWidget { background-color: #E5E5EA; border-radius: 10px; }");
        messageLabel->setStyleSheet("color: black; font-size: 12px; padding: 5px;");
    }
} 