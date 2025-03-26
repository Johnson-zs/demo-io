#include <QApplication>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QIcon>
#include <QProcess>
#include <qsearch/search_manager.h>
#include <qsearch/search_query.h>

class AppLauncher : public QWidget {
    Q_OBJECT
public:
    AppLauncher() {
        // 设置UI
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        searchEdit = new QLineEdit(this);
        searchEdit->setPlaceholderText("搜索应用...");
        layout->addWidget(searchEdit);
        
        resultsList = new QListWidget(this);
        resultsList->setIconSize(QSize(32, 32));
        layout->addWidget(resultsList);
        
        // 连接信号
        connect(searchEdit, &QLineEdit::textChanged, this, &AppLauncher::onSearchTextChanged);
        connect(resultsList, &QListWidget::itemActivated, this, &AppLauncher::onItemActivated);
        
        // 设置窗口
        setWindowTitle("应用启动器");
        resize(400, 500);
        
        // 初始展示所有应用
        searchApplications("");
    }

private slots:
    void onSearchTextChanged(const QString& text) {
        searchApplications(text);
    }
    
    void onItemActivated(QListWidgetItem* item) {
        // 获取应用执行命令
        QString exec = item->data(Qt::UserRole).toString();
        
        // 处理命令行(移除参数占位符如%f, %u等)
        QStringList parts = exec.split(' ');
        QString program = parts.first();
        
        QStringList args;
        for (int i = 1; i < parts.size(); i++) {
            QString arg = parts[i];
            if (!arg.startsWith('%')) {
                args << arg;
            }
        }
        
        // 启动应用程序
        QProcess::startDetached(program, args);
    }
    
    void onSearchCompleted(int searchId) {
        // 获取搜索结果
        QSearch::SearchResult results = searchManager.searchResults(searchId);
        
        // 显示结果
        resultsList->clear();
        
        for (const QSearch::ResultItem& item : results.items()) {
            QListWidgetItem* listItem = new QListWidgetItem(resultsList);
            
            // 设置图标
            QString iconPath = item.metadata("icon").toString();
            if (!iconPath.isEmpty()) {
                if (QFile::exists(iconPath)) {
                    listItem->setIcon(QIcon(iconPath));
                } else {
                    // 尝试从系统图标主题加载
                    listItem->setIcon(QIcon::fromTheme(iconPath, QIcon::fromTheme("application-x-executable")));
                }
            } else {
                listItem->setIcon(QIcon::fromTheme("application-x-executable"));
            }
            
            // 设置显示文本
            QString comment = item.metadata("comment").toString();
            listItem->setText(item.name + (comment.isEmpty() ? "" : "\n" + comment));
            
            // 存储执行命令
            listItem->setData(Qt::UserRole, item.metadata("exec"));
            
            // 设置工具提示
            listItem->setToolTip(item.name + "\n" + comment);
        }
    }
    
private:
    void searchApplications(const QString& searchText) {
        // 构建应用搜索查询
        QSearch::SearchQuery query(searchText);
        query.setType(QSearch::QueryType::Application);
        
        // 开始搜索
        int searchId = searchManager.startSearch(query);
        
        // 连接搜索完成信号
        connect(&searchManager, &QSearch::SearchManager::searchCompleted,
                this, &AppLauncher::onSearchCompleted, Qt::UniqueConnection);
    }
    
    QLineEdit* searchEdit;
    QListWidget* resultsList;
    QSearch::SearchManager searchManager;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    AppLauncher launcher;
    launcher.show();
    
    return app.exec();
}

#include "main.moc" 