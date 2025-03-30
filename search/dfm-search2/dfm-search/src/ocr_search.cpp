#include <dfm-search/ocr_search.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QImage>

namespace DFM {
namespace Search {

// OCRSearchEngine私有实现
class OCRSearchEngine::Impl {
public:
    Impl(OCRSearchEngine* q)
        : q_ptr(q)
        , initialized(false)
    {
    }
    
    // 引用外部类
    OCRSearchEngine* q_ptr;
    
    // 状态相关
    bool initialized;
    OCRIndexConfig config;
    std::vector<OCRInfo> ocrIndex;
    
    // 互斥锁
    mutable QMutex mutex;
    
    // 初始化OCR索引 - 实际实现中应该从数据库加载
    void initializeOCRIndex() {
        // 模拟添加一些OCR索引项
        ocrIndex.clear();
        
        // 添加测试数据
        OCRInfo sample1;
        sample1.setFilePath("/path/to/image1.jpg");
        sample1.setText("这是一个测试图片文本示例");
        sample1.metadata().insert("creationDate", "2023-09-01");
        ocrIndex.push_back(sample1);
        
        OCRInfo sample2;
        sample2.setFilePath("/path/to/image2.png");
        sample2.setText("这是另一个包含OCR文本的图片");
        sample2.metadata().insert("creationDate", "2023-10-15");
        ocrIndex.push_back(sample2);
    }
    
    // 简单的OCR文本搜索
    std::vector<OCRInfo> searchText(const QString& query, int maxResults) const {
        if (query.isEmpty()) {
            return {};
        }
        
        std::vector<OCRInfo> results;
        QString queryLower = query.toLower();
        
        for (const auto& item : ocrIndex) {
            // 简单字符串匹配
            if (item.text().toLower().contains(queryLower)) {
                results.push_back(item);
                
                if (static_cast<int>(results.size()) >= maxResults) {
                    break;
                }
            }
        }
        
        return results;
    }
};

// OCRSearchEngine构造函数
OCRSearchEngine::OCRSearchEngine()
    : d(new Impl(this))
{
}

// OCRSearchEngine析构函数
OCRSearchEngine::~OCRSearchEngine() = default;

// 初始化
bool OCRSearchEngine::initialize(const OCRIndexConfig& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        qWarning() << "OCR搜索引擎已经初始化";
        return true; // 已经初始化，视为成功
    }
    
    d->config = config;
    
    // 初始化OCR索引
    d->initializeOCRIndex();
    
    d->initialized = true;
    emit indexUpdated();
    
    return true;
}

// 更新索引
bool OCRSearchEngine::update()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return false;
    }
    
    // 重新初始化OCR索引
    d->initializeOCRIndex();
    
    emit indexUpdated();
    
    return true;
}

// 重建索引
bool OCRSearchEngine::rebuild()
{
    return update();
}

// 搜索OCR文本
std::vector<OCRInfo> OCRSearchEngine::search(const QString& query, int maxResults) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return {};
    }
    
    auto results = d->searchText(query, maxResults);
    
    emit const_cast<OCRSearchEngine*>(this)->searchCompleted(results);
    
    return results;
}

// 处理图像
QString OCRSearchEngine::processImage(const QString& imagePath) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return QString();
    }
    
    // 简单模拟OCR处理，实际实现中应该调用OCR库
    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "无法加载图像:" << imagePath;
        return QString();
    }
    
    // 模拟处理耗时
    QString extractedText = "这是从图像中提取的OCR文本示例。";
    
    // 将结果添加到索引
    OCRInfo newInfo;
    newInfo.setFilePath(imagePath);
    newInfo.setText(extractedText);
    newInfo.metadata().insert("processedDate", QDateTime::currentDateTime().toString());
    
    const_cast<OCRSearchEngine*>(this)->addToIndex(newInfo);
    
    return extractedText;
}

// 添加到索引
bool OCRSearchEngine::addToIndex(const OCRInfo& ocrInfo)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return false;
    }
    
    // 检查是否已存在该文件
    auto it = std::find_if(d->ocrIndex.begin(), d->ocrIndex.end(), 
                          [&](const OCRInfo& info) { 
                              return info.filePath() == ocrInfo.filePath(); 
                          });
    
    if (it != d->ocrIndex.end()) {
        // 已存在，更新
        *it = ocrInfo;
    } else {
        // 不存在，添加新项
        d->ocrIndex.push_back(ocrInfo);
    }
    
    emit indexUpdated();
    
    return true;
}

// 从索引中移除
bool OCRSearchEngine::removeFromIndex(const QString& filePath)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return false;
    }
    
    auto it = std::remove_if(d->ocrIndex.begin(), d->ocrIndex.end(),
                            [&](const OCRInfo& info) {
                                return info.filePath() == filePath;
                            });
    
    if (it != d->ocrIndex.end()) {
        d->ocrIndex.erase(it, d->ocrIndex.end());
        emit indexUpdated();
        return true;
    }
    
    return false;
}

// 清空索引
bool OCRSearchEngine::clearIndex()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "OCR搜索引擎未初始化";
        return false;
    }
    
    d->ocrIndex.clear();
    emit indexUpdated();
    
    return true;
}

// 获取索引中的项数
int OCRSearchEngine::getItemCount() const
{
    QMutexLocker locker(&d->mutex);
    return static_cast<int>(d->ocrIndex.size());
}

} // namespace Search
} // namespace DFM 