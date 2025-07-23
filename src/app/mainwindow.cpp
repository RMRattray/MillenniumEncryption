#include "mainwindow.h"
#include "login.h"
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showLoginWidget();
}

void MainWindow::showLoginWidget()
{
    loginWidget = new LoginWidget(this);
    setCentralWidget(loginWidget);
    // connect(loginButton, &QPushButton::clicked, this, [this]() {
    //     if (usernameEdit->text() == "name" && passwordEdit->text() == "word") {
    //         showMainCentralWidget();
    //     } else {
    //         loginButton->setText("Try again");
    //     }
    // });
}

void MainWindow::showMainCentralWidget()
{
    mainCentralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainCentralWidget);
    // Left frame
    leftFrame = new QFrame(mainCentralWidget);
    leftFrame->setFrameShape(QFrame::StyledPanel);
    leftFrame->setFixedWidth(200);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftEmptyFrame = new QFrame(leftFrame);
    leftEmptyFrame->setFrameShape(QFrame::NoFrame);
    leftLayout->addWidget(leftEmptyFrame);
    leftLayout->addSpacing(10);
    codebookLabel = new QLabel("Codebook: NONE", leftFrame);
    codebookLabel->setFixedHeight(30);
    leftLayout->addWidget(codebookLabel);
    viewCodebooksButton = new QPushButton("View codebooks", leftFrame);
    viewCodebooksButton->setFixedHeight(30);
    leftLayout->addWidget(viewCodebooksButton);
    leftLayout->setStretch(0, 1);
    leftLayout->setStretch(1, 0);
    leftLayout->setStretch(2, 0);
    leftLayout->setStretch(3, 0);
    // Right frame
    rightFrame = new QFrame(mainCentralWidget);
    rightFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightEmptyFrame = new QFrame(rightFrame);
    rightEmptyFrame->setFrameShape(QFrame::NoFrame);
    rightLayout->addWidget(rightEmptyFrame);
    QHBoxLayout *bottomRow = new QHBoxLayout();
    rightTextBox = new QLineEdit(rightFrame);
    bottomRow->addWidget(rightTextBox);
    sendButton = new QPushButton(rightFrame);
    sendButton->setFixedWidth(50);
    sendButton->setIcon(QIcon::fromTheme("send"));
    bottomRow->addWidget(sendButton);
    rightLayout->addLayout(bottomRow);
    rightLayout->setStretch(0, 1);
    rightLayout->setStretch(1, 0);
    mainLayout->addWidget(leftFrame);
    mainLayout->addWidget(rightFrame);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    setCentralWidget(mainCentralWidget);
}

MainWindow::~MainWindow()
{
    // Qt will delete child widgets automatically
}