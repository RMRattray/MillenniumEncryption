#include "requestsbox.h"
#include "packet.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>
#include <QTcpSocket>

RequestsBox::RequestsBox(sqlite3 *db, QTcpSocket *socket, QWidget *parent)
    : QWidget(parent), database(db), socket(socket)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Add "Create Friend Request" button at the bottom
    QPushButton *createRequestButton = new QPushButton("Create Friend Request", this);
    createRequestButton->setFixedHeight(30);
    layout->addWidget(createRequestButton);
    
    connect(createRequestButton, &QPushButton::clicked, this, &RequestsBox::createFriendRequest);
}

RequestsBox::~RequestsBox()
{
    // Qt will delete child widgets automatically
}

void RequestsBox::handlePacket(unsigned char *packet)
{
    switch (*packet) {
        case PacketFromServerType::FRIEND_REQUEST_FORWARD: {
            friendRequestForward forward(packet);
            QString from = QString::fromStdString(forward.from);
            addRequest(from, true); // Has buttons for Accept/Reject/Hide
            break;
        }
        case PacketFromServerType::FRIEND_REQUEST_RESPONSE: {
            friendRequestResponse response(packet);
            QMessageBox msgBox;
            switch (response.response) {
                case FriendRequestResponse::DOES_NOT_EXIST:
                    msgBox.setText(QString::fromStdString("No user named '" + response.to + "'."));
                    msgBox.exec();
                break;
                case FriendRequestResponse::DATABASE_ERROR:
                    msgBox.setText("A database error occurred while trying to contact the user.");
                    msgBox.exec();
                break;
                case FriendRequestResponse::PENDING:
                    addRequest(QString::fromStdString(response.to), false);
                break;
                case FriendRequestResponse::ACCEPT:
                case FriendRequestResponse::REJECT:
                    removeRequest(QString::fromStdString(response.to));
                    break;
            }
            break;
        }
    }
}

void RequestsBox::addRequest(const QString &from, bool hasButtons)
{
    if (requestWidgets.contains(from)) {
        return; // Already exists
    }
    
    RequestBox *requestBox = new RequestBox(from, hasButtons, this);
    requestWidgets[from] = requestBox;
    layout->insertWidget(layout->count() - 1, requestBox); // Insert before the button
    
    connect(requestBox, &RequestBox::acceptRequest, this, [this](const QString &username) {
        sendFriendRequestAcknowledge("", username, FriendRequestResponse::ACCEPT);
        // Add friend to database
        const char *sql = "INSERT INTO friends (friend_name, status) VALUES (?, ?)";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.toUtf8().constData(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, FriendStatus::ONLINE);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        removeRequest(username);
    });
    
    connect(requestBox, &RequestBox::rejectRequest, this, [this](const QString &username) {
        sendFriendRequestAcknowledge("", username, FriendRequestResponse::REJECT);
        removeRequest(username);
    });
    
    connect(requestBox, &RequestBox::hideRequest, this, [this](const QString &username) {
        sendFriendRequestAcknowledge("", username, FriendRequestResponse::HIDE);
        removeRequest(username);
    });
}

void RequestsBox::removeRequest(const QString &from)
{
    if (requestWidgets.contains(from)) {
        RequestBox *requestBox = requestWidgets[from];
        layout->removeWidget(requestBox);
        requestWidgets.remove(from);
        delete requestBox;
    }
}

void RequestsBox::sendFriendRequestAcknowledge(const QString &to, const QString &from, int response)
{
    friendRequestAcknowledge packet(to.toStdString(), from.toStdString(), static_cast<FriendRequestResponse>(response));
    unsigned char buffer[PACKET_BUFFER_SIZE + 1];
    buffer[PACKET_BUFFER_SIZE] = 0;
    packet.write_to_packet(buffer);
    
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(reinterpret_cast<const char*>(buffer), PACKET_BUFFER_SIZE);
    }
}

void RequestsBox::sendFriendRequestSend(const QString &target)
{
    friendRequestSend packet(target.toStdString());
    unsigned char buffer[PACKET_BUFFER_SIZE + 1];
    buffer[PACKET_BUFFER_SIZE] = 0;
    packet.write_to_packet(buffer);
    
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(reinterpret_cast<const char*>(buffer), PACKET_BUFFER_SIZE);
    }
}

void RequestsBox::createFriendRequest()
{
    FriendRequestDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();
        if (!username.isEmpty()) {
            sendFriendRequestSend(username);
        }
    }
}

RequestBox::RequestBox(const QString &username, bool hasButtons, QWidget *parent)
    : QWidget(parent), username(username)
{
    setFixedHeight(40);
    setStyleSheet("QWidget { background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 4px; }");
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    QLabel *nameLabel = new QLabel(username, this);
    nameLabel->setStyleSheet("font-weight: bold; color: #333;");
    layout->addWidget(nameLabel);
    
    layout->addStretch();
    
    if (hasButtons) {
        // Create Accept, Reject, Hide buttons
        acceptButton = new QPushButton("Accept", this);
        acceptButton->setFixedSize(60, 25);
        acceptButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 3px; }");
        layout->addWidget(acceptButton);
        
        rejectButton = new QPushButton("Reject", this);
        rejectButton->setFixedSize(60, 25);
        rejectButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 3px; }");
        layout->addWidget(rejectButton);
        
        hideButton = new QPushButton("Hide", this);
        hideButton->setFixedSize(60, 25);
        hideButton->setStyleSheet("QPushButton { background-color: #9E9E9E; color: white; border: none; border-radius: 3px; }");
        layout->addWidget(hideButton);
        
        connect(acceptButton, &QPushButton::clicked, this, &RequestBox::onAcceptClicked);
        connect(rejectButton, &QPushButton::clicked, this, &RequestBox::onRejectClicked);
        connect(hideButton, &QPushButton::clicked, this, &RequestBox::onHideClicked);
    } else {
        // Show "Pending" label
        statusLabel = new QLabel("Pending", this);
        statusLabel->setStyleSheet("color: #FF9800; font-size: 10px; font-weight: bold;");
        layout->addWidget(statusLabel);
    }
}

void RequestBox::handlePacket(unsigned char *packet)
{
    // This method is not used for RequestBox, but kept for consistency
    // Packet handling is done at the RequestsBox level
}

void RequestBox::onAcceptClicked()
{
    emit acceptRequest(username);
}

void RequestBox::onRejectClicked()
{
    emit rejectRequest(username);
}

void RequestBox::onHideClicked()
{
    emit hideRequest(username);
}

FriendRequestDialog::FriendRequestDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Create Friend Request");
    setFixedSize(300, 120);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *label = new QLabel("Enter username:", this);
    layout->addWidget(label);
    
    usernameEdit = new QLineEdit(this);
    layout->addWidget(usernameEdit);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *sendButton = new QPushButton("Send", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    
    buttonLayout->addWidget(sendButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    connect(sendButton, &QPushButton::clicked, this, &FriendRequestDialog::onSendClicked);
    connect(cancelButton, &QPushButton::clicked, this, &FriendRequestDialog::onCancelClicked);
    
    // Set focus to username edit
    usernameEdit->setFocus();
}

QString FriendRequestDialog::getUsername() const
{
    return username;
}

void FriendRequestDialog::onSendClicked()
{
    username = usernameEdit->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a username.");
        return;
    }
    accept();
}

void FriendRequestDialog::onCancelClicked()
{
    reject();
} 