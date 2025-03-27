#include "lucenesearchengine.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <unistd.h>

using namespace Lucene;

LuceneSearchEngine::LuceneSearchEngine()
{
}

void LuceneSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_cachedAllFiles.clear();
}

QString LuceneSearchEngine::getIndexDirectory() const
{
    return QString("/run/user/%1/deepin-anything-server").arg(getuid());
}

QString LuceneSearchEngine::getHomeDirectory() const
{
    QString homeDir;

    if (QFileInfo::exists("/data/home")) {
        homeDir = "/data";
    } else if (QFileInfo::exists("/persistent/home")) {
        homeDir = "/persistent";
    }

    homeDir.append(QDir::homePath());

    return homeDir;
}

QStringList LuceneSearchEngine::performLuceneSearch(const QString &originPath, const QString &key, bool nrt) const
{
    QString keywords = key;
    QString path = originPath;
    
    if (path.startsWith(QDir::homePath()))
        path.replace(0, QDir::homePath().length(), getHomeDirectory());

    if (keywords.isEmpty()) {
        return {};
    }

    // 原始词条
    String query_terms = StringUtils::toUnicode(keywords.toStdString());

    // 给普通 parser 用
    if (keywords.at(0) == QChar('*') || keywords.at(0) == QChar('?')) {
        keywords = keywords.mid(1);
    }

    try {
        int32_t max_results;

        // 获取索引目录
        QString indexDir = getIndexDirectory();
        qDebug() << "搜索索引目录:" << indexDir;

        // 打开索引目录
        FSDirectoryPtr directory = FSDirectory::open(StringUtils::toUnicode(indexDir.toStdString()));

        // 检查索引是否存在
        if (!IndexReader::indexExists(directory)) {
            qWarning() << "索引不存在:" << indexDir;
            return QStringList();
        }

        // 打开索引读取器
        IndexReaderPtr reader = IndexReader::open(directory, true);
        if (reader->numDocs() == 0) {
            qWarning() << "索引为空，没有文档";
            return QStringList();
        }

        // 创建搜索器
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);

        if (reader->numDocs() == 0) {
            qWarning() << "索引为空，没有文档";
            return QStringList();
        }

        max_results = reader->numDocs();

        String queryString = L"*" + StringUtils::toLower(StringUtils::toUnicode(keywords.toStdString())) + L"*";
        TermPtr term = newLucene<Term>(L"file_name", queryString);
        QueryPtr query = newLucene<WildcardQuery>(term);

        auto search_results = searcher->search(query, max_results);

        QStringList results;
        results.reserve(search_results->scoreDocs.size());
        QStringList dirs, files;
        dirs.reserve(search_results->scoreDocs.size() / 2);   // 预估目录数量
        files.reserve(search_results->scoreDocs.size() / 2);   // 预估文件数量

        for (const auto &score_doc : search_results->scoreDocs) {
            DocumentPtr doc = searcher->doc(score_doc->doc);
            auto result = QString::fromStdWString(doc->get(L"full_path"));

            // 先进行路径过滤，避免不必要的类型判断
            if (!result.startsWith(path)) continue;

            // 延迟获取类型，只有通过路径过滤的才需要判断
            auto type = QString::fromStdWString(doc->get(L"file_type"));

            // 使用移动语义 + 条件分类
            if (type == "dir") {
                dirs.append(std::move(result));   // 目录列表
            } else {
                files.append(std::move(result));   // 文件列表
            }
        }

        // 合并结果（O(1) 时间复杂度操作）
        results = std::move(dirs) + std::move(files);

        return results;
    } catch (const LuceneException &e) {
        qWarning() << "Lucene搜索异常:" << QString::fromStdWString(e.getError());
        return {};
    }
}

QVector<FileData> LuceneSearchEngine::convertToFileData(const QStringList &paths) const
{
    QVector<FileData> result;
    result.reserve(paths.size());
    
    for (const QString &path : paths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            result.append(FileData::fromFileInfo(fileInfo));
        }
    }
    
    return result;
}

QVector<FileData> LuceneSearchEngine::getAllFiles(int limit) const
{
    // 对于Lucene++实现，我们可以先做一个空搜索来获取所有文件
    // 这里可以实现缓存以提高性能
    if (m_cachedAllFiles.isEmpty()) {
        QStringList allPaths = performLuceneSearch(m_currentPath, "*", false);
        m_cachedAllFiles = convertToFileData(allPaths);
    }
    
    if (limit < 0 || limit >= m_cachedAllFiles.size()) {
        return m_cachedAllFiles;
    }
    
    return QVector<FileData>(m_cachedAllFiles.constBegin(), m_cachedAllFiles.constBegin() + limit);
}

QVector<FileData> LuceneSearchEngine::searchFiles(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        return getAllFiles(1000);  // 限制返回数量以保证性能
    }
    
    QStringList paths = performLuceneSearch(m_currentPath, keyword, false);
    return convertToFileData(paths);
} 