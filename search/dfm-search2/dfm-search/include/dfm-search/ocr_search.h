#ifndef DFM_OCR_SEARCH_H
#define DFM_OCR_SEARCH_H

#include <memory>
#include <vector>
#include <functional>
#include <future>

#include <QString>
#include <QStringList>
#include <QImage>
#include <QRect>
#include <QVariant>

#include "search_engine.h" // 引入SearchResult等相关结构

namespace DFM {
namespace Search {

/**
 * @brief OCR引擎类型枚举
 */
enum class OCREngineType {
    Default,     ///< 默认引擎
    Tesseract,   ///< Tesseract OCR引擎
    External,    ///< 外部引擎
    Custom       ///< 自定义引擎
};

/**
 * @brief OCR语言类型枚举
 */
enum class OCRLanguage {
    Auto,        ///< 自动检测
    Chinese,     ///< 中文
    English,     ///< 英文
    Japanese,    ///< 日文
    Korean,      ///< 韩文
    Custom       ///< 自定义语言
};

/**
 * @brief OCR区域信息
 */
class OCRRegion {
public:
    QString text;           ///< 识别出的文本
    QRect boundingBox;      ///< 文本区域
    double confidence{0.0}; ///< 识别置信度 (0.0-1.0)
    QString language;       ///< 识别出的语言
};

/**
 * @brief OCR结果类
 */
class OCRResult {
public:
    QString filePath;                ///< 图像文件路径
    std::vector<OCRRegion> regions;  ///< 识别出的文本区域
    QString fullText;                ///< 完整文本（所有区域合并）
    double averageConfidence{0.0};   ///< 平均置信度
    double processingTime{0.0};      ///< 处理时间（秒）
    
    // 元数据
    QVariantMap metadata;
};

/**
 * @brief OCR配置类
 */
class OCRConfig {
public:
    OCREngineType engineType{OCREngineType::Default}; ///< OCR引擎类型
    OCRLanguage language{OCRLanguage::Auto};          ///< 识别语言
    QString customLanguage;                           ///< 自定义语言代码
    QString enginePath;                               ///< 引擎路径
    int dpi{300};                                     ///< 图像DPI
    bool enhanceImage{true};                          ///< 是否增强图像
    double minConfidence{0.5};                        ///< 最小置信度
    int maxThreads{2};                                ///< 最大线程数
    
    // 自定义配置选项
    QVariantMap customOptions;
};

/**
 * @brief OCR搜索引擎类
 */
class OCRSearchEngine : public QObject {
    Q_OBJECT
    
public:
    // PIMPL模式
    class Impl;
    
    OCRSearchEngine();
    virtual ~OCRSearchEngine();
    
    /**
     * @brief 初始化OCR引擎
     * @param config OCR配置
     * @return 是否初始化成功
     */
    bool initialize(const OCRConfig& config = OCRConfig());
    
    /**
     * @brief 对单个图像进行OCR识别
     * @param imagePath 图像路径
     * @return OCR识别结果
     */
    OCRResult recognizeImage(const QString& imagePath);
    
    /**
     * @brief 对内存中的图像进行OCR识别
     * @param image 图像数据
     * @param imageName 图像名称（可选）
     * @return OCR识别结果
     */
    OCRResult recognizeImage(const QImage& image, const QString& imageName = QString());
    
    /**
     * @brief 异步识别图像
     * @param imagePath 图像路径
     * @return 异步任务对象
     */
    std::future<OCRResult> recognizeImageAsync(const QString& imagePath);
    
    /**
     * @brief 对多个图像进行批量OCR识别
     * @param imagePaths 图像路径列表
     * @param maxParallel 最大并行数
     * @return OCR识别结果列表
     */
    std::vector<OCRResult> batchRecognize(const QStringList& imagePaths, int maxParallel = 2);
    
    /**
     * @brief 搜索包含指定文本的图像
     * @param text 搜索文本
     * @param searchPaths 搜索路径
     * @param options 搜索选项
     * @return 搜索结果
     */
    SearchResult searchImagesWithText(const QString& text, 
                                      const QStringList& searchPaths, 
                                      const SearchOptions& options = SearchOptions());
    
    /**
     * @brief 停止当前搜索任务
     */
    void stopSearch();
    
    /**
     * @brief 获取OCR引擎版本信息
     * @return 版本信息
     */
    QString engineVersion() const;
    
    /**
     * @brief 获取支持的语言列表
     * @return 支持的语言列表
     */
    QStringList supportedLanguages() const;
    
signals:
    void recognitionCompleted(const DFM::Search::OCRResult& result);
    void batchProgress(int current, int total);
    void searchProgress(int percentage);
    void searchCompleted(const DFM::Search::SearchResult& results);
    
private:
    std::unique_ptr<Impl> d;
};

/**
 * @brief 将OCREngineType转换为字符串
 */
QString ocrEngineTypeToString(OCREngineType type);

/**
 * @brief 将OCRLanguage转换为字符串
 */
QString ocrLanguageToString(OCRLanguage language);

} // namespace Search
} // namespace DFM

#endif // DFM_OCR_SEARCH_H 