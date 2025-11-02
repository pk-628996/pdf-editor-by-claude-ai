#include "pdfviewer.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>
#include <QKeyEvent>
#include <cmath>

// PDFViewer implementation
PDFViewer::PDFViewer(QWidget *parent)
    : QScrollArea(parent)
    , document_(nullptr)
    , renderer_(std::make_unique<pdfeditor::Renderer>())
    , renderQuality_(pdfeditor::RenderQuality::Medium)
    , currentPage_(0)
    , zoom_(1.0)
    , rotation_(0)
    , viewMode_(SinglePage)
    , isSelecting_(false)
    , searchCaseSensitive_(false)
    , currentSearchResult_(-1)
    , maxCacheSize_(10)
{
    setupUi();
}

PDFViewer::~PDFViewer() = default;

void PDFViewer::setupUi() {
    setWidgetResizable(false);
    setAlignment(Qt::AlignCenter);
    
    contentWidget_ = new QWidget(this);
    setWidget(contentWidget_);
    
    // Enable mouse tracking for selection
    setMouseTracking(true);
    contentWidget_->setMouseTracking(true);
}

void PDFViewer::setDocument(pdfeditor::Document* document) {
    document_ = document;
    currentPage_ = 0;
    pageCache_.clear();
    
    // Clear old page widgets
    for (auto* widget : pageWidgets_) {
        delete widget;
    }
    pageWidgets_.clear();
    
    if (document_) {
        // Create page widgets based on view mode
        updateLayout();
        renderCurrentPage();
        emit documentChanged();
    }
}

void PDFViewer::goToPage(int pageIndex) {
    if (!document_) return;
    
    if (pageIndex < 0) pageIndex = 0;
    if (pageIndex >= document_->page_count()) {
        pageIndex = document_->page_count() - 1;
    }
    
    if (currentPage_ != pageIndex) {
        currentPage_ = pageIndex;
        renderCurrentPage();
        emit pageChanged(currentPage_);
    }
}

void PDFViewer::nextPage() {
    goToPage(currentPage_ + 1);
}

void PDFViewer::previousPage() {
    goToPage(currentPage_ - 1);
}

void PDFViewer::firstPage() {
    goToPage(0);
}

void PDFViewer::lastPage() {
    if (document_) {
        goToPage(document_->page_count() - 1);
    }
}

int PDFViewer::pageCount() const {
    return document_ ? document_->page_count() : 0;
}

void PDFViewer::zoomIn() {
    setZoom(zoom_ * 1.25);
}

void PDFViewer::zoomOut() {
    setZoom(zoom_ / 1.25);
}

void PDFViewer::zoomFit() {
    setZoom(calculateZoomToFit());
}

void PDFViewer::zoomFitWidth() {
    setZoom(calculateZoomToFitWidth());
}

void PDFViewer::zoomFitHeight() {
    setZoom(calculateZoomToFitHeight());
}

void PDFViewer::zoomActual() {
    setZoom(1.0);
}

void PDFViewer::setZoom(double zoom) {
    if (zoom < 0.1) zoom = 0.1;
    if (zoom > 10.0) zoom = 10.0;
    
    if (std::abs(zoom_ - zoom) > 0.01) {
        zoom_ = zoom;
        
        // Update all page widgets
        for (auto* widget : pageWidgets_) {
            widget->setZoom(zoom_);
        }
        
        updateLayout();
        renderCurrentPage();
        emit zoomChanged(zoom_);
    }
}

void PDFViewer::rotateClockwise() {
    setRotation((rotation_ + 90) % 360);
}

void PDFViewer::rotateCounterclockwise() {
    setRotation((rotation_ + 270) % 360);
}

void PDFViewer::setRotation(int degrees) {
    degrees = degrees % 360;
    if (degrees < 0) degrees += 360;
    
    if (rotation_ != degrees) {
        rotation_ = degrees;
        
        // Update all page widgets
        for (auto* widget : pageWidgets_) {
            widget->setRotation(rotation_);
        }
        
        renderCurrentPage();
        emit rotationChanged(rotation_);
    }
}

void PDFViewer::setViewMode(ViewMode mode) {
    if (viewMode_ != mode) {
        viewMode_ = mode;
        updateLayout();
        renderCurrentPage();
    }
}

void PDFViewer::setRenderQuality(pdfeditor::RenderQuality quality) {
    if (renderQuality_ != quality) {
        renderQuality_ = quality;
        pageCache_.clear();
        renderCurrentPage();
    }
}

