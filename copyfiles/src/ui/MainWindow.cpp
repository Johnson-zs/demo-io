#include "MainWindow.h"
#include "core/CopyTaskManager.h"
#include "algorithms/DefaultCopyAlgorithm.h"
#include "algorithms/synccopyalgorithm.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_centralWidget(nullptr), m_taskManager(nullptr), m_currentTask(nullptr), m_progressDialog(nullptr), m_lastBytes(0)
{
    setWindowTitle("File Copy Algorithm Validation Framework");
    setMinimumSize(800, 600);
    resize(1000, 700);

    // Initialize components
    m_taskManager = new CopyTaskManager(this);

    // Register default algorithm
    m_taskManager->registerAlgorithm("Sync Algorithm",
                                     new SyncCopyAlgorithm(this));
    m_taskManager->registerAlgorithm("Default Algorithm",
                                     new DefaultCopyAlgorithm(this));

    setupUI();
    setupMenuBar();
    setupStatusBar();
    connectSignals();
    updateUI();

    // Initialize time tracking
    m_lastUpdateTime = QTime::currentTime();
}

MainWindow::~MainWindow()
{
    if (m_currentTask) {
        m_currentTask->stop();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create source selection group
    m_sourceGroup = new QGroupBox("Source Selection", this);
    m_sourceLayout = new QGridLayout(m_sourceGroup);

    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setPlaceholderText("Select source file or directory...");
    m_sourceEdit->setReadOnly(true);

    m_selectFileButton = new QPushButton("Select File", this);
    m_selectDirButton = new QPushButton("Select Directory", this);

    m_sourceLayout->addWidget(new QLabel("Source:"), 0, 0);
    m_sourceLayout->addWidget(m_sourceEdit, 0, 1);
    m_sourceLayout->addWidget(m_selectFileButton, 0, 2);
    m_sourceLayout->addWidget(m_selectDirButton, 0, 3);

    // Create destination selection group
    m_destGroup = new QGroupBox("Destination Selection", this);
    m_destLayout = new QHBoxLayout(m_destGroup);

    m_destEdit = new QLineEdit(this);
    m_destEdit->setPlaceholderText("Select destination directory...");
    m_destEdit->setReadOnly(true);

    m_selectDestButton = new QPushButton("Select Destination", this);

    m_destLayout->addWidget(new QLabel("Destination:"));
    m_destLayout->addWidget(m_destEdit);
    m_destLayout->addWidget(m_selectDestButton);

    // Create algorithm selection group
    m_algorithmGroup = new QGroupBox("Algorithm Selection", this);
    m_algorithmLayout = new QVBoxLayout(m_algorithmGroup);

    m_algorithmCombo = new QComboBox(this);
    m_algorithmInfo = new QLabel("Algorithm information will be displayed here.", this);
    m_algorithmInfo->setWordWrap(true);
    m_algorithmInfo->setStyleSheet("QLabel { color: #666; font-style: italic; }");

    m_algorithmLayout->addWidget(new QLabel("Select Algorithm:"));
    m_algorithmLayout->addWidget(m_algorithmCombo);
    m_algorithmLayout->addWidget(m_algorithmInfo);

    // Create progress group
    m_progressGroup = new QGroupBox("Progress", this);
    m_progressLayout = new QVBoxLayout(m_progressGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    m_progressLabel = new QLabel("Ready", this);
    m_currentFileLabel = new QLabel("", this);
    m_speedLabel = new QLabel("", this);

    m_progressLayout->addWidget(m_progressBar);
    m_progressLayout->addWidget(m_progressLabel);
    m_progressLayout->addWidget(m_currentFileLabel);
    m_progressLayout->addWidget(m_speedLabel);

    // Create control buttons
    m_buttonLayout = new QHBoxLayout();

    m_startButton = new QPushButton("Start Copy", this);
    m_pauseButton = new QPushButton("Pause", this);
    m_stopButton = new QPushButton("Stop", this);

    m_startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    m_pauseButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; }");
    m_stopButton->setStyleSheet("QPushButton { background-color: #F44336; color: white; font-weight: bold; }");

    m_buttonLayout->addWidget(m_startButton);
    m_buttonLayout->addWidget(m_pauseButton);
    m_buttonLayout->addWidget(m_stopButton);
    m_buttonLayout->addStretch();

    // Create log group
    m_logGroup = new QGroupBox("Log", this);
    m_logLayout = new QVBoxLayout(m_logGroup);

    m_logEdit = new QTextEdit(this);
    m_logEdit->setMaximumHeight(150);
    m_logEdit->setReadOnly(true);
    m_logEdit->setStyleSheet("QTextEdit { background-color: #ffffff; border: 1px solid #cccccc; font-family: monospace; font-size: 9pt; }");

    m_clearLogButton = new QPushButton("Clear Log", this);
    m_clearLogButton->setMaximumWidth(100);

    QHBoxLayout *logButtonLayout = new QHBoxLayout();
    logButtonLayout->addStretch();
    logButtonLayout->addWidget(m_clearLogButton);

    m_logLayout->addWidget(m_logEdit);
    m_logLayout->addLayout(logButtonLayout);

    // Add all groups to main layout
    m_mainLayout->addWidget(m_sourceGroup);
    m_mainLayout->addWidget(m_destGroup);
    m_mainLayout->addWidget(m_algorithmGroup);
    m_mainLayout->addWidget(m_progressGroup);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_logGroup);

    // Populate algorithm combo box
    QStringList algorithms = m_taskManager->getAvailableAlgorithms();
    m_algorithmCombo->addItems(algorithms);

    if (!algorithms.isEmpty()) {
        onAlgorithmChanged();
    }
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Help menu
    QMenu *helpMenu = menuBar->addMenu("&Help");

    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About",
                           "File Copy Algorithm Validation Framework v1.0\n\n"
                           "A demonstration application for validating and comparing "
                           "different file copying algorithms.");
    });
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::connectSignals()
{
    // File selection buttons
    connect(m_selectFileButton, &QPushButton::clicked,
            this, &MainWindow::selectSourceFile);
    connect(m_selectDirButton, &QPushButton::clicked,
            this, &MainWindow::selectSourceDirectory);
    connect(m_selectDestButton, &QPushButton::clicked,
            this, &MainWindow::selectDestination);

    // Control buttons
    connect(m_startButton, &QPushButton::clicked,
            this, &MainWindow::startCopy);
    connect(m_pauseButton, &QPushButton::clicked,
            this, &MainWindow::pauseCopy);
    connect(m_stopButton, &QPushButton::clicked,
            this, &MainWindow::stopCopy);

    // Algorithm selection
    connect(m_algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);

    // Log clear button
    connect(m_clearLogButton, &QPushButton::clicked,
            m_logEdit, &QTextEdit::clear);

    // Task manager signals
    connect(m_taskManager, &CopyTaskManager::taskCreated,
            [this](CopyTask *task) {
                m_logEdit->append(QString("Task created: %1 -> %2")
                                          .arg(task->getSource())
                                          .arg(task->getDestination()));
            });
}

