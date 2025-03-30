#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class SearchWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchTypeChanged(int index);
    void onSearchModeChanged(int index);
    void setupSearchConnections();
    void about();

private:
    void setupMenus();
    void setupActions();

private:
    Ui::MainWindow *ui;
    SearchWidget *m_searchWidget;
};

#endif // MAINWINDOW_H 