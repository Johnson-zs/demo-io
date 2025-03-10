#include "searchengine.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

using namespace Lucene;

SearchEngine::SearchEngine()
    : writer(nullptr), searcher(nullptr)
{
    indexPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/lucene_index";
    qDebug() << "索引路径：" << indexPath;
}

SearchEngine::~SearchEngine()
{
    if (writer) {
        try {
            writer->close();
            qDebug() << "索引写入器已关闭";
        } catch (const LuceneException &e) {
            qDebug() << "关闭索引写入器时发生错误：" << QString::fromStdWString(e.getError());
        } catch (...) {
            qDebug() << "关闭索引写入器时发生未知错误";
        }
    }
}

void SearchEngine::initializeIndex()
{
    try {
        // 清理旧的索引目录
        QDir dir(indexPath);
        if (dir.exists()) {
            dir.removeRecursively();
            qDebug() << "已清理旧的索引目录";
        }

        QDir().mkpath(indexPath);
        // 使用KeywordAnalyzer，它会将整个输入作为一个词处理，不会分词
        writer = newLucene<IndexWriter>(FSDirectory::open(StringUtils::toUnicode(indexPath.toStdString())),
                                      newLucene<KeywordAnalyzer>(),
                                      true,
                                      IndexWriter::MaxFieldLengthLIMITED);
        qDebug() << "索引初始化成功";
    } catch (const LuceneException &e) {
        qDebug() << "索引初始化失败：" << QString::fromStdWString(e.getError());
    } catch (...) {
        qDebug() << "索引初始化时发生未知错误";
    }
}

void SearchEngine::createIndex(const QStringList &files)
{
    try {
        // 重新初始化索引，清除旧索引
        initializeIndex();

        qDebug() << "开始创建索引，文件数量：" << files.size();
        int processedFiles = 0;

        for (const QString &file : files) {
            QFileInfo fileInfo(file);
            QString fileName = fileInfo.fileName().toLower();   // 转换为小写
            
            qDebug() << "正在索引文件:" << fileName;  // 添加日志，查看实际索引的文件名

            DocumentPtr doc = newLucene<Document>();
            // 存储完整路径用于显示
            doc->add(newLucene<Field>(L"path",
                                    StringUtils::toUnicode(file.toStdString()),
                                    Field::STORE_YES,
                                    Field::INDEX_NOT_ANALYZED));
            // 索引文件名用于搜索（小写）
            doc->add(newLucene<Field>(L"filename",
                                    StringUtils::toUnicode(fileName.toStdString()),
                                    Field::STORE_YES,
                                    Field::INDEX_ANALYZED));
            writer->addDocument(doc);
            processedFiles++;
        }

        writer->optimize();
        writer->commit();
        qDebug() << "索引创建完成，共索引" << processedFiles << "个文件";

    } catch (const LuceneException &e) {
        qDebug() << "创建索引失败：" << QString::fromStdWString(e.getError());
    } catch (...) {
        qDebug() << "创建索引时发生未知错误";
    }
}

QStringList SearchEngine::search(const QString &queryStr)
{
    QStringList results;
    try {
        if (!writer) {
            qDebug() << "搜索失败：索引未创建";
            return results;
        }

        qDebug() << "开始搜索：" << queryStr;

        IndexReaderPtr reader = writer->getReader();
        if (!reader) {
            qDebug() << "搜索失败：无法获取索引读取器";
            return results;
        }

        searcher = newLucene<IndexSearcher>(reader);
        if (!searcher) {
            qDebug() << "搜索失败：无法创建搜索器";
            return results;
        }

        // 将查询字符串转换为小写
        String queryString = L"*" + StringUtils::toUnicode(queryStr.toLower().toStdString()) + L"*";
        qDebug() << "转换后的查询字符串：" << QString::fromStdWString(queryString.c_str());

        // 使用通配符查询
        TermPtr term = newLucene<Term>(L"filename", queryString);
        if (!term) {
            qDebug() << "搜索失败：无法创建查询词";
            return results;
        }

        QueryPtr wildcardQuery = newLucene<WildcardQuery>(term);
        if (!wildcardQuery) {
            qDebug() << "搜索失败：无法创建通配符查询";
            return results;
        }

        // 执行搜索
        TopDocsPtr hits = searcher->search(wildcardQuery, INT_MAX);  // 使用最大值而不是固定的100
        if (!hits) {
            qDebug() << "搜索失败：无法获取搜索结果";
            return results;
        }

        qDebug() << "找到" << hits->totalHits << "个匹配结果";

        // 如果没有找到结果，列出所有索引的文件名
        if (hits->totalHits == 0) {
            qDebug() << "未找到结果，列出所有索引的文件名：";
            TermEnumPtr terms = reader->terms(newLucene<Term>(L"filename", L""));
            while (terms && terms->next()) {
                TermPtr currentTerm = terms->term();
                if (currentTerm && currentTerm->field() == L"filename") {
                    qDebug() << "索引中的文件名："
                            << QString::fromStdWString(currentTerm->text().c_str());
                }
            }
            return results;
        }

        // 获取结果
        Collection<ScoreDocPtr> scoreDocs = hits->scoreDocs;
        if (!scoreDocs) {
            qDebug() << "搜索失败：无法获取评分文档集合";
            return results;
        }

        for (int32_t i = 0; i < hits->totalHits && i < scoreDocs.size(); ++i) {
            ScoreDocPtr scoreDoc = scoreDocs[i];
            if (!scoreDoc) {
                qDebug() << "警告：第" << i << "个结果为空，跳过";
                continue;
            }

            try {
                DocumentPtr doc = searcher->doc(scoreDoc->doc);
                if (!doc) {
                    qDebug() << "警告：无法获取第" << i << "个文档，跳过";
                    continue;
                }

                String path = doc->get(L"path");
                String filename = doc->get(L"filename");
                if (!path.empty()) {
                    qDebug() << "匹配文件：" << QString::fromStdWString(filename.c_str())
                            << "路径：" << QString::fromStdWString(path.c_str());
                    results.append(QString::fromStdWString(path.c_str()));
                }
            } catch (const LuceneException &e) {
                qDebug() << "处理第" << i << "个结果时出错："
                        << QString::fromStdWString(e.getError());
                continue;
            } catch (...) {
                qDebug() << "处理第" << i << "个结果时出现未知错误";
                continue;
            }
        }

    } catch (const LuceneException &e) {
        qDebug() << "搜索失败：" << QString::fromStdWString(e.getError());
    } catch (...) {
        qDebug() << "搜索时发生未知错误";
    }
    return results;
}