void PDFViewer::clearSelection() {
    selection_ = QRect();
    isSelecting_ = false;
    emit selectionChanged(selection_);
}

void PDFViewer::findText(const QString& text, bool caseSensitive) {
    searchText_ = text;
    searchCaseSensitive_ = caseSensitive;
    searchResults_.clear();
    currentSearchResult_ = -1;
    
    if (text.isEmpty() || !document_) return;
    
    // Search all pages
    for (int i = 0; i < document_->page_count(); ++i) {
        pdfeditor::Page* page = document_->get_page(i);
        if (page) {
            // Get page text
            std::string pageText = page->get_text();
            
            // TODO: Find all occurrences and their positions
            // This is simplified - real implementation needs text layout info
        }
    }
}

void PDFViewer::findNext() {
    if (searchResults_.empty()) return;
    
    currentSearchResult_ = (currentSearchResult_ + 1) % searchResults_.size();
    // TODO: Scroll to and highlight result
}

void PDFViewer::findPrevious() {
    if (searchResults_.empty()) return;
    
    currentSearchResult_--;
    if (currentSearchResult_ < 0) {
        currentSearchResult_ = searchResults_.size() - 1;
    }
    // TODO: Scroll to and highlight result
}

void PDFViewer::clearSearch() {
    searchText_.clear();
    searchResults_.clear();
    currentSearchResult_ = -1;
}

void PDFViewer::renderCurrentPage() {
    if (!document_ || currentPage_ < 0 || currentPage_ >= document_->page_count()) {
        return;
    }
    
    // Check cache first
    if (pageCache_.contains(currentPage_)) {
        // Update display from cache
        if (currentPage_ < pageWidgets_.size()) {
            // Convert QPixmap to QImage for display
            pageWidgets_[currentPage_]->setImage(pageCache_[currentPage_].toImage());
        }
        return;
    }
    
    // Render page
    pdfeditor::Page* page = document_->get_page(currentPage_);
    if (!page) return;
    
    pdfeditor::RenderOptions options;
    options.dpi = 72.0f * zoom_;
    options.anti_aliasing = pdfeditor::AntiAliasing::All;
    
    auto result = renderer_->render_page(page, options);
    if (result.is_ok()) {
        auto buffer = result.value();
        
        // Convert to QImage
        QImage image(
            buffer->data(),
            buffer->width(),
            buffer->height(),
            buffer->stride(),
            QImage::Format_RGB888
        );
        
        // Deep copy
        QImage imageCopy = image.copy();
        
        // Update widget
        if (currentPage_ < pageWidgets_.size()) {
            pageWidgets_[currentPage_]->setImage(imageCopy);
        }
        
        // Cache the rendered page
        if (pageCache_.size() >= maxCacheSize_) {
            // Remove oldest entry
            auto it = pageCache_.begin();
            pageCache_.erase(it);
        }
        pageCache_[currentPage_] = QPixmap::fromImage(imageCopy);
    }
}

void PDFViewer::renderVisiblePages() {
    // TODO: Render only visible pages for performance
    renderCurrentPage();
}

void PDFViewer::updateLayout() {
    if (!document_) return;
    
    QVBoxLayout* layout = new QVBoxLayout(contentWidget_);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);
    
    switch (viewMode_) {
        case SinglePage: {
            // One page at a time
            if (pageWidgets_.empty()) {
                auto* widget = new PageWidget(currentPage_, contentWidget_);
                widget->setZoom(zoom_);
                widget->setRotation(rotation_);
                pageWidgets_.push_back(widget);
                layout->addWidget(widget);
            }
            break;
        }
        
        case Continuous: {
            // All pages in vertical scroll
            for (int i = 0; i < document_->page_count(); ++i) {
                auto* widget = new PageWidget(i, contentWidget_);
                widget->setZoom(zoom_);
                widget->setRotation(rotation_);
                pageWidgets_.push_back(widget);
                layout->addWidget(widget);
            }
            break;
        }
        
        case FacingPages: {
            // Two pages side by side
            // TODO: Implement facing pages layout
            break;
        }
    }
    
    contentWidget_->setLayout(layout);
}

void PDFViewer::updateScrollBars() {
    // Update scroll bar ranges based on content size
}