void MainWindow::selectSourceFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Select Source File", QString(), "All Files (*)");

    if (!fileName.isEmpty()) {
        m_sourceEdit->setText(fileName);
        m_logEdit->append(QString("Source file selected: %1").arg(fileName));
        updateUI();
    }
}

void MainWindow::selectSourceDirectory()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select Source Directory", QString());

    if (!dirName.isEmpty()) {
        m_sourceEdit->setText(dirName);
        m_logEdit->append(QString("Source directory selected: %1").arg(dirName));
        updateUI();
    }
}

void MainWindow::selectDestination()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select Destination Directory", QString());

    if (!dirName.isEmpty()) {
        m_destEdit->setText(dirName);
        m_logEdit->append(QString("Destination selected: %1").arg(dirName));
        updateUI();
    }
}

void MainWindow::startCopy()
{
    QString source = m_sourceEdit->text();
    QString destination = m_destEdit->text();
    QString algorithm = m_algorithmCombo->currentText();

    if (source.isEmpty() || destination.isEmpty()) {
        showError("Please select both source and destination.");
        return;
    }

    // Prepare destination path
    QFileInfo sourceInfo(source);
    QString finalDestination;

    if (sourceInfo.isFile()) {
        // For files, append the filename to destination directory
        finalDestination = destination + "/" + sourceInfo.fileName();
    } else if (sourceInfo.isDir()) {
        // For directories, append the directory name to destination
        finalDestination = destination + "/" + sourceInfo.baseName();
    } else {
        showError("Source is neither a file nor a directory.");
        return;
    }

    // Check if destination already exists
    if (QFileInfo::exists(finalDestination)) {
        int ret = QMessageBox::question(this, "File Exists",
                                        QString("Destination '%1' already exists. Overwrite?").arg(finalDestination),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    m_logEdit->append(QString("Final destination: %1").arg(finalDestination));

    // Create task
    m_currentTask = m_taskManager->createTask(source, finalDestination, algorithm);
    if (!m_currentTask) {
        showError("Failed to create copy task.");
        return;
    }

    // Connect task signals
    connect(m_currentTask, &CopyTask::stateChanged,
            this, &MainWindow::onTaskStateChanged);
    connect(m_currentTask, &CopyTask::errorOccurred,
            this, &MainWindow::onTaskError);
    connect(m_currentTask, &CopyTask::progressChanged,
            this, &MainWindow::onTaskProgressChanged);

    // Start the task
    m_startTime = QTime::currentTime();
    m_lastBytes = 0;
    m_lastUpdateTime = QTime::currentTime();

    m_taskManager->startTask(m_currentTask);

    m_logEdit->append(QString("Copy started: %1").arg(QTime::currentTime().toString()));
    updateUI();
}

void MainWindow::pauseCopy()
{
    if (m_currentTask) {
        TaskState state = m_currentTask->getState();
        if (state == TaskState::Running) {
            m_taskManager->pauseTask(m_currentTask);
            m_pauseButton->setText("Resume");
            m_logEdit->append("Copy paused");
        } else if (state == TaskState::Paused) {
            m_taskManager->resumeTask(m_currentTask);
            m_pauseButton->setText("Pause");
            m_logEdit->append("Copy resumed");
        }
        updateUI();
    }
}

void MainWindow::resumeCopy()
{
    // This method is now handled by pauseCopy()
    pauseCopy();
}

void MainWindow::stopCopy()
{
    if (m_currentTask) {
        m_taskManager->stopTask(m_currentTask);
        m_pauseButton->setText("Pause");   // Reset button text
        m_logEdit->append("Copy stopped");
        resetProgress();
        updateUI();
    }
}

void MainWindow::onAlgorithmChanged()
{
    QString algorithmName = m_algorithmCombo->currentText();
    if (algorithmName.isEmpty()) {
        return;
    }

    CopyAlgorithm *algorithm = m_taskManager->getAlgorithm(algorithmName);
    if (algorithm) {
        QString info = QString("Algorithm: %1\nSupports Pause/Resume: %2")
                               .arg(algorithm->getName())
                               .arg(algorithm->supportsPause() ? "Yes" : "No");
        m_algorithmInfo->setText(info);
    }
}

void MainWindow::onTaskStateChanged(TaskState state)
{
    QString stateStr;
    switch (state) {
    case TaskState::Created:
        stateStr = "Created";
        break;
    case TaskState::Running:
        stateStr = "Running";
        break;
    case TaskState::Paused:
        stateStr = "Paused";
        break;
    case TaskState::Completed:
        stateStr = "Completed";
        break;
    case TaskState::Stopped:
        stateStr = "Stopped";
        break;
    case TaskState::Error:
        stateStr = "Error";
        break;
    }

    m_statusLabel->setText(QString("Status: %1").arg(stateStr));
    m_logEdit->append(QString("Task state changed: %1").arg(stateStr));

    if (state == TaskState::Completed) {
        // Calculate and log total duration
        QTime endTime = QTime::currentTime();
        int durationMs = m_startTime.msecsTo(endTime);
        QString durationStr = formatDuration(durationMs);
        m_logEdit->append(QString("Task completed successfully! Total time: %1").arg(durationStr));

        showSuccess("Copy completed successfully!");
        resetProgress();
    }

    updateUI();
}

void MainWindow::onTaskError(const QString &error)
{
    showError(QString("Copy error: %1").arg(error));
    resetProgress();
    updateUI();
}

void MainWindow::onTaskProgressChanged(const CopyProgress &progress)
{
    updateProgress(progress.copiedBytes, progress.totalBytes);
    if (!progress.currentFile.isEmpty()) {
        m_currentFileLabel->setText(QString("Copying: %1").arg(QFileInfo(progress.currentFile).fileName()));
    }
}

void MainWindow::updateUI()
{
    bool hasTask = (m_currentTask != nullptr);
    bool isRunning = hasTask && (m_currentTask->getState() == TaskState::Running);
    bool isPaused = hasTask && (m_currentTask->getState() == TaskState::Paused);
    bool canStart = !m_sourceEdit->text().isEmpty() && !m_destEdit->text().isEmpty() && (!hasTask || (m_currentTask->getState() == TaskState::Completed || m_currentTask->getState() == TaskState::Stopped || m_currentTask->getState() == TaskState::Error));

    m_startButton->setEnabled(canStart);
    m_pauseButton->setEnabled(isRunning || isPaused);
    m_stopButton->setEnabled(isRunning || isPaused);

    // Update button text based on state
    if (isPaused) {
        m_startButton->setText("Resume");
        m_startButton->setEnabled(true);
        connect(m_startButton, &QPushButton::clicked,
                this, &MainWindow::resumeCopy, Qt::UniqueConnection);
    } else {
        m_startButton->setText("Start Copy");
        disconnect(m_startButton, &QPushButton::clicked,
                   this, &MainWindow::resumeCopy);
        connect(m_startButton, &QPushButton::clicked,
                this, &MainWindow::startCopy, Qt::UniqueConnection);
    }

    // Enable/disable file selection during copy
    bool canSelectFiles = !isRunning && !isPaused;
    m_selectFileButton->setEnabled(canSelectFiles);
    m_selectDirButton->setEnabled(canSelectFiles);
    m_selectDestButton->setEnabled(canSelectFiles);
    m_algorithmCombo->setEnabled(canSelectFiles);
}

void MainWindow::updateProgress(qint64 current, qint64 total)
{
    if (total > 0) {
        int percentage = static_cast<int>((current * 100) / total);
        m_progressBar->setValue(percentage);

        QString progressText = QString("%1% (%2 / %3)")
                                       .arg(percentage)
                                       .arg(formatBytes(current))
                                       .arg(formatBytes(total));
        m_progressLabel->setText(progressText);

        // Calculate speed
        QTime currentTime = QTime::currentTime();
        int msecsSinceLastUpdate = m_lastUpdateTime.msecsTo(currentTime);

        if (msecsSinceLastUpdate > 1000) {   // Update speed every second
            qint64 bytesDiff = current - m_lastBytes;
            double speed = (bytesDiff * 1000.0) / msecsSinceLastUpdate;   // bytes per second

            m_speedLabel->setText(QString("Speed: %1/s").arg(formatBytes(static_cast<qint64>(speed))));

            m_lastBytes = current;
            m_lastUpdateTime = currentTime;
        }
    }
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "Error", message);
    m_logEdit->append(QString("ERROR: %1").arg(message));
}

void MainWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "Success", message);
    m_logEdit->append(QString("SUCCESS: %1").arg(message));
}

