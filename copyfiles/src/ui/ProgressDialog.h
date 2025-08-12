#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include "core/ProgressObserver.h"

/**
 * @brief Detailed progress dialog
 * 
 * This dialog provides a detailed view of the copy progress,
 * including current file, speed, ETA, and detailed log.
 */
class ProgressDialog : public QDialog, public ProgressObserver
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = nullptr);
    virtual ~ProgressDialog() = default;

    // ProgressObserver implementation
    void onProgressUpdate(qint64 current, qint64 total) override;
    void onFileStart(const QString& filename) override;
    void onFileComplete(const QString& filename) override;
    void onError(const QString& error) override;
    void onComplete() override;

    void reset();

signals:
    void cancelRequested();
    void pauseRequested();

private slots:
    void onCancelClicked();
    void onPauseClicked();

private:
    void setupUI();
    void updateETA(qint64 current, qint64 total);
    QString formatTime(int seconds);
    QString formatBytes(qint64 bytes);

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_currentFileLabel;
    QLabel* m_speedLabel;
    QLabel* m_etaLabel;
    QTextEdit* m_logEdit;
    
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_pauseButton;
    QPushButton* m_cancelButton;
    QPushButton* m_closeButton;

    // State tracking
    QTime m_startTime;
    QTime m_lastUpdateTime;
    qint64 m_lastBytes;
    bool m_isPaused;
};

#endif // PROGRESSDIALOG_H 