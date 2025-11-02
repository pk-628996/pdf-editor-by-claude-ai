#include "mainwindow.h"
#include "pdfeditor/core.h"
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    // Enable high DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setOrganizationName("PDFEditor");
    app.setOrganizationDomain("pdfeditor.org");
    app.setApplicationName("PDFEditor");
    app.setApplicationVersion(QString::fromStdString(pdfeditor::Library::get_version()));
    app.setApplicationDisplayName("PDFEditor");
    
    // Set application style
    QSettings settings;
    QString style = settings.value("ui/style", "Fusion").toString();
    app.setStyle(QStyleFactory::create(style));
    
    // Initialize PDF library
    if (!pdfeditor::Library::initialize()) {
        QMessageBox::critical(nullptr, 
            "Initialization Error",
            "Failed to initialize PDF library. The application will now exit.");
        return 1;
    }
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("PDFEditor - A production-ready PDF editor");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add file argument
    parser.addPositionalArgument("file", "PDF file to open", "[file]");
    
    // Process arguments
    parser.process(app);
    
    // Create main window
    MainWindow mainWindow;
    mainWindow.show();
    
    // Open file if specified
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        QString fileName = args.first();
        QFileInfo fileInfo(fileName);
        
        if (fileInfo.exists()) {
            mainWindow.openFile(fileName);
        } else {
            QMessageBox::warning(&mainWindow,
                "File Not Found",
                QString("Cannot find file: %1").arg(fileName));
        }
    }
    
    // Run event loop
    int result = app.exec();
    
    // Cleanup
    pdfeditor::Library::shutdown();
    
    return result;
}
