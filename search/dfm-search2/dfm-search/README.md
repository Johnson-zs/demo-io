# DFM-Search: Modern C++/Qt6 Linux Desktop Search Library

DFM-Search is a modern search library designed for Linux desktop environments, built on C++17 and Qt6 framework. It provides a comprehensive set of search features, including filename search, content search, application search, and image OCR search.

## Features

- **Multiple search types**: Supports filename, file content, application, and image OCR search
- **Multiple search mechanisms**: Supports both indexed and real-time search mechanisms
- **Rich search options**: Supports case sensitivity, pinyin search, boolean logic, fuzzy matching, regular expressions, and more
- **Full search control**: Provides start, pause, resume, and stop search control functions
- **Flexible result handling**: Supports asynchronous result callbacks and pagination
- **Plugin system**: Supports extending search functionality through plugins
- **Modern C++ design**: Utilizes C++17 features and avoids virtual functions to ensure ABI compatibility
- **Qt integration**: Deeply integrated with Qt's signal-slot system

## Build Requirements

- C++17 compatible compiler (e.g. GCC 7+ or Clang 5+)
- Qt 6.2+
- CMake 3.16+

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Installation

```bash
sudo make install
```

## Usage Examples

### Basic Filename Search

```cpp
#include <dfm-search/dfm_search.h>

// Initialize the search library
DFM::Search::initialize();

// Create a filename search engine
auto engine = DFM::Search::createFilenameSearchEngine();

// Set the callback function
engine->setResultCallback([](const DFM::Search::SearchResult& results) {
    for (const auto& item : results.items) {
        if (auto fileItem = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(item)) {
            qDebug() << "Found:" << fileItem->filePath;
        }
    }
});

// Create a search query
DFM::Search::SearchQuery query = DFM::Search::SearchQuery::createFilenameQuery("*.txt");
query.options.searchPaths = {QDir::homePath()};
query.options.caseSensitive = false;

// Start the search
engine->search(query);

// Wait for the search to complete...

// Shutdown the search library
DFM::Search::shutdown();
```

### Indexed Content Search

```cpp
#include <dfm-search/dfm_search.h>

// Initialize the search library
DFM::Search::initialize();

// Create an index engine
auto indexEngine = DFM::Search::IndexEngineFactory::instance().createEngine(
    DFM::Search::IndexType::FileContent);

// Configure the index
DFM::Search::IndexConfig indexConfig;
indexConfig.indexPaths = {QDir::homePath() + "/Documents"};
indexConfig.excludePaths = {QDir::homePath() + "/Documents/tmp"};
indexConfig.filePatterns = {"*.txt", "*.md", "*.cpp"};

// Initialize the index engine
indexEngine->initialize(indexConfig);

// Wait for the indexing to complete...

// Create a content search engine
auto searchEngine = DFM::Search::createContentSearchEngine(true);

// Set the callback
searchEngine->setResultCallback([](const DFM::Search::SearchResult& results) {
    for (const auto& item : results.items) {
        if (auto contentItem = std::dynamic_pointer_cast<DFM::Search::ContentResultItem>(item)) {
            qDebug() << "Found in:" << contentItem->filePath;
            qDebug() << "Line:" << contentItem->lineNumber;
            qDebug() << "Content:" << contentItem->matchedContent;
        }
    }
});

// Create a search query
DFM::Search::SearchQuery query = DFM::Search::SearchQuery::createContentQuery("hello world");
query.mechanism = DFM::Search::SearchMechanism::Indexed;

// Start the search
searchEngine->search(query);

// Shutdown the search library
DFM::Search::shutdown();
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Documentation

Full API documentation can be generated using Doxygen:

```bash
cd docs
doxygen Doxyfile
```

## Contributing

Contributions to the code, reporting issues, or suggesting improvements are welcome. Please see the CONTRIBUTING.md file for details.