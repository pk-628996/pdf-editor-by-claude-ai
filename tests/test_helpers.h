#pragma once

#include "pdfeditor/core.h"
#include "pdfeditor/document.h"
#include <QString>
#include <QTemporaryFile>
#include <memory>

namespace pdfeditor {
namespace test {

// Test fixture base class
class TestFixture {
public:
    TestFixture();
    virtual ~TestFixture();
    
    // Setup/teardown
    virtual void setUp();
    virtual void tearDown();
    
protected:
    // Helper to get test data path
    QString testDataPath(const QString& filename) const;
    
    // Helper to create temporary file
    QString createTempFile(const QString& suffix = ".pdf");
    
    // Create a simple test PDF
    std::unique_ptr<Document> createTestDocument(int pageCount = 1);
    
    // Create PDF with bookmarks
    std::unique_ptr<Document> createDocumentWithBookmarks();
    
    // Create PDF with metadata
    std::unique_ptr<Document> createDocumentWithMetadata();
    
    // Create PDF with annotations
    std::unique_ptr<Document> createDocumentWithAnnotations();
    
    // Verify document is valid
    bool verifyDocument(Document* doc);
    
    // Compare two files
    bool compareFiles(const QString& file1, const QString& file2);
    
    // Temporary files created during test
    std::vector<std::unique_ptr<QTemporaryFile>> tempFiles_;
};

// Assertions
#define ASSERT_DOCUMENT_VALID(doc) \
    QVERIFY(doc != nullptr); \
    QVERIFY(doc->page_count() > 0)

#define ASSERT_PAGE_VALID(page) \
    QVERIFY(page != nullptr); \
    QVERIFY(page->width() > 0); \
    QVERIFY(page->height() > 0)

#define ASSERT_RESULT_OK(result) \
    QVERIFY(result.is_ok()); \
    if (!result.is_ok()) { \
        qDebug() << "Error:" << QString::fromStdString(result.error_message()); \
    }

#define ASSERT_RESULT_ERROR(result) \
    QVERIFY(result.is_error())

// Mock objects
class MockProgressCallback {
public:
    MockProgressCallback();
    
    bool operator()(int current, int total, const std::string& message);
    
    int callCount() const { return callCount_; }
    int lastCurrent() const { return lastCurrent_; }
    int lastTotal() const { return lastTotal_; }
    std::string lastMessage() const { return lastMessage_; }
    
private:
    int callCount_;
    int lastCurrent_;
    int lastTotal_;
    std::string lastMessage_;
};

// Utility functions
namespace utils {
    // Generate random string
    std::string randomString(int length);
    
    // Generate random PDF content
    std::vector<uint8_t> generateRandomPdfContent();
    
    // Create minimal valid PDF
    std::vector<uint8_t> createMinimalPdf();
    
    // Measure execution time
    template<typename Func>
    double measureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        return diff.count();
    }
}

} // namespace test
} // namespace pdfeditor