double PDFViewer::calculateZoomToFit() {
    if (!document_ || !viewport()) return 1.0;
    
    pdfeditor::Page* page = document_->get_page(currentPage_);
    if (!page) return 1.0;
    
    float pageWidth = page->width();
    float pageHeight = page->height();
    
    QSize viewportSize = viewport()->size();
    
    double zoomX = viewportSize.width() / pageWidth;
    double zoomY = viewportSize.height() / pageHeight;
    
    return std::min(zoomX, zoomY) * 0.95; // 95% to leave margin
}

double PDFViewer::calculateZoomToFitWidth() {
    if (!document_ || !viewport()) return 1.0;
    
    pdfeditor::Page* page = document_->get_page(currentPage_);
    if (!page) return 1.0;
    
    float pageWidth = page->width();
    QSize viewportSize = viewport()->size();
    
    return (viewportSize.width() / pageWidth) * 0.95;
}

double PDFViewer::calculateZoomToFitHeight() {
    if (!document_ || !viewport()) return 1.0;
    
    pdfeditor::Page* page = document_->get_page(currentPage_);
    if (!page) return 1.0;
    
    float pageHeight = page->height();
    QSize viewportSize = viewport()->size();
    
    return (viewportSize.height() / pageHeight) * 0.95;
}

void PDFViewer::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        // Normal scrolling
        QScrollArea::wheelEvent(event);
    }
}

void PDFViewer::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_PageDown:
        case Qt::Key_Space:
            nextPage();
            event->accept();
            break;
            
        case Qt::Key_PageUp:
        case Qt::Key_Backspace:
            previousPage();
            event->accept();
            break;
            
        case Qt::Key_Home:
            if (event->modifiers() & Qt::ControlModifier) {
                firstPage();
            }
            event->accept();
            break;
            
        case Qt::Key_End:
            if (event->modifiers() & Qt::ControlModifier) {
                lastPage();
            }
            event->accept();
            break;
            
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomIn();
            }
            event->accept();
            break;
            
        case Qt::Key_Minus:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomOut();
            }
            event->accept();
            break;
            
        case Qt::Key_0:
            if (event->modifiers() & Qt::ControlModifier) {
                zoomActual();
            }
            event->accept();
            break;
            
        default:
            QScrollArea::keyPressEvent(event);
    }
}

void PDFViewer::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);
    // TODO: Handle zoom-to-fit on resize
}

void PDFViewer::onPageRendered(int pageIndex, const QImage& image) {
    if (pageIndex < pageWidgets_.size()) {
        pageWidgets_[pageIndex]->setImage(image);
    }
}

void PDFViewer::onRenderError(int pageIndex, const QString& error) {
    // TODO: Display error message
}

// PageWidget implementation
PageWidget::PageWidget(int pageIndex, QWidget *parent)
    : QWidget(parent)
    , pageIndex_(pageIndex)
    , zoom_(1.0)
    , rotation_(0)
    , isSelecting_(false)
{
    setMouseTracking(true);
}

void PageWidget::setImage(const QImage& image) {
    image_ = image;
    pixmap_ = QPixmap::fromImage(image_);
    
    // Update widget size
    setFixedSize(image_.size());
    update();
}

void PageWidget::setZoom(double zoom) {
    zoom_ = zoom;
    update();
}

void PageWidget::setRotation(int degrees) {
    rotation_ = degrees;
    update();
}

QSize PageWidget::pageSize() const {
    return image_.size();
}

void PageWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    if (!pixmap_.isNull()) {
        // Draw the page
        painter.drawPixmap(0, 0, pixmap_);
        
        // Draw selection if active
        if (isSelecting_ && !selection_.isEmpty()) {
            painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
            painter.setBrush(QBrush(QColor(0, 0, 255, 50)));
            painter.drawRect(selection_);
        }
    } else {
        // Draw placeholder
        painter.fillRect(rect(), Qt::white);
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, 
            QString("Page %1").arg(pageIndex_ + 1));
    }
}

void PageWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isSelecting_ = true;
        selectionStart_ = event->pos();
        selection_ = QRect(selectionStart_, selectionStart_);
        emit clicked(pageIndex_);
    }
    QWidget::mousePressEvent(event);
}

void PageWidget::mouseMoveEvent(QMouseEvent *event) {
    if (isSelecting_) {
        selection_ = QRect(selectionStart_, event->pos()).normalized();
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void PageWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && isSelecting_) {
        isSelecting_ = false;
        if (!selection_.isEmpty()) {
            emit selectionChanged(selection_);
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void PageWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked(pageIndex_);
    }
    QWidget::mouseDoubleClickEvent(event);
}
