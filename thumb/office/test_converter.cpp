#include "src/OfficeConverter.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

class TestConverter : public QObject
{
    Q_OBJECT

public:
    TestConverter(const QString &inputFile) : m_inputFile(inputFile)
    {
        m_converter = new OfficeConverter(this);
        connect(m_converter, &OfficeConverter::conversionFinished,
                this, &TestConverter::onConversionFinished);
        connect(m_converter, &OfficeConverter::conversionError,
                this, &TestConverter::onConversionError);
        connect(m_converter, &OfficeConverter::conversionProgress,
                this, &TestConverter::onConversionProgress);
    }

    void start()
    {
        QString outputFile = "/tmp/test_output.pdf";
        qDebug() << "开始测试转换:" << m_inputFile << "=>" << outputFile;
        m_converter->convertToPdf(m_inputFile, outputFile);
    }

private slots:
    void onConversionFinished(const QString &pdfPath)
    {
        qDebug() << "转换成功:" << pdfPath;
        QCoreApplication::quit();
    }

    void onConversionError(const QString &error)
    {
        qDebug() << "转换失败:" << error;
        QCoreApplication::quit();
    }

    void onConversionProgress(int percentage)
    {
        qDebug() << "转换进度:" << percentage << "%";
    }

private:
    QString m_inputFile;
    OfficeConverter *m_converter;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    if (argc < 2) {
        qDebug() << "用法:" << argv[0] << "<Office文件路径>";
        return 1;
    }
    
    QString inputFile = argv[1];
    TestConverter test(inputFile);
    
    QTimer::singleShot(0, &test, &TestConverter::start);
    
    return app.exec();
}

#include "test_converter.moc" 