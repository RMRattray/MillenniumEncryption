#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Login screen widgets
    QWidget *loginWidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;

    // Main central widget and its components
    QWidget *mainCentralWidget;
    QFrame *leftFrame;
    QFrame *rightFrame;
    QLabel *codebookLabel;
    QPushButton *viewCodebooksButton;
    QFrame *leftEmptyFrame;
    QFrame *rightEmptyFrame;
    QLineEdit *rightTextBox;
    QPushButton *sendButton;

    void showLoginWidget();
    void showMainCentralWidget();
};