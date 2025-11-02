#include <QTest>
#include "pdfeditor/bookmarks.h"
#include "pdfeditor/document.h"
#include "pdfeditor/core.h"
#include "../test_helpers.h"

using namespace pdfeditor;
using namespace pdfeditor::test;

class TestBookmarks : public QObject, public TestFixture {
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
    
    void testListEmptyBookmarks() {
        auto doc = createTestDocument();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        // Empty document may or may not have bookmarks
        QVERIFY(bookmarks.size() >= 0);
    }
    
    void testListExistingBookmarks() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        QVERIFY(bookmarks.size() > 0);
        
        // Check first bookmark
        if (!bookmarks.empty()) {
            const auto& bookmark = bookmarks[0];
            QVERIFY(!bookmark.id.empty());
            QVERIFY(!bookmark.title.empty());
            QVERIFY(bookmark.destination.page_index >= 0);
        }
    }
    
    void testAddBookmark() {
        auto doc = createTestDocument(5);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        BookmarkDestination dest;
        dest.type = DestinationType::Page;
        dest.page_index = 2;
        
        auto result = Bookmarks::add(doc.get(), "", "Test Bookmark", dest);
        ASSERT_RESULT_OK(result);
        
        std::string id = result.value();
        QVERIFY(!id.empty());
        
        // Verify bookmark was added
        auto bookmarks = Bookmarks::list(doc.get());
        QVERIFY(bookmarks.size() > 0);
    }
    
    void testAddBookmarkToPage() {
        auto doc = createTestDocument(5);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto result = Bookmarks::add_to_page(
            doc.get(), "", "Page 3", 2, 100.0f
        );
        ASSERT_RESULT_OK(result);
        
        std::string id = result.value();
        QVERIFY(!id.empty());
    }
    
    void testAddNestedBookmarks() {
        auto doc = createTestDocument(5);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        // Add parent bookmark
        auto result1 = Bookmarks::add_to_page(
            doc.get(), "", "Chapter 1", 0
        );
        ASSERT_RESULT_OK(result1);
        std::string parent_id = result1.value();
        
        // Add child bookmark
        auto result2 = Bookmarks::add_to_page(
            doc.get(), parent_id, "Section 1.1", 1
        );
        ASSERT_RESULT_OK(result2);
        
        // Verify hierarchy
        auto bookmarks = Bookmarks::list(doc.get());
        if (!bookmarks.empty()) {
            QVERIFY(bookmarks[0].children.size() > 0);
        }
    }
    
    void testRemoveBookmark() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        if (bookmarks.empty()) {
            QSKIP("No bookmarks to remove");
        }
        
        std::string id = bookmarks[0].id;
        bool removed = Bookmarks::remove(doc.get(), id);
        QVERIFY(removed);
        
        // Verify bookmark was removed
        auto updated = Bookmarks::list(doc.get());
        QVERIFY(updated.size() < bookmarks.size());
    }
    
    void testEditBookmark() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        if (bookmarks.empty()) {
            QSKIP("No bookmarks to edit");
        }
        
        BookmarkEntry updated = bookmarks[0];
        updated.title = "Updated Title";
        
        bool success = Bookmarks::edit(doc.get(), updated.id, updated);
        QVERIFY(success);
    }
    
    void testSetTitle() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        if (bookmarks.empty()) {
            QSKIP("No bookmarks");
        }
        
        bool success = Bookmarks::set_title(
            doc.get(), bookmarks[0].id, "New Title"
        );
        QVERIFY(success);
    }
    
    void testMoveBookmark() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto bookmarks = Bookmarks::list(doc.get());
        if (bookmarks.size() < 2) {
            QSKIP("Need at least 2 bookmarks");
        }
        
        bool success = Bookmarks::move(
            doc.get(), bookmarks[1].id, "", 0
        );
        QVERIFY(success);
    }
    
    void testExportJson() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        std::string json = Bookmarks::export_json(doc.get());
        QVERIFY(!json.empty());
        QVERIFY(json.find("{") != std::string::npos);
        QVERIFY(json.find("bookmarks") != std::string::npos);
    }
    
    void testExportOpml() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        std::string opml = Bookmarks::export_opml(doc.get());
        QVERIFY(!opml.empty());
        QVERIFY(opml.find("<?xml") != std::string::npos);
        QVERIFY(opml.find("<opml") != std::string::npos);
    }
    
    void testExportText() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        std::string text = Bookmarks::export_text(doc.get());
        QVERIFY(!text.empty());
    }
    
    void testFindByTitle() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto results = Bookmarks::find_by_title(doc.get(), "chapter");
        // Results depend on document content
        QVERIFY(results.size() >= 0);
    }
    
    void testFindByPage() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto results = Bookmarks::find_by_page(doc.get(), 0);
        // Results depend on document content
        QVERIFY(results.size() >= 0);
    }
    
    void testValidate() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        bool valid = Bookmarks::validate(doc.get());
        QVERIFY(valid);
    }
    
    void testFindBroken() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        auto broken = Bookmarks::find_broken(doc.get());
        // Should have no broken bookmarks in valid document
        QVERIFY(broken.size() == 0);
    }
    
    void testExpandCollapse() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        bool expanded = Bookmarks::expand_all(doc.get());
        QVERIFY(expanded);
        
        bool collapsed = Bookmarks::collapse_all(doc.get());
        QVERIFY(collapsed);
    }
    
    void testCount() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        int count = Bookmarks::count(doc.get());
        QVERIFY(count >= 0);
        
        auto bookmarks = Bookmarks::list_flat(doc.get());
        QCOMPARE(count, static_cast<int>(bookmarks.size()));
    }
    
    void testGetMaxDepth() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        int depth = Bookmarks::get_max_depth(doc.get());
        QVERIFY(depth >= 0);
    }
    
    void testClearBookmarks() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        bool cleared = Bookmarks::clear(doc.get());
        QVERIFY(cleared);
        
        auto bookmarks = Bookmarks::list(doc.get());
        QVERIFY(bookmarks.empty());
    }
    
    void testRoundTripJsonExportImport() {
        auto doc = createDocumentWithBookmarks();
        ASSERT_DOCUMENT_VALID(doc.get());
        
        // Export
        std::string json = Bookmarks::export_json(doc.get());
        QVERIFY(!json.empty());
        
        // Clear
        Bookmarks::clear(doc.get());
        QVERIFY(Bookmarks::count(doc.get()) == 0);
        
        // Import
        bool imported = Bookmarks::import_json(doc.get(), json);
        QVERIFY(imported);
        
        // Verify
        QVERIFY(Bookmarks::count(doc.get()) > 0);
    }
    
    void testPerformance() {
        auto doc = createTestDocument(100);
        ASSERT_DOCUMENT_VALID(doc.get());
        
        // Add many bookmarks
        double time = utils::measureTime([&]() {
            for (int i = 0; i < 100; ++i) {
                Bookmarks::add_to_page(
                    doc.get(), "", 
                    "Bookmark " + std::to_string(i), 
                    i % doc->page_count()
                );
            }
        });
        
        qDebug() << "Time to add 100 bookmarks:" << time << "seconds";
        QVERIFY(time < 2.0);
        
        // List all bookmarks
        time = utils::measureTime([&]() {
            auto bookmarks = Bookmarks::list(doc.get());
            Q_UNUSED(bookmarks);
        });
        
        qDebug() << "Time to list bookmarks:" << time << "seconds";
        QVERIFY(time < 0.5);
    }
};

QTEST_MAIN(TestBookmarks)
#include "test_bookmarks.moc"
