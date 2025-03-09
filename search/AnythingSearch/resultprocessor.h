#ifndef RESULTPROCESSOR_H
#define RESULTPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <QFileInfo>
#include <QMap>
#include <QMutex>

class ResultProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ResultProcessor(QObject *parent = nullptr);
    
    // 处理和排序搜索结果
    void processResults(const QStringList &results, const QString &query);
    
    // 设置偏好的文件扩展名
    void setPreferredExtensions(const QStringList &extensions);

signals:
    // 处理完成信号
    void processingFinished(const QStringList &sortedResults);

private:
    // 计算文件得分
    double calculateScore(const QString &filePath, const QString &query);
    
    // 计算模糊匹配得分
    double calculateFuzzyScore(const QString &fileName, const QString &query);
    
    // 基于文件扩展名的加分
    double fileExtensionBonus(const QString &filePath);
    
    // 单词边界匹配判断
    bool matchesWordBoundaries(const QString &fileName, const QString &query);

    QStringList m_preferredExtensions;
    QMap<QString, double> m_scoreCache;
    QMutex m_mutex;
};

#endif // RESULTPROCESSOR_H 