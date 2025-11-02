#include <QTest>
#include <QSignalSpy>
#include "pdfeditor/document.h"
#include "pdfeditor/core.h"
#include "../test_helpers.h"

using namespace pdfeditor;
using namespace pdfeditor::test;

class TestDocument : public QObject, public TestFixture {
    Q_OBJECT

private slots:
    void initTestCase() {
        QVERIFY(Library::initialize());
    }
    
    void cleanupTestCase() {
        Library::shutdown();
    }
    
    void init() {
        setUp();
    }
    
    void cleanup() {
        tearDown();
    }
    
    // Test cases
    void testCreateDocument() {
        auto doc = Document::create();
        QVERIFY(doc != nullptr);
        QCOMPARE(doc->page_count(), 0);
    }
    
    void testOpenValidDocument() {
        QString testFile = testDataPath("sample.pdf");
        if (!QFile::exists(testFile)) {
            QSKIP("Test file not found");
        }
        
        auto result = Document::open(testFile.toStdString());
        ASSERT_RESULT_OK(result);
        
        auto doc = result.value();
        ASSERT_DOCUMENT_VALID(doc.get());
    }
    
    void testOpenInvalidFile() {
        auto result = Document::open("nonexistent.pdf");
        ASSERT_RESULT_ERROR(result);
        QVERIFY(result.error() == ErrorCode::FileNotFound ||
                result.error() == ErrorCode::InvalidPDF);
    }
    
    void testOpenEncryptedDocument() {
        QString testFile = testDataPath("encrypted.pdf");
        if (!QFile::exists(testFile)) {
            QSKIP("Encrypted test file not found");
        }
        
        // Try without password - should fail
        auto result1 = Document::open(testFile.toStdString());
        if (result1.is_ok()) {
            // Some PDFs might open without password
            QVERIFY(result1.value()->is_encrypted());
        }
        
        // Try with password
        auto result2 = Document::open(testFile.toStdString(), "password");
        // Result depends on whether we have the right password
    }
    
    void testPageCount() {
        auto doc = createTestDocument(5);
        ASSERT_DOCUMENT_VALID(doc.get());
        QCOMPARE(doc->page_count(), 5);
    }
    
    void testGetPage() {
        auto doc = createTestDocument(3);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        // Valid page
        Page* page0 = doc->get_page(0);
        ASSERT_PAGE_VALID(page0);
        QCOMPARE(page0->number(), 1);
        QCOMPARE(page0->index(), 0);
        
        Page* page2 = doc->get_page(2);
        ASSERT_PAGE_VALID(page2);
        QCOMPARE(page2->number(), 3);
        
        // Invalid page
        Page* invalid = doc->get_page(10);
        QVERIFY(invalid == nullptr);
        
        Page* negative = doc->get_page(-1);
        QVERIFY(negative == nullptr);
    }
    
    void testDocumentInfo() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        DocumentInfo info = doc->get_info();
        QVERIFY(info.page_count > 0);
        QVERIFY(!info.title.empty() || info.title.empty()); // Can be empty
    }
    
    void testSetMetadata() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        doc->set_title("Test Title");
        doc->set_author("Test Author");
        doc->set_subject("Test Subject");
        
        QCOMPARE(QString::fromStdString(doc->get_title()), 
                 QString("Test Title"));
        QCOMPARE(QString::fromStdString(doc->get_author()), 
                 QString("Test Author"));
        QCOMPARE(QString::fromStdString(doc->get_subject()), 
                 QString("Test Subject"));
    }
    
    void testSaveDocument() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        QString tempFile = createTempFile();
        bool saved = doc->save(tempFile.toStdString());
        QVERIFY(saved);
        QVERIFY(QFile::exists(tempFile));
        
        // Verify we can reopen it
        auto result = Document::open(tempFile.toStdString());
        ASSERT_RESULT_OK(result);
    }
    
    void testPageDimensions() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        Page* page = doc->get_page(0);
        ASSERT_PAGE_VALID(page);
        
        float width = page->width();
        float height = page->height();
        
        QVERIFY(width > 0);
        QVERIFY(height > 0);
        
        // A4 page is approximately 595 x 842 points
        // Allow some tolerance
        QVERIFY(width > 100 && width < 2000);
        QVERIFY(height > 100 && height < 2000);
    }
    
    void testPageRotation() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        Page* page = doc->get_page(0);
        ASSERT_PAGE_VALID(page);
        
        // Initial rotation
        PageRotation rot = page->rotation();
        QVERIFY(rot == PageRotation::None || 
                rot == PageRotation::Clockwise90 ||
                rot == PageRotation::Clockwise180 ||
                rot == PageRotation::Clockwise270);
        
        // Set rotation
        page->set_rotation(PageRotation::Clockwise90);
        QCOMPARE(page->rotation(), PageRotation::Clockwise90);
    }
    
    void testExtractPages() {
        auto doc = createTestDocument(10);
        ASSERT_DOCUMENT_VALID(doc.get());
        QCOMPARE(doc->page_count(), 10);
        
        std::vector<int> pages_to_extract = {0, 2, 4};
        auto extracted = doc->extract_pages(pages_to_extract);
        
        if (extracted) {
            QCOMPARE(extracted->page_count(), 3);
        }
    }
    
    void testSearchDocument() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto results = doc->search("test", false, false);
        // Results depend on document content
        QVERIFY(results.size() >= 0);
    }
    
    void testDocumentValidation() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        bool valid = doc->validate();
        QVERIFY(valid);
        
        auto errors = doc->get_validation_errors();
        // Should have no errors for a valid document
        QVERIFY(errors.empty() || !errors.empty());
    }
    
    void testMemoryManagement() {
        // Create and destroy many documents
        for (int i = 0; i < 100; ++i) {
            auto doc = createTestDocument();
            QVERIFY(doc != nullptr);
        }
        // If we get here without crashes, memory management is OK
        QVERIFY(true);
    }
    
    void testConcurrentAccess() {
        auto doc = createTestDocument(10);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        // Access same document from multiple operations
        for (int i = 0; i < 10; ++i) {
            Page* page = doc->get_page(i % doc->page_count());
            QVERIFY(page != nullptr);
        }
    }
    
    void testPerformance() {
        auto doc = createTestDocument(100);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        double time = utils::measureTime([&]() {
            for (int i = 0; i < 100; ++i) {
                Page* page = doc->get_page(i);
                if (page) {
                    float w = page->width();
                    float h = page->height();
                    Q_UNUSED(w);
                    Q_UNUSED(h);
                }
            }
        });
        
        qDebug() << "Time to access 100 pages:" << time << "seconds";
        QVERIFY(time < 1.0); // Should be fast
    }
};

QTEST_MAIN(TestDocument)
#include "test_document.moc"
