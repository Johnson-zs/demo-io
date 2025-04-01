#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>

using namespace Lucene;

// Helper function to read file content
QString readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

// 使用字符级索引来支持部分匹配
void indexDirectory(const QString &dirPath, const QString &indexPath)
{
    qInfo() << "Indexing directory:" << dirPath;
    qInfo() << "Index will be stored at:" << indexPath;
    
    // Create index directory if it doesn't exist
    QDir indexDir(indexPath);
    if (!indexDir.exists()) {
        indexDir.mkpath(".");
    }
    
    try {
        // 使用WhitespaceAnalyzer而不是StandardAnalyzer
        // WhitespaceAnalyzer只按空白符分词，保留更多原始信息
        AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
        
        // 配置索引选项
        IndexWriterPtr writer = newLucene<IndexWriter>(
            FSDirectory::open(StringUtils::toUnicode(indexPath.toStdString())),
            analyzer,
            true,   // Overwrite existing index
            IndexWriter::MaxFieldLengthUNLIMITED);  // 不限制字段长度
        
        // Find all txt files
        QDir dir(dirPath);
        QStringList txtFiles = dir.entryList(QStringList() << "*.txt", QDir::Files);
        
        qInfo() << "Found" << txtFiles.size() << "txt files to index";
        
        // Process each file
        for (const QString &fileName : txtFiles) {
            QString filePath = dir.absoluteFilePath(fileName);
            QString content = readTextFile(filePath);
            
            if (!content.isEmpty()) {
                // Create a Lucene document
                DocumentPtr doc = newLucene<Document>();
                
                // Add file path as a field
                doc->add(newLucene<Field>(L"path", 
                         StringUtils::toUnicode(filePath.toStdString()), 
                         Field::STORE_YES, 
                         Field::INDEX_NOT_ANALYZED));
                
                // 存储完整的原始内容，用于高亮显示
                doc->add(newLucene<Field>(L"content", 
                         StringUtils::toUnicode(content.toStdString()), 
                         Field::STORE_YES,  
                         Field::INDEX_ANALYZED));
                
                // 创建一个单字符索引字段，支持部分匹配
                // 将所有内容转换为小写，按字符拆分并重新组合
                QString lowerContent = content.toLower();
                
                // 添加一个拆分为单字符的字段，用于支持部分匹配
                String charContent;
                for (int i = 0; i < lowerContent.length(); i++) {
                    if (i > 0) charContent += L" ";
                    charContent += StringUtils::toUnicode(QString(lowerContent[i]).toStdString());
                }
                
                doc->add(newLucene<Field>(L"chars", 
                         charContent,
                         Field::STORE_NO, 
                         Field::INDEX_ANALYZED));
                
                // Add the document to the index
                writer->addDocument(doc);
                qInfo() << "Indexed file:" << filePath;
            }
        }
        
        // Optimize and close the writer
        writer->optimize();
        writer->close();
        
        qInfo() << "Indexing completed successfully";
    } catch (LuceneException &e) {
        qCritical() << "Error during indexing:" << QString::fromStdWString(e.getError());
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("text-indexer");
    QCoreApplication::setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Index text files in a directory");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add directory option
    parser.addPositionalArgument("directory", "Directory containing txt files to index");
    
    // Parse command line arguments
    parser.process(app);
    
    // Get directory argument
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "Error: No directory specified.";
        parser.showHelp(1);
        return 1;
    }
    
    QString dirPath = args.first();
    QFileInfo dirInfo(dirPath);
    if (!dirInfo.exists() || !dirInfo.isDir()) {
        qCritical() << "Error: The specified path is not a valid directory:" << dirPath;
        return 1;
    }
    
    // Get home directory for index storage
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString indexPath = homePath + "/text-index";
    
    // Create index
    indexDirectory(dirPath, indexPath);
    
    return 0;
}
