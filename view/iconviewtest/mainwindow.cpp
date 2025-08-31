#include "mainwindow.h"
#include "fileviewmodel.h"
#include "fileviewdelegate.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QListView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    populateTestData();
}

MainWindow::~MainWindow() 
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create and setup list view
    listView = new QListView(this);
    listView->setViewMode(QListView::IconMode);
    listView->setResizeMode(QListView::Adjust);
    listView->setMovement(QListView::Static);
    listView->setFlow(QListView::LeftToRight);
    listView->setWrapping(true);
    listView->setSpacing(5);
    listView->setUniformItemSizes(false);
    // Remove fixed grid size to allow flexible group header sizing
    // listView->setGridSize(QSize(90, 100));
    listView->setLayoutMode(QListView::Batched);
    listView->setStyleSheet("QListView { background-color: white; border: none; }");
    
    // Create model and delegate
    model = new FileViewModel(this);
    delegate = new FileViewDelegate(this);
    
    listView->setModel(model);
    listView->setItemDelegate(delegate);
    
    mainLayout->addWidget(listView);
    
    // Set window properties
    setWindowTitle("Icon View Test - File Browser");
    resize(800, 600);
}

void MainWindow::populateTestData()
{
    model->populateTestData();
}
