#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <memory>
#include "pdfeditor/document.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PDFViewer;
class BookmarksPanel;
class ThumbnailsPanel;
class AnnotationToolbar;
class MetadataDialog;
class SearchDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Open file
    bool openFile(const QString& fileName);
    
    // Get current document
    pdfeditor::Document* currentDocument() const { return document_.get(); }
    
protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // File menu
    void onFileNew();
    void onFileOpen();
    void onFileOpenRecent();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();
    void onFileProperties();
    void onFilePrint();
    void onFileExit();
    
    // Edit menu
    void onEditUndo();
    void onEditRedo();
    void onEditCut();
    void onEditCopy();
    void onEditPaste();
    void onEditSelectAll();
    void onEditFind();
    void onEditReplace();
    void onEditPreferences();
    
    // View menu
    void onViewZoomIn();
    void onViewZoomOut();
    void onViewZoomFit();
    void onViewZoomActual();
    void onViewZoomCustom();
    void onViewRotateClockwise();
    void onViewRotateCounterclockwise();
    void onViewSinglePage();
    void onViewContinuous();
    void onViewFacingPages();
    void onViewFullScreen();
    void onViewShowBookmarks();
    void onViewShowThumbnails();
    void onViewShowToolbars();
    
    // Document menu
    void onDocumentInsertPages();
    void onDocumentExtractPages();
    void onDocumentDeletePages();
    void onDocumentRotatePages();
    void onDocumentCropPages();
    void onDocumentSplitDocument();
    void onDocumentMergeDocuments();
    void onDocumentCompare();
    
    // Tools menu
    void onToolsAddText();
    void onToolsAddImage();
    void onToolsAddAnnotation();
    void onToolsAddBookmark();
    void onToolsEditMetadata();
    void onToolsOCR();
    void onToolsRedact();
    void onToolsOptimize();
    void onToolsSign();
    void onToolsEncrypt();
    void onToolsDecrypt();
    
    // Help menu
    void onHelpContents();
    void onHelpAbout();
    
    // Document events
    void onDocumentChanged();
    void onPageChanged(int pageIndex);
    void onSelectionChanged();
    
    // Viewer events
    void onZoomChanged(double zoom);
    void onRotationChanged(int rotation);
    
private:
    void setupUi();
    void createActions();
    void createMenus();
    void createToolbars();
    void createDockWidgets();
    void createStatusBar();
    void loadSettings();
    void saveSettings();
    void updateRecentFiles(const QString& fileName);
    void updateWindowTitle();
    void updateActions();
    
    bool saveDocument();
    bool saveDocumentAs();
    bool closeDocument();
    bool maybeSave();
    
    Ui::MainWindow *ui;
    
    // Core components
    PDFViewer *pdfViewer_;
    std::unique_ptr<pdfeditor::Document> document_;
    QString currentFile_;
    bool isModified_;
    
    // Dock widgets
    QDockWidget *bookmarksDock_;
    QDockWidget *thumbnailsDock_;
    BookmarksPanel *bookmarksPanel_;
    ThumbnailsPanel *thumbnailsPanel_;
    
    // Toolbars
    QToolBar *fileToolBar_;
    QToolBar *editToolBar_;
    QToolBar *viewToolBar_;
    AnnotationToolbar *annotationToolBar_;
    
    // Dialogs
    MetadataDialog *metadataDialog_;
    SearchDialog *searchDialog_;
    
    // Menus
    QMenu *fileMenu_;
    QMenu *editMenu_;
    QMenu *viewMenu_;
    QMenu *documentMenu_;
    QMenu *toolsMenu_;
    QMenu *helpMenu_;
    QMenu *recentFilesMenu_;
    
    // Actions - File
    QAction *actionNew_;
    QAction *actionOpen_;
    QAction *actionSave_;
    QAction *actionSaveAs_;
    QAction *actionClose_;
    QAction *actionProperties_;
    QAction *actionPrint_;
    QAction *actionExit_;
    
    // Actions - Edit
    QAction *actionUndo_;
    QAction *actionRedo_;
    QAction *actionCut_;
    QAction *actionCopy_;
    QAction *actionPaste_;
    QAction *actionSelectAll_;
    QAction *actionFind_;
    QAction *actionReplace_;
    QAction *actionPreferences_;
    
    // Actions - View
    QAction *actionZoomIn_;
    QAction *actionZoomOut_;
    QAction *actionZoomFit_;
    QAction *actionZoomActual_;
    QAction *actionRotateClockwise_;
    QAction *actionRotateCounterclockwise_;
    QAction *actionSinglePage_;
    QAction *actionContinuous_;
    QAction *actionFacingPages_;
    QAction *actionFullScreen_;
    QAction *actionShowBookmarks_;
    QAction *actionShowThumbnails_;
    
    // Actions - Tools
    QAction *actionAddText_;
    QAction *actionAddImage_;
    QAction *actionAddAnnotation_;
    QAction *actionAddBookmark_;
    QAction *actionEditMetadata_;
    QAction *actionOCR_;
    QAction *actionRedact_;
    QAction *actionOptimize_;
    QAction *actionSign_;
    QAction *actionEncrypt_;
    QAction *actionDecrypt_;
    
    // Recent files
    static const int MaxRecentFiles = 10;
    QAction *recentFileActions_[MaxRecentFiles];
    QStringList recentFiles_;
};
