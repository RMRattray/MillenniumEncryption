#include "codebookwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QMap>
#include <QDebug>

#include <memory>
#include <map>

CodebookWindow::CodebookWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    layout = new QVBoxLayout();
    central->setLayout(layout);
    setCentralWidget(central);

    // --- Codebook Name ---
    codebookNameBox = new QLineEdit();
    codebookNameBox->setPlaceholderText("New secret codebook");
    layout->addWidget(codebookNameBox);

    // --- Method Selection ---
    codebookMethodBox = new QComboBox();
    codebookMethodBox->addItem("Load from file");
    codebookMethodBox->addItem("Keyword");
    codebookMethodBox->addItem("Manual strings");
    layout->addWidget(codebookMethodBox);

    // --- File Select Zone ---
    fileSelectZone = new QWidget();
    {
        QHBoxLayout *h = new QHBoxLayout();
        fileSelectZone->setLayout(h);

        fileNameBox = new QLabel("No file selected");
        QPushButton * selectButton = new QPushButton("Select File");

        h->addWidget(fileNameBox);
        h->addWidget(selectButton);

        connect(selectButton, &QPushButton::clicked, this, [this]() {
            QString filename = QFileDialog::getOpenFileName(this, "Open Codebook File");
            if (!filename.isEmpty()) {
                fileNameBox->setText(filename);
            }
        });
    }

    // --- Keyword Zone ---
    keywordWriteZone = new QWidget();
    {
        QHBoxLayout *h = new QHBoxLayout();
        keywordWriteZone->setLayout(h);

        keywordBox = new QLineEdit();
        keywordBox->setPlaceholderText("Enter keyword");
        h->addWidget(keywordBox);
    }

    // --- Manual String Zone ---
    manualStringZone = new QWidget();
    {
        QVBoxLayout *v = new QVBoxLayout();
        manualStringZone->setLayout(v);

        QScrollArea *scroll = new QScrollArea();
        QWidget *scrollContent = new QWidget();
        QVBoxLayout *scrollLayout = new QVBoxLayout();

        // Create a QLineEdit for each printable ASCII character
        for (uint8_t c = 32; c < 127; c++) {
            QHBoxLayout *row = new QHBoxLayout();
            QLabel *label = new QLabel(QString("Character '%1'").arg(QChar(c)));
            QLineEdit *edit = new QLineEdit();
            edit->setObjectName(QString("char_%1").arg(c));
            row->addWidget(label);
            row->addWidget(edit);
            scrollLayout->addLayout(row);
        }

        scrollContent->setLayout(scrollLayout);
        scroll->setWidget(scrollContent);
        scroll->setWidgetResizable(true);

        v->addWidget(scroll);
    }

    // Add all zones but hide them initially
    layout->addWidget(fileSelectZone);
    layout->addWidget(keywordWriteZone);
    layout->addWidget(manualStringZone);

    // fileSelectZone->hide();
    keywordWriteZone->hide();
    manualStringZone->hide();

    // --- Final Create Button ---
    createButton = new QPushButton("Create Codebook");
    layout->addWidget(createButton);

    // --- Method Switching ---
    connect(codebookMethodBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        fileSelectZone->hide();
        keywordWriteZone->hide();
        manualStringZone->hide();

        if (index == 0) fileSelectZone->show();
        if (index == 1) keywordWriteZone->show();
        if (index == 2) manualStringZone->show();
    });

    // --- Create Codebook ---
    connect(createButton, &QPushButton::clicked, this, [this]() {
        int method = codebookMethodBox->currentIndex();
        std::shared_ptr<FullCodebook> result;

        if (method == 0) {
            // Load from file
            QString filename = fileNameBox->text();
            if (filename.isEmpty() || filename == "No file selected") {
                qWarning() << "No file selected";
                return;
            }

            result = std::make_shared<FullCodebook>("");
            if (!result->read_from_file(filename.toStdString())) {
                qWarning() << "Failed to read file";
                return;
            }
            
        }

        else if (method == 1) {
            // Keyword
            QString keyword = keywordBox->text();
            if (keyword.isEmpty()) {
                qWarning() << "Keyword empty";
                return;
            }

            result = std::make_shared<FullCodebook>(keyword.toStdString());
        }

        else if (method == 2) {
            // Manual strings
            std::map<uint8_t, readable_code> strings;

            for (uint8_t c = 32; c < 127; c++) {
                QLineEdit *edit = manualStringZone->findChild<QLineEdit*>(QString("char_%1").arg(c));
                if (!edit) continue;
                strings[c] = edit->text().toStdString();
            }

            result = std::make_shared<FullCodebook>("");

            if (!result->read_from_strings(strings)) {
                qWarning() << "Failed to read manual strings";
                return;
            }
        }

        emit reportNewCodebook(codebookNameBox->text(), result);
        close();
    });
}

CodebookWindow::~CodebookWindow() {}