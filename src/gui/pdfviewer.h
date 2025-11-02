#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>
#include "pdfeditor/document.h"
#include "pdfeditor/renderer.h"

class PageWidget;

class PDFViewer : public QScrollArea {
    Q_OBJECT

public:
    enum ViewMode {
        SinglePage,
        Continuous,
        FacingPages
    };
    
    explicit PDFViewer(QWidget *parent = nullptr);
    ~PDFViewer();
    
    // Document management
    void setDocument(pdfeditor::Document* document);
    pdfeditor::Document* document() const { return document_; }
    
    // Page navigation
    void goToPage(int pageIndex);
    void nextPage();
    void previousPage();
    void firstPage();
    void lastPage();
    
    int currentPage() const { return currentPage_; }
    int pageCount() const;
    
    // Zoom operations
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomFitWidth();
    void zoomFitHeight();
    void zoomActual();
    void setZoom(double zoom);
    double zoom() const { return zoom_; }
    
    // Rotation
    void rotateClockwise();
    void rotateCounterclockwise();
    void setRotation(int degrees);
    int rotation() const { return rotation_; }
    
    // View mode
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const { return viewMode_; }
    
    // Rendering quality
    void setRenderQuality(pdfeditor::RenderQuality quality);
    pdfeditor::RenderQuality renderQuality() const { return renderQuality_; }
    
    // Selection
    QRect selection() const { return selection_; }
    void clearSelection();
    
    // Search
    void findText(const QString& text, bool caseSensitive = false);
    void findNext();
    void findPrevious();
    void clearSearch();
    
signals:
    void pageChanged(int pageIndex);
    void zoomChanged(double zoom);
    void rotationChanged(int degrees);
    void selectionChanged(const QRect& selection);
    void documentChanged();
    void linkClicked(const QString& url);
    
protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
private slots:
    void onPageRendered(int pageIndex, const QImage& image);
    void onRenderError(int pageIndex, const QString& error);
    
private:
    void setupUi();
    void renderCurrentPage();
    void renderVisiblePages();
    void updateLayout();
    void updateScrollBars();
    
    // Calculate zoom to fit
    double calculateZoomToFit();
    double calculateZoomToFitWidth();
    double calculateZoomToFitHeight();
    
    // Document and rendering
    pdfeditor::Document* document_;
    std::unique_ptr<pdfeditor::Renderer> renderer_;
    pdfeditor::RenderQuality renderQuality_;
    
    // View state
    int currentPage_;
    double zoom_;
    int rotation_;
    ViewMode viewMode_;
    
    // Page widgets
    QWidget* contentWidget_;
    std::vector<PageWidget*> pageWidgets_;
    
    // Selection
    QRect selection_;
    bool isSelecting_;
    QPoint selectionStart_;
    
    // Search
    QString searchText_;
    bool searchCaseSensitive_;
    std::vector<QRect> searchResults_;
    int currentSearchResult_;
    
    // Cache
    QMap<int, QPixmap> pageCache_;
    int maxCacheSize_;
};

// Internal widget for rendering a single page
class PageWidget : public QWidget {
    Q_OBJECT

public:
    explicit PageWidget(int pageIndex, QWidget *parent = nullptr);
    
    void setImage(const QImage& image);
    void setZoom(double zoom);
    void setRotation(int degrees);
    
    int pageIndex() const { return pageIndex_; }
    QSize pageSize() const;
    
signals:
    void clicked(int pageIndex);
    void doubleClicked(int pageIndex);
    void selectionChanged(const QRect& selection);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
private:
    int pageIndex_;
    QImage image_;
    QPixmap pixmap_;
    double zoom_;
    int rotation_;
    
    // Selection
    bool isSelecting_;
    QPoint selectionStart_;
    QRect selection_;
};
