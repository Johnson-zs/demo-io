#include "ProgressDialog.h"
#include <QTime>
#include <QFileInfo>

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent)
    , m_lastBytes(0)
    , m_isPaused(false)
{
    setWindowTitle("Copy Progress");
    setMinimumSize(400, 300);
    resize(500, 400);
    setModal(true);
    
    setupUI();
    reset();
}

void ProgressDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    m_titleLabel = new QLabel("Copying Files...", this);
    m_titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    
    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    // Progress info labels
    m_progressLabel = new QLabel("0% (0 B / 0 B)", this);
    m_currentFileLabel = new QLabel("", this);
    m_speedLabel = new QLabel("Speed: 0 B/s", this);
    m_etaLabel = new QLabel("ETA: --", this);
    
    // Log area
    m_logEdit = new QTextEdit(this);
    m_logEdit->setMaximumHeight(120);
    m_logEdit->setReadOnly(true);
    m_logEdit->setStyleSheet("QTextEdit { background-color: #f8f8f8; font-family: monospace; font-size: 9px; }");
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    m_pauseButton = new QPushButton("Pause", this);
    m_cancelButton = new QPushButton("Cancel", this);
    m_closeButton = new QPushButton("Close", this);
    
    m_pauseButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; }");
    m_cancelButton->setStyleSheet("QPushButton { background-color: #F44336; color: white; }");
    m_closeButton->setEnabled(false);
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_pauseButton);
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_closeButton);
    
    // Add to main layout
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_progressLabel);
    m_mainLayout->addWidget(m_currentFileLabel);
    m_mainLayout->addWidget(m_speedLabel);
    m_mainLayout->addWidget(m_etaLabel);
    m_mainLayout->addWidget(m_logEdit);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_pauseButton, &QPushButton::clicked, this, &ProgressDialog::onPauseClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancelClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void ProgressDialog::onProgressUpdate(qint64 current, qint64 total)
{
    if (total > 0) {
        int percentage = static_cast<int>((current * 100) / total);
        m_progressBar->setValue(percentage);
        
        QString progressText = QString("%1% (%2 / %3)")
                              .arg(percentage)
                              .arg(formatBytes(current))
                              .arg(formatBytes(total));
        m_progressLabel->setText(progressText);
        
        updateETA(current, total);
        
        // Calculate speed
        QTime currentTime = QTime::currentTime();
        int msecsSinceLastUpdate = m_lastUpdateTime.msecsTo(currentTime);
        
        if (msecsSinceLastUpdate > 1000) { // Update every second
            qint64 bytesDiff = current - m_lastBytes;
            double speed = (bytesDiff * 1000.0) / msecsSinceLastUpdate;
            
            m_speedLabel->setText(QString("Speed: %1/s").arg(formatBytes(static_cast<qint64>(speed))));
            
            m_lastBytes = current;
            m_lastUpdateTime = currentTime;
        }
    }
}

void ProgressDialog::onFileStart(const QString& filename)
{
    QString shortName = QFileInfo(filename).fileName();
    m_currentFileLabel->setText(QString("Copying: %1").arg(shortName));
    m_logEdit->append(QString("Started: %1").arg(shortName));
}

void ProgressDialog::onFileComplete(const QString& filename)
{
    QString shortName = QFileInfo(filename).fileName();
    m_logEdit->append(QString("Completed: %1").arg(shortName));
}

void ProgressDialog::onError(const QString& error)
{
    m_logEdit->append(QString("ERROR: %1").arg(error));
    m_titleLabel->setText("Copy Failed!");
    m_titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; color: red; }");
    
    m_pauseButton->setEnabled(false);
    m_cancelButton->setText("Close");
    m_closeButton->setEnabled(true);
}

void ProgressDialog::onComplete()
{
    m_titleLabel->setText("Copy Completed Successfully!");
    m_titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; color: green; }");
    m_currentFileLabel->setText("All files copied successfully");
    m_etaLabel->setText("ETA: Completed");
    
    m_pauseButton->setEnabled(false);
    m_cancelButton->setText("Close");
    m_closeButton->setEnabled(true);
    
    m_logEdit->append("Copy operation completed successfully!");
}

void ProgressDialog::reset()
{
    m_progressBar->setValue(0);
    m_progressLabel->setText("0% (0 B / 0 B)");
    m_currentFileLabel->setText("");
    m_speedLabel->setText("Speed: 0 B/s");
    m_etaLabel->setText("ETA: --");
    m_logEdit->clear();
    
    m_titleLabel->setText("Copying Files...");
    m_titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    
    m_pauseButton->setEnabled(true);
    m_pauseButton->setText("Pause");
    m_cancelButton->setText("Cancel");
    m_closeButton->setEnabled(false);
    
    m_startTime = QTime::currentTime();
    m_lastUpdateTime = QTime::currentTime();
    m_lastBytes = 0;
    m_isPaused = false;
}

void ProgressDialog::onCancelClicked()
{
    if (m_cancelButton->text() == "Close") {
        accept();
    } else {
        emit cancelRequested();
    }
}

void ProgressDialog::onPauseClicked()
{
    if (m_isPaused) {
        m_pauseButton->setText("Pause");
        m_isPaused = false;
        m_logEdit->append("Resumed by user");
    } else {
        m_pauseButton->setText("Resume");
        m_isPaused = true;
        m_logEdit->append("Paused by user");
    }
    
    emit pauseRequested();
}

void ProgressDialog::updateETA(qint64 current, qint64 total)
{
    if (current == 0 || total == 0) {
        m_etaLabel->setText("ETA: --");
        return;
    }
    
    QTime currentTime = QTime::currentTime();
    int elapsedMs = m_startTime.msecsTo(currentTime);
    
    if (elapsedMs < 1000) { // Less than 1 second
        m_etaLabel->setText("ETA: Calculating...");
        return;
    }
    
    double progress = static_cast<double>(current) / total;
    if (progress <= 0.01) { // Less than 1% progress
        m_etaLabel->setText("ETA: Calculating...");
        return;
    }
    
    int totalEstimatedMs = static_cast<int>(elapsedMs / progress);
    int remainingMs = totalEstimatedMs - elapsedMs;
    int remainingSeconds = remainingMs / 1000;
    
    m_etaLabel->setText(QString("ETA: %1").arg(formatTime(remainingSeconds)));
}

QString ProgressDialog::formatTime(int seconds)
{
    if (seconds < 60) {
        return QString("%1s").arg(seconds);
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        int remainingSeconds = seconds % 60;
        return QString("%1m %2s").arg(minutes).arg(remainingSeconds);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return QString("%1h %2m").arg(hours).arg(minutes);
    }
}

QString ProgressDialog::formatBytes(qint64 bytes)
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

#include "ProgressDialog.moc" 