#include "mocksearcher.h"

#include <QDirIterator>
#include <QRandomGenerator>
#include <QDebug>
#include <QRegularExpression>

MockSearcher::MockSearcher(QObject *parent)
    : SearcherInterface(parent),
      m_isCancelled(false),
      m_simulateErrors(false),
      m_simulatedDelay(100),
      m_maxFileCount(10000)
{
    m_delayTimer = new QTimer(this);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, [this]() {
        performSearch(m_currentPath, m_currentQuery);
    });
}

MockSearcher::~MockSearcher()
{
    cancelSearch();
}

bool MockSearcher::requestSearch(const QString &path, const QString &text)
{
    if (path.isEmpty() || text.isEmpty())
        return false;
        
    // 取消先前的搜索
    cancelSearch();
    
    m_currentPath = path;
    m_currentQuery = text;
    m_isCancelled = false;
    
    // 模拟网络延迟
    m_delayTimer->start(m_simulatedDelay);
    
    return true;
}

void MockSearcher::cancelSearch()
{
    m_delayTimer->stop();
    m_isCancelled = true;
    
    // 等待搜索线程完成
    if (m_searchFuture.isRunning()) {
        m_searchFuture.waitForFinished();
    }
}

void MockSearcher::setSimulatedDelay(int msec)
{
    m_simulatedDelay = msec;
}

void MockSearcher::setSimulateErrors(bool simulate)
{
    m_simulateErrors = simulate;
}

void MockSearcher::setMaxFileCount(int count)
{
    m_maxFileCount = count;
}

void MockSearcher::performSearch(const QString &path, const QString &text)
{
    // 随机模拟错误
    if (m_simulateErrors && QRandomGenerator::global()->bounded(10) == 0) {
        emit searchFailed(text, "模拟的搜索错误");
        return;
    }
    
    // 在单独的线程中执行搜索
    m_searchFuture = QtConcurrent::run([this, path, text]() {
        QStringList results;
        
        try {
            // 使用通配符模式进行搜索
            QString pattern = "*";
            if (!text.isEmpty()) {
                pattern = QString("*%1*").arg(text);
            }
            
            results = scanDirectory(path, pattern);
            
            // 限制结果数量
            if (results.size() > m_maxFileCount) {
                results = results.mid(0, m_maxFileCount);
            }
            
            // // 格式化结果为 Anything 格式
            // for (int i = 0; i < results.size(); ++i) {
            //     results[i] = formatResult(results[i]);
            // }
            
            // 检查是否已取消
            if (m_isCancelled) {
                return;
            }
            
            // 发送结果
            emit searchFinished(text, results);
        }
        catch (const std::exception &e) {
            if (!m_isCancelled) {
                emit searchFailed(text, QString("搜索异常: %1").arg(e.what()));
            }
        }
    });
}

QStringList MockSearcher::scanDirectory(const QString &path, const QString &pattern, int maxDepth)
{
    QStringList results;
    
    if (maxDepth <= 0 || m_isCancelled) {
        return results;
    }
    
    QDir dir(path);
    dir.setNameFilters(QStringList() << pattern);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    
    // 添加匹配的文件
    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        if (m_isCancelled) {
            return results;
        }
        results.append(fileInfo.absoluteFilePath());
    }
    
    // 递归处理子目录
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QFileInfoList dirList = dir.entryInfoList();
    for (const QFileInfo &dirInfo : dirList) {
        if (m_isCancelled) {
            return results;
        }
        results.append(scanDirectory(dirInfo.absoluteFilePath(), pattern, maxDepth - 1));
    }
    
    return results;
}

QString MockSearcher::formatResult(const QString &path)
{
    QFileInfo fileInfo(path);
    QString suffix = fileInfo.suffix();
    
    // 格式化为 Anything 格式: 路径<\>后缀
    return QString("%1<\\>%2").arg(path, suffix);
} 
