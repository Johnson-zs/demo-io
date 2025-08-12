#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include "core/ProgressObserver.h"
#include "core/CopyTask.h"

// Forward declarations
class CopyTaskManager;
class ProgressDialog;

/**
 * @brief Main application window
 * 
 * This is the primary UI for the file copy algorithm validation framework.
 * It provides controls for selecting source/destination, choosing algorithms,
 * and monitoring copy progress.
 */
class MainWindow : public QMainWindow, public ProgressObserver
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    // ProgressObserver implementation
    void onProgressUpdate(qint64 current, qint64 total) override;
    void onFileStart(const QString& filename) override;
    void onFileComplete(const QString& filename) override;
    void onError(const QString& error) override;
    void onComplete() override;
    bool shouldPause() const override;
    bool shouldStop() const override;
    void waitWhilePaused() override;

private slots:
    void selectSourceFile();
    void selectSourceDirectory();
    void selectDestination();
    void startCopy();
    void pauseCopy();
    void resumeCopy();
    void stopCopy();
    void onAlgorithmChanged();
    void onTaskStateChanged(TaskState state);
    void onTaskError(const QString& error);
    void onTaskProgressChanged(const CopyProgress& progress);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void updateUI();
    void updateProgress(qint64 current, qint64 total);
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void resetProgress();
    QString formatBytes(qint64 bytes);
    QString formatDuration(int durationMs);

    // UI Components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    
    // Source selection group
    QGroupBox* m_sourceGroup;
    QGridLayout* m_sourceLayout;
    QLineEdit* m_sourceEdit;
    QPushButton* m_selectFileButton;
    QPushButton* m_selectDirButton;
    
    // Destination selection group
    QGroupBox* m_destGroup;
    QHBoxLayout* m_destLayout;
    QLineEdit* m_destEdit;
    QPushButton* m_selectDestButton;
    
    // Algorithm selection group
    QGroupBox* m_algorithmGroup;
    QVBoxLayout* m_algorithmLayout;
    QComboBox* m_algorithmCombo;
    QLabel* m_algorithmInfo;
    
    // Progress group
    QGroupBox* m_progressGroup;
    QVBoxLayout* m_progressLayout;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_currentFileLabel;
    QLabel* m_speedLabel;
    
    // Control buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_startButton;
    QPushButton* m_pauseButton;
    QPushButton* m_stopButton;
    
    // Log area
    QGroupBox* m_logGroup;
    QVBoxLayout* m_logLayout;
    QTextEdit* m_logEdit;
    QPushButton* m_clearLogButton;
    
    // Status bar
    QLabel* m_statusLabel;
    
    // Core components
    CopyTaskManager* m_taskManager;
    CopyTask* m_currentTask;
    ProgressDialog* m_progressDialog;
    
    // State tracking
    QTime m_startTime;
    qint64 m_lastBytes;
    QTime m_lastUpdateTime;
};

#endif // MAINWINDOW_H 