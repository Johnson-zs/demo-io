#include "resultprocessor.h"

#include <QtConcurrent>
#include <QFileInfo>
#include <algorithm>

ResultProcessor::ResultProcessor(QObject *parent)
    : QObject(parent)
{
    // 初始化默认偏好扩展名
    m_preferredExtensions << "txt" << "pdf" << "doc" << "docx" << "xls" << "xlsx" << "ppt" << "pptx";
}

void ResultProcessor::processResults(const QStringList &results, const QString &query)
{
    if (results.isEmpty()) {
        emit processingFinished(QStringList());
        return;
    }

#if 0
    // 创建结果副本
    QStringList sortedResults = results;
    
    // 并行计算分数
    QMutexLocker locker(&m_mutex);
    m_scoreCache.clear();
    
    QtConcurrent::blockingMap(sortedResults, [this, query](const QString &filePath) {
        QMutexLocker locker(&m_mutex);
        m_scoreCache[filePath] = calculateScore(filePath, query);
    });
    
    // 根据分数排序
    std::sort(sortedResults.begin(), sortedResults.end(), [this](const QString &a, const QString &b) {
        QMutexLocker locker(&m_mutex);
        return m_scoreCache[a] > m_scoreCache[b]; // 降序排列
    });
    
    emit processingFinished(sortedResults);
#else
    emit processingFinished(results);
#endif
}

void ResultProcessor::setPreferredExtensions(const QStringList &extensions)
{
    m_preferredExtensions = extensions;
}

double ResultProcessor::calculateScore(const QString &filePath, const QString &query)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    double score = 0.0;
    
    // 1. 精确匹配得分最高
    if (fileName.compare(query, Qt::CaseInsensitive) == 0) {
        score += 100.0;
    }
    // 2. 前缀匹配次之
    else if (fileName.startsWith(query, Qt::CaseInsensitive)) {
        score += 80.0;
    }
    // 3. 单词边界匹配
    else if (matchesWordBoundaries(fileName, query)) {
        score += 60.0;
    }
    // 4. 子串匹配
    else if (fileName.contains(query, Qt::CaseInsensitive)) {
        score += 40.0;
    }
    // 5. 模糊匹配
    else {
        score += calculateFuzzyScore(fileName, query);
    }
    
    // 加上文件扩展名偏好得分
    score += fileExtensionBonus(filePath);
    
    // 路径深度惩罚（浅路径优先）
    int depth = filePath.count('/');
    score -= depth * 0.5;
    
    return score;
}

double ResultProcessor::calculateFuzzyScore(const QString &fileName, const QString &query)
{
    // 简单的模糊匹配算法
    double score = 0;
    int lastFoundPos = -1;
    
    // 检查查询中的每个字符是否按顺序出现在文件名中
    for (int i = 0; i < query.length(); i++) {
        int pos = fileName.indexOf(query[i], lastFoundPos + 1, Qt::CaseInsensitive);
        if (pos == -1) {
            return 0; // 如果有字符不匹配，则得分为0
        }
        
        // 连续匹配得分高
        if (lastFoundPos != -1 && pos == lastFoundPos + 1) {
            score += 2.0;
        } else {
            score += 1.0;
        }
        
        lastFoundPos = pos;
    }
    
    return score * 5.0;  // 调整得分范围
}

double ResultProcessor::fileExtensionBonus(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    // 偏好的扩展名给额外加分
    if (m_preferredExtensions.contains(extension)) {
        return 10.0;
    }
    
    return 0.0;
}

bool ResultProcessor::matchesWordBoundaries(const QString &fileName, const QString &query)
{
    // 检查查询是否匹配文件名中的单词边界
    // 例如，"doc"应该匹配"my_document.txt"中的"doc"
    QRegularExpression wordBoundaryRegex(QString("\\b%1").arg(QRegularExpression::escape(query)), 
                                        QRegularExpression::CaseInsensitiveOption);
    
    return wordBoundaryRegex.match(fileName).hasMatch();
} 
