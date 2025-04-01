#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>

using namespace Lucene;

// 高亮匹配内容的简单函数
QString highlightText(const QString &text, const QString &keyword, int contextChars = 50)
{
    QString result;
    int keywordLen = keyword.length();
    int textLen = text.length();
    
    // 查找所有匹配位置
    int pos = 0;
    while ((pos = text.indexOf(keyword, pos, Qt::CaseInsensitive)) != -1) {
        // 确定上下文范围
        int start = qMax(0, pos - contextChars);
        int end = qMin(textLen, pos + keywordLen + contextChars);
        
        // 提取上下文
        QString before = text.mid(start, pos - start);
        QString match = text.mid(pos, keywordLen);
        QString after = text.mid(pos + keywordLen, end - (pos + keywordLen));
        
        // 添加省略号
        if (start > 0) before = "..." + before;
        if (end < textLen) after = after + "...";
        
        // 高亮匹配文本 (使用HTML标签)
        result += before + "<b>" + match + "</b>" + after + "\n\n";
        
        pos += keywordLen;
    }
    
    return result;
}

// 搜索部分单词匹配
void searchFiles(const QString &keyword, const QString &indexPath)
{
    qInfo() << "Searching for:" << keyword;
    qInfo() << "Using index at:" << indexPath;
    
    try {
        // Check if index exists
        QDir indexDir(indexPath);
        if (!indexDir.exists()) {
            qCritical() << "Error: Index directory does not exist:" << indexPath;
            qInfo() << "Please run the text-indexer first to create an index.";
            return;
        }
        
        // Open the index
        DirectoryPtr dir = FSDirectory::open(StringUtils::toUnicode(indexPath.toStdString()));
        IndexReaderPtr reader = IndexReader::open(dir);
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        
        // 创建查询
        BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
        
        // 1. 转换关键词为小写，分解为单个字符
        QString lowerKeyword = keyword.toLower();
        String charQuery;
        for (int i = 0; i < lowerKeyword.length(); i++) {
            if (i > 0) charQuery += L" ";
            charQuery += StringUtils::toUnicode(QString(lowerKeyword[i]).toStdString());
        }
        
        // 2. 对字符字段使用标准查询
        QueryParserPtr charParser = newLucene<QueryParser>(
            LuceneVersion::LUCENE_CURRENT, 
            L"chars", 
            newLucene<WhitespaceAnalyzer>());
        QueryPtr charsQuery = charParser->parse(charQuery);
        
        // 3. 原始内容的标准查询
        String keywordStr = StringUtils::toUnicode(keyword.toStdString());
        QueryParserPtr contentParser = newLucene<QueryParser>(
            LuceneVersion::LUCENE_CURRENT, 
            L"content", 
            newLucene<WhitespaceAnalyzer>());
        
        // 对完整关键词进行查询
        try {
            QueryPtr contentQuery = contentParser->parse(keywordStr);
            booleanQuery->add(contentQuery, BooleanClause::SHOULD);
        } catch (ParseException &e) {
            qWarning() << "Content parse error:" << QString::fromStdWString(e.getError());
        }
        
        // 添加字符级查询
        booleanQuery->add(charsQuery, BooleanClause::MUST);
        
        // 执行查询
        Collection<ScoreDocPtr> hits = searcher->search(booleanQuery, 1000)->scoreDocs;
        
        qInfo() << "Found" << hits.size() << "matching documents";
        
        // 显示结果
        for (int i = 0; i < hits.size(); ++i) {
            DocumentPtr doc = searcher->doc(hits[i]->doc);
            String path = doc->get(L"path");
            String content = doc->get(L"content");
            
            QString filePath = QString::fromStdWString(path.c_str());
            QString fileContent = QString::fromStdWString(content.c_str());
            
            // 检查文本是否包含关键词的任何部分
            bool containsAnyPart = false;
            for (int j = 2; j <= keyword.length(); j++) { // 至少考虑2个字符
                for (int k = 0; k <= keyword.length() - j; k++) {
                    QString part = keyword.mid(k, j);
                    if (fileContent.contains(part, Qt::CaseInsensitive)) {
                        containsAnyPart = true;
                        break;
                    }
                }
                if (containsAnyPart) break;
            }
            
            if (containsAnyPart || fileContent.contains(keyword, Qt::CaseInsensitive)) {
                qInfo() << filePath;
                
                // 根据您的需求，你可以选择在控制台输出高亮内容，或者将其保存到文件
                // 这里我们输出部分高亮内容用于演示
                QString highlighted = highlightText(fileContent, keyword);
                if (!highlighted.isEmpty()) {
                    qInfo() << "---- Matches ----";
                    qInfo().noquote() << highlighted;
                    qInfo() << "----------------";
                }
            }
        }
        
        // Close the reader
        reader->close();
        dir->close();
    } catch (LuceneException &e) {
        qCritical() << "Error during searching:" << QString::fromStdWString(e.getError());
    } catch (std::exception &e) {
        qCritical() << "Standard exception:" << e.what();
    } catch (...) {
        qCritical() << "Unknown error during search";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("text-searcher");
    QCoreApplication::setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Search for text in indexed files");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add keyword option
    parser.addPositionalArgument("keyword", "Keyword to search for");
    
    // Parse command line arguments
    parser.process(app);
    
    // Get keyword argument
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "Error: No keyword specified.";
        parser.showHelp(1);
        return 1;
    }
    
    QString keyword = args.first();
    
    // Get home directory for index storage
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString indexPath = homePath + "/text-index";
    
    // Perform search
    searchFiles(keyword, indexPath);
    
    return 0;
}
