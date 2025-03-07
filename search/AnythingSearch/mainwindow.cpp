#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupSearcher();
}

MainWindow::~MainWindow() { }

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 路径选择部分
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathEdit = new QLineEdit(this);
    browseButton = new QPushButton("浏览", this);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);

    // 搜索部分
    searchEdit = new QLineEdit(this);
    searchEdit->setEnabled(false);
    searchEdit->setPlaceholderText("输入搜索内容...");

    // 结果列表
    resultList = new QListWidget(this);

    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(searchEdit);
    mainLayout->addWidget(resultList);

    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectDirectory);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchEditChanged);
    connect(pathEdit, &QLineEdit::textChanged, this, &MainWindow::onPathEditChanged);

    resize(800, 600);
}

void MainWindow::setupSearcher()
{
    searcher = new AnythingSearcher(this);
}

void MainWindow::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择目录",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
        searchEdit->clear();
    }
}

void MainWindow::performSearch(const QString &text)
{
    if (text.isEmpty()) {
        resultList->clear();
        return;
    }

    QString searchPath = pathEdit->text();
    if (searchPath.isEmpty()) {
        QMessageBox::warning(this, "Warning!", "Not search path！");
        return;
    }

    qDebug() << "Search :" << text << "in" << searchPath;
    resultList->clear();

    // do anything search
    if (!searcher->requestSedarch(searchPath, text)) {
        QMessageBox::warning(this, "Warning!", "Anything bad！");
    }
}

void MainWindow::onSearchEditChanged(const QString &text)
{
    // TODO: perforcmance
    performSearch(text);
}

void MainWindow::onPathEditChanged(const QString &text)
{
    searchEdit->clear();
    searchEdit->setEnabled(!text.isEmpty());
}
