#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QToolBar>
#include <QAction>
#include <QPixmap>
#include <QTemporaryFile>
#include <QMovie>
#include <QResizeEvent>
#include <memory>

#include <poppler/qt6/poppler-qt6.h>

class OfficeConverter;

class OfficePreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OfficePreviewWidget(QWidget *parent = nullptr);
    ~OfficePreviewWidget();

    void previewFile(const QString &filePath);

signals:
    void previewProgress(int percentage);
    void previewError(const QString &error);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onConversionFinished(const QString &pdfPath);
    void onConversionError(const QString &error);
    void onConversionProgress(int percentage);
    
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    void onZoomSliderChanged(int value);
    void onPageChanged();
    void previousPage();
    void nextPage();
    void onPageSpinBoxChanged(int page);

private:
    void setupUI();
    void setupToolBar();
    void loadPdf(const QString &pdfPath);
    void renderCurrentPage();
    void updatePageInfo();
    void updateZoomInfo();
    void showLoadingAnimation();
    void hideLoadingAnimation();
    
    // UI组件
    QVBoxLayout *m_mainLayout;
    QToolBar *m_toolBar;
    QScrollArea *m_scrollArea;
    QLabel *m_imageLabel;
    QLabel *m_loadingLabel;
    QMovie *m_loadingMovie;
    
    // 工具栏控件
    QPushButton *m_prevPageButton;
    QPushButton *m_nextPageButton;
    QSpinBox *m_pageSpinBox;
    QLabel *m_pageCountLabel;
    QSlider *m_zoomSlider;
    QLabel *m_zoomLabel;
    QPushButton *m_zoomFitButton;
    QPushButton *m_zoomActualButton;
    
    // PDF相关 - 使用智能指针
    std::unique_ptr<Poppler::Document> m_pdfDocument;
    int m_currentPage;
    double m_scaleFactor;
    QPixmap m_currentPixmap;
    
    // 转换器
    OfficeConverter *m_converter;
    QTemporaryFile *m_tempPdfFile;
    
    // 常量
    static const double MIN_SCALE;
    static const double MAX_SCALE;
    static const double SCALE_STEP;
}; 