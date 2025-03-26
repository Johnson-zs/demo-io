#pragma once

#include <QObject>
#include <QString>
#include "../index/indexmanager.h"
#include "ocrprocessor.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 为IndexManager添加图片OCR支持的扩展
class QTSEARCHKIT_EXPORT ImageIndexExtension : public QObject {
    Q_OBJECT
public:
    explicit ImageIndexExtension(IndexManager* manager, QObject* parent = nullptr);
    
    // 设置OCR处理器
    void setOcrProcessor(std::shared_ptr<OcrProcessor> processor);
    std::shared_ptr<OcrProcessor> ocrProcessor() const;
    
    // 设置要处理的图片类型
    void setSupportedImageFormats(const QStringList& formats);
    QStringList supportedImageFormats() const;
    
    // OCR索引设置
    void setExtractImageText(bool extract);
    bool extractImageText() const;
    
    // 图片索引批量设置
    void setMaxImageSize(const QSize& size);
    QSize maxImageSize() const;
    
    // 图片特征提取 (用于相似图像搜索)
    void setExtractImageFeatures(bool extract);
    bool extractImageFeatures() const;
    
signals:
    void imageProcessed(const QString& path, bool success);
    void ocrError(const QString& path, const QString& error);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 