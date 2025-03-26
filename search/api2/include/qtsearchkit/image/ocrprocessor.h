#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <QFuture>
#include <QVariantMap>
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT OcrOptions {
public:
    OcrOptions();
    
    // 语言设置
    void setLanguage(const QString& language);
    QString language() const;
    
    // OCR 精度模式 (高速/标准/高精度)
    enum AccuracyMode {
        Fast,
        Standard,
        Accurate
    };
    void setAccuracyMode(AccuracyMode mode);
    AccuracyMode accuracyMode() const;
    
    // 图像预处理选项
    void setPreprocessImage(bool preprocess);
    bool preprocessImage() const;
    
    // 区域检测
    void setDetectRegions(bool detect);
    bool detectRegions() const;
    
    // 输出格式设置
    void setOutputFormat(const QString& format); // text, json, hocr等
    QString outputFormat() const;
    
    // 特定OCR引擎的额外选项
    void setEngineOptions(const QVariantMap& options);
    QVariantMap engineOptions() const;
    
private:
    QString m_language = "eng";
    AccuracyMode m_accuracyMode = Standard;
    bool m_preprocessImage = true;
    bool m_detectRegions = true;
    QString m_outputFormat = "text";
    QVariantMap m_engineOptions;
};

// OCR处理器接口
class QTSEARCHKIT_EXPORT OcrProcessor : public QObject {
    Q_OBJECT
public:
    explicit OcrProcessor(QObject* parent = nullptr);
    virtual ~OcrProcessor() override;
    
    // 设置OCR选项
    virtual void setOptions(const OcrOptions& options);
    virtual OcrOptions options() const;
    
    // 处理图像得到文本
    virtual QFuture<QString> processImage(const QImage& image) = 0;
    virtual QFuture<QString> processImageFile(const QString& imagePath) = 0;
    
    // 处理图像后返回带位置信息的文本
    struct TextRegion {
        QString text;
        QRect rect;
        float confidence;
    };
    virtual QFuture<QList<TextRegion>> processImageWithRegions(const QImage& image) = 0;
    
    // 引擎信息
    virtual QString engineName() const = 0;
    virtual QString engineVersion() const = 0;
    virtual QStringList supportedLanguages() const = 0;
    
signals:
    void processingProgress(int percent);
    void processingError(const QString& error);
    
protected:
    OcrOptions m_options;
};

// Tesseract实现示例
class QTSEARCHKIT_EXPORT TesseractOcrProcessor : public OcrProcessor {
    Q_OBJECT
public:
    explicit TesseractOcrProcessor(QObject* parent = nullptr);
    ~TesseractOcrProcessor() override;
    
    QFuture<QString> processImage(const QImage& image) override;
    QFuture<QString> processImageFile(const QString& imagePath) override;
    QFuture<QList<TextRegion>> processImageWithRegions(const QImage& image) override;
    
    QString engineName() const override;
    QString engineVersion() const override;
    QStringList supportedLanguages() const override;
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 