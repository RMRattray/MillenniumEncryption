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

RequestsBox::RequestsBox(QWidget *parent)
    : QWidget(parent)
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

void RequestsBox::processFriendRequest(QString name) {
    addRequest(name, true);
}

void RequestsBox::processFriendResponse(QString name, int resp) {
    QMessageBox msgBox;
    switch (static_cast<FriendRequestResponse>(resp)) {
        case FriendRequestResponse::DOES_NOT_EXIST:
            msgBox.setText(QString::fromStdString("No user with that name."));
            msgBox.exec();
        break;
        case FriendRequestResponse::DATABASE_ERROR:
            msgBox.setText("A database error occurred while trying to contact the user.");
            msgBox.exec();
        break;
        case FriendRequestResponse::PENDING:
            addRequest(name, false);
        break;
        case FriendRequestResponse::ACCEPT:
            announceNewFriend(name);
        case FriendRequestResponse::REJECT:
            removeRequest(name);
            break;
        default:
            break;
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
        emit requestFriendResponse(username, static_cast<int>(FriendRequestResponse::ACCEPT));
        announceNewFriend(username);
        removeRequest(username);
    });
    
    connect(requestBox, &RequestBox::rejectRequest, this, [this](const QString &username) {
        emit requestFriendResponse(username, static_cast<int>(FriendRequestResponse::REJECT));
        removeRequest(username);
    });
    
    connect(requestBox, &RequestBox::hideRequest, this, [this](const QString &username) {
        emit requestFriendResponse(username, static_cast<int>(FriendRequestResponse::HIDE));
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

void RequestsBox::createFriendRequest()
{
    FriendRequestDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();

        // TODO:  Access database here in such a way as to prevent sending
        // friend requests to friends

        // std::string target_name = username.toStdString();
        // const char *sql_chk = "SELECT COUNT(*) FROM friends WHERE friend_name = ? ;";
        // sqlite3_stmt *stmt_chk;
        // int rc = sqlite3_prepare_v2(database, sql_chk, -1, &stmt_chk, nullptr);
        // if (rc != SQLITE_OK) {
        //     qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(database);
        //     return;
        // }
        // sqlite3_bind_text(stmt_chk, 1, target_name.c_str(), -1, SQLITE_STATIC);
        // rc = sqlite3_step(stmt_chk);
        // int ans = sqlite3_column_int(stmt_chk, 0);
        // sqlite3_finalize(stmt_chk);

        QMessageBox msgBox;
        // if (ans) {
        //     msgBox.setText("You are already friends with " + username);
        //     msgBox.exec();
        //     return;
        // }

        // else
        if (requestWidgets.contains(username)) {
            msgBox.setText(QString::fromStdString("You already have a request " + ( requestWidgets[username]->is_pending ? std::string("to ") : std::string("from ") )) + username + ".");
            msgBox.exec();
        }
        else {
            requestFriendRequest(username);
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
        is_pending = false;
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
        is_pending = true;
    }
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