void MainWindow::resetProgress()
{
    m_progressBar->setValue(0);
    m_progressLabel->setText("Ready");
    m_currentFileLabel->setText("");
    m_speedLabel->setText("");
    m_statusLabel->setText("Ready");
}

QString MainWindow::formatBytes(qint64 bytes)
{
    const QStringList units = { "B", "KB", "MB", "GB", "TB" };
    int unitIndex = 0;
    double size = bytes;

    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

QString MainWindow::formatDuration(int durationMs)
{
    if (durationMs < 1000) {
        return QString("%1ms").arg(durationMs);
    } else if (durationMs < 60000) {
        double seconds = durationMs / 1000.0;
        return QString("%1s").arg(seconds, 0, 'f', 2);
    } else if (durationMs < 3600000) {
        int minutes = durationMs / 60000;
        int seconds = (durationMs % 60000) / 1000;
        return QString("%1m %2s").arg(minutes).arg(seconds);
    } else {
        int hours = durationMs / 3600000;
        int minutes = (durationMs % 3600000) / 60000;
        int seconds = (durationMs % 60000) / 1000;
        return QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
    }
}

// ProgressObserver implementation
void MainWindow::onProgressUpdate(qint64 current, qint64 total)
{
    QMetaObject::invokeMethod(
            this, [this, current, total]() {
                updateProgress(current, total);
            },
            Qt::QueuedConnection);
}

void MainWindow::onFileStart(const QString &filename)
{
    QMetaObject::invokeMethod(
            this, [this, filename]() {
                m_currentFileLabel->setText(QString("Copying: %1").arg(QFileInfo(filename).fileName()));
            },
            Qt::QueuedConnection);
}

void MainWindow::onFileComplete(const QString &filename)
{
    QMetaObject::invokeMethod(
            this, [this, filename]() {
                m_logEdit->append(QString("Completed: %1").arg(QFileInfo(filename).fileName()));
            },
            Qt::QueuedConnection);
}

void MainWindow::onError(const QString &error)
{
    QMetaObject::invokeMethod(
            this, [this, error]() {
                showError(error);
            },
            Qt::QueuedConnection);
}

void MainWindow::onComplete()
{
    QMetaObject::invokeMethod(
            this, [this]() {
                showSuccess("Copy operation completed successfully!");
                resetProgress();
                updateUI();
            },
            Qt::QueuedConnection);
}

bool MainWindow::shouldPause() const
{
    // MainWindow doesn't control pause state directly
    // This is handled by the CopyWorker
    return false;
}

bool MainWindow::shouldStop() const
{
    // MainWindow doesn't control stop state directly
    // This is handled by the CopyWorker
    return false;
}

void MainWindow::waitWhilePaused()
{
    // MainWindow doesn't implement waiting directly
    // This is handled by the CopyWorker
}
