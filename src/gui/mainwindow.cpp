#include "mainwindow.h"
#include "pdfviewer.h"
#include "bookmarkspanel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , document_(nullptr)
    , isModified_(false)
    , metadataDialog_(nullptr)
    , searchDialog_(nullptr)
{
    setupUi();
    createActions();
    createMenus();
    createToolbars();
    createDockWidgets();
    createStatusBar();
    loadSettings();
    updateActions();
    
    setAcceptDrops(true);
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::setupUi() {
    setWindowTitle("PDFEditor");
    resize(1200, 800);
    
    // Create central PDF viewer
    pdfViewer_ = new PDFViewer(this);
    setCentralWidget(pdfViewer_);
    
    // Connect viewer signals
    connect(pdfViewer_, &PDFViewer::pageChanged,
            this, &MainWindow::onPageChanged);
    connect(pdfViewer_, &PDFViewer::zoomChanged,
            this, &MainWindow::onZoomChanged);
    connect(pdfViewer_, &PDFViewer::rotationChanged,
            this, &MainWindow::onRotationChanged);
    connect(pdfViewer_, &PDFViewer::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(pdfViewer_, &PDFViewer::documentChanged,
            this, &MainWindow::onDocumentChanged);
}

void MainWindow::createActions() {
    // File actions
    actionNew_ = new QAction(QIcon(":/icons/new.png"), tr("&New"), this);
    actionNew_->setShortcuts(QKeySequence::New);
    actionNew_->setStatusTip(tr("Create a new PDF document"));
    connect(actionNew_, &QAction::triggered, this, &MainWindow::onFileNew);
    
    actionOpen_ = new QAction(QIcon(":/icons/open.png"), tr("&Open..."), this);
    actionOpen_->setShortcuts(QKeySequence::Open);
    actionOpen_->setStatusTip(tr("Open an existing PDF document"));
    connect(actionOpen_, &QAction::triggered, this, &MainWindow::onFileOpen);
    
    actionSave_ = new QAction(QIcon(":/icons/save.png"), tr("&Save"), this);
    actionSave_->setShortcuts(QKeySequence::Save);
    actionSave_->setStatusTip(tr("Save the document"));
    connect(actionSave_, &QAction::triggered, this, &MainWindow::onFileSave);
    
    actionSaveAs_ = new QAction(tr("Save &As..."), this);
    actionSaveAs_->setShortcuts(QKeySequence::SaveAs);
    actionSaveAs_->setStatusTip(tr("Save the document with a new name"));
    connect(actionSaveAs_, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    
    actionClose_ = new QAction(tr("&Close"), this);
    actionClose_->setShortcuts(QKeySequence::Close);
    connect(actionClose_, &QAction::triggered, this, &MainWindow::onFileClose);
    
    actionPrint_ = new QAction(QIcon(":/icons/print.png"), tr("&Print..."), this);
    actionPrint_->setShortcuts(QKeySequence::Print);
    connect(actionPrint_, &QAction::triggered, this, &MainWindow::onFilePrint);
    
    actionExit_ = new QAction(tr("E&xit"), this);
    actionExit_->setShortcuts(QKeySequence::Quit);
    connect(actionExit_, &QAction::triggered, this, &MainWindow::onFileExit);
    
    // Edit actions
    actionUndo_ = new QAction(QIcon(":/icons/undo.png"), tr("&Undo"), this);
    actionUndo_->setShortcuts(QKeySequence::Undo);
    connect(actionUndo_, &QAction::triggered, this, &MainWindow::onEditUndo);
    
    actionRedo_ = new QAction(QIcon(":/icons/redo.png"), tr("&Redo"), this);
    actionRedo_->setShortcuts(QKeySequence::Redo);
    connect(actionRedo_, &QAction::triggered, this, &MainWindow::onEditRedo);
    
    actionCopy_ = new QAction(QIcon(":/icons/copy.png"), tr("&Copy"), this);
    actionCopy_->setShortcuts(QKeySequence::Copy);
    connect(actionCopy_, &QAction::triggered, this, &MainWindow::onEditCopy);
    
    actionFind_ = new QAction(QIcon(":/icons/find.png"), tr("&Find..."), this);
    actionFind_->setShortcuts(QKeySequence::Find);
    connect(actionFind_, &QAction::triggered, this, &MainWindow::onEditFind);
    
    // View actions
    actionZoomIn_ = new QAction(QIcon(":/icons/zoom-in.png"), tr("Zoom &In"), this);
    actionZoomIn_->setShortcut(QKeySequence::ZoomIn);
    connect(actionZoomIn_, &QAction::triggered, pdfViewer_, &PDFViewer::zoomIn);
    
    actionZoomOut_ = new QAction(QIcon(":/icons/zoom-out.png"), tr("Zoom &Out"), this);
    actionZoomOut_->setShortcut(QKeySequence::ZoomOut);
    connect(actionZoomOut_, &QAction::triggered, pdfViewer_, &PDFViewer::zoomOut);
    
    actionZoomFit_ = new QAction(tr("Fit Page"), this);
    connect(actionZoomFit_, &QAction::triggered, pdfViewer_, &PDFViewer::zoomFit);
    
    actionZoomActual_ = new QAction(tr("Actual Size"), this);
    actionZoomActual_->setShortcut(QKeySequence(tr("Ctrl+0")));
    connect(actionZoomActual_, &QAction::triggered, pdfViewer_, &PDFViewer::zoomActual);
    
    actionRotateClockwise_ = new QAction(QIcon(":/icons/rotate-right.png"), 
                                         tr("Rotate &Clockwise"), this);
    connect(actionRotateClockwise_, &QAction::triggered,
            pdfViewer_, &PDFViewer::rotateClockwise);
    
    actionRotateCounterclockwise_ = new QAction(QIcon(":/icons/rotate-left.png"),
                                                 tr("Rotate &Counterclockwise"), this);
    connect(actionRotateCounterclockwise_, &QAction::triggered,
            pdfViewer_, &PDFViewer::rotateCounterclockwise);
    
    actionFullScreen_ = new QAction(tr("&Full Screen"), this);
    actionFullScreen_->setShortcut(QKeySequence::FullScreen);
    actionFullScreen_->setCheckable(true);
    connect(actionFullScreen_, &QAction::triggered, [this](bool checked) {
        if (checked) {
            showFullScreen();
        } else {
            showNormal();
        }
    });
    
    actionShowBookmarks_ = new QAction(tr("Show &Bookmarks"), this);
    actionShowBookmarks_->setCheckable(true);
    actionShowBookmarks_->setChecked(true);
    connect(actionShowBookmarks_, &QAction::triggered, [this](bool checked) {
        bookmarksDock_->setVisible(checked);
    });
    
    actionShowThumbnails_ = new QAction(tr("Show &Thumbnails"), this);
    actionShowThumbnails_->setCheckable(true);
    actionShowThumbnails_->setChecked(true);
    connect(actionShowThumbnails_, &QAction::triggered, [this](bool checked) {
        thumbnailsDock_->setVisible(checked);
    });
    
    // Tools actions
    actionAddBookmark_ = new QAction(QIcon(":/icons/bookmark-add.png"),
                                     tr("Add &Bookmark"), this);
    connect(actionAddBookmark_, &QAction::triggered, this, &MainWindow::onToolsAddBookmark);
    
    actionEditMetadata_ = new QAction(tr("Edit &Metadata..."), this);
    connect(actionEditMetadata_, &QAction::triggered, this, &MainWindow::onToolsEditMetadata);
}

void MainWindow::createMenus() {
    // File menu
    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(actionNew_);
    fileMenu_->addAction(actionOpen_);
    recentFilesMenu_ = fileMenu_->addMenu(tr("Open &Recent"));
    fileMenu_->addSeparator();
    fileMenu_->addAction(actionSave_);
    fileMenu_->addAction(actionSaveAs_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(actionClose_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(actionPrint_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(actionExit_);
    
    // Edit menu
    editMenu_ = menuBar()->addMenu(tr("&Edit"));
    editMenu_->addAction(actionUndo_);
    editMenu_->addAction(actionRedo_);
    editMenu_->addSeparator();
    editMenu_->addAction(actionCopy_);
    editMenu_->addSeparator();
    editMenu_->addAction(actionFind_);
    
    // View menu
    viewMenu_ = menuBar()->addMenu(tr("&View"));
    viewMenu_->addAction(actionZoomIn_);
    viewMenu_->addAction(actionZoomOut_);
    viewMenu_->addAction(actionZoomFit_);
    viewMenu_->addAction(actionZoomActual_);
    viewMenu_->addSeparator();
    viewMenu_->addAction(actionRotateClockwise_);
    viewMenu_->addAction(actionRotateCounterclockwise_);
    viewMenu_->addSeparator();
    viewMenu_->addAction(actionFullScreen_);
    viewMenu_->addSeparator();
    viewMenu_->addAction(actionShowBookmarks_);
    viewMenu_->addAction(actionShowThumbnails_);
    
    // Tools menu
    toolsMenu_ = menuBar()->addMenu(tr("&Tools"));
    toolsMenu_->addAction(actionAddBookmark_);
    toolsMenu_->addAction(actionEditMetadata_);
    
    // Help menu
    helpMenu_ = menuBar()->addMenu(tr("&Help"));
    helpMenu_->addAction(tr("&Contents"), this, &MainWindow::onHelpContents);
    helpMenu_->addSeparator();
    helpMenu_->addAction(tr("&About"), this, &MainWindow::onHelpAbout);
}

void MainWindow::createToolbars() {
    // File toolbar
    fileToolBar_ = addToolBar(tr("File"));
    fileToolBar_->setObjectName("FileToolBar");
    fileToolBar_->addAction(actionOpen_);
    fileToolBar_->addAction(actionSave_);
    fileToolBar_->addAction(actionPrint_);
    
    // Edit toolbar
    editToolBar_ = addToolBar(tr("Edit"));
    editToolBar_->setObjectName("EditToolBar");
    editToolBar_->addAction(actionUndo_);
    editToolBar_->addAction(actionRedo_);
    editToolBar_->addAction(actionCopy_);
    editToolBar_->addAction(actionFind_);
    
    // View toolbar
    viewToolBar_ = addToolBar(tr("View"));
    viewToolBar_->setObjectName("ViewToolBar");
    viewToolBar_->addAction(actionZoomIn_);
    viewToolBar_->addAction(actionZoomOut_);
    viewToolBar_->addAction(actionZoomFit_);
    viewToolBar_->addAction(actionRotateClockwise_);
    viewToolBar_->addAction(actionRotateCounterclockwise_);
}

void MainWindow::createDockWidgets() {
    // Bookmarks dock
    bookmarksDock_ = new QDockWidget(tr("Bookmarks"), this);
    bookmarksDock_->setObjectName("BookmarksDock");
    bookmarksPanel_ = new BookmarksPanel(this);
    bookmarksDock_->setWidget(bookmarksPanel_);
    addDockWidget(Qt::LeftDockWidgetArea, bookmarksDock_);
    
    // Thumbnails dock
    thumbnailsDock_ = new QDockWidget(tr("Thumbnails"), this);
    thumbnailsDock_->setObjectName("ThumbnailsDock");
    thumbnailsPanel_ = new ThumbnailsPanel(this);
    thumbnailsDock_->setWidget(thumbnailsPanel_);
    addDockWidget(Qt::LeftDockWidgetArea, thumbnailsDock_);
    
    // Tab the dock widgets
    tabifyDockWidget(bookmarksDock_, thumbnailsDock_);
    bookmarksDock_->raise();
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

bool MainWindow::openFile(const QString& fileName) {
    if (fileName.isEmpty()) {
        return false;
    }
    
    // Try to open the document
    auto result = pdfeditor::Document::open(fileName.toStdString());
    
    if (!result.is_ok()) {
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to open document:\n%1\n\nError: %2")
                .arg(fileName)
                .arg(QString::fromStdString(result.error_message())));
        return false;
    }
    
    // Close current document if any
    closeDocument();
    
    // Set new document
    document_ = result.value();
    currentFile_ = fileName;
    isModified_ = false;
    
    // Update viewer
    pdfViewer_->setDocument(document_.get());
    
    // Update UI
    updateWindowTitle();
    updateRecentFiles(fileName);
    updateActions();
    
    // Update panels
    bookmarksPanel_->setDocument(document_.get());
    thumbnailsPanel_->setDocument(document_.get());
    
    statusBar()->showMessage(tr("Opened: %1").arg(fileName), 3000);
    
    return true;
}

// Continue in next part...
