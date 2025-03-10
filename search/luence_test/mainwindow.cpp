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
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 路径选择部分
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathEdit = new QLineEdit(this);
    browseButton = new QPushButton("浏览", this);
    indexButton = new QPushButton("创建索引", this);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    pathLayout->addWidget(indexButton);

    // 搜索部分
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("输入搜索内容...");

    // 结果列表
    resultList = new QListWidget(this);

    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(searchEdit);
    mainLayout->addWidget(resultList);

    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectDirectory);
    connect(indexButton, &QPushButton::clicked, this, &MainWindow::createIndex);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::performSearch);

    resize(800, 600);
}

void MainWindow::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择目录",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
        fileList.clear();

        // 只扫描第一级目录
        QDir selectedDir(dir);
        QFileInfoList entries = selectedDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo &fileInfo : entries) {
            if (fileInfo.isFile()) {
                fileList.append(fileInfo.absoluteFilePath());
            }
        }

        resultList->clear();
        resultList->addItems(fileList);
        qDebug() << "找到" << fileList.size() << "个文件";
    }
}

void MainWindow::createIndex()
{
    if (fileList.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有可索引的文件！");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    searchEngine.createIndex(fileList);
    QApplication::restoreOverrideCursor();

    QMessageBox::information(this, "完成", "索引创建完成！");
}

void MainWindow::performSearch(const QString &text)
{
    resultList->clear();

    if (text.isEmpty()) {
        resultList->addItems(fileList);
        return;
    }

    if (!searchEngine.hasIndex()) {
        QMessageBox::warning(this, "警告", "请先创建索引！");
        return;
    }

    QStringList results = searchEngine.search(text);
    resultList->addItems(results);
    qDebug() << "搜索到" << results.size() << "个结果";
}
