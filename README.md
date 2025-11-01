# PDFEditor

A production-ready, open-source PDF editor aiming for feature parity with Adobe Acrobat Pro DC.

![License](https://img.shields.io/badge/license-AGPL--3.0-blue.svg)
![Build Status](https://img.shields.io/github/actions/workflow/status/username/PDFEditor/cross-build.yml)
![Version](https://img.shields.io/badge/version-1.0.0-green.svg)

## ✨ Features

### Core Functionality
- **📄 Viewing**: Fast, accurate rendering with single, continuous, and facing page modes
- **✏️ Editing**: Text and image editing with intelligent content reflow
- **📝 Annotations**: Highlights, underlines, sticky notes, stamps, and freehand drawing
- **📋 Forms**: Create, fill, and edit PDF forms (AcroForms support)
- **🔐 Signatures**: Digital signatures with PKCS#7/PAdES standards
- **🔍 OCR**: Multi-language text recognition powered by Tesseract
- **🚫 Redaction**: Permanent content removal with metadata sanitization

### Advanced Features
- **📑 Bookmarks**: Complete outline/chapter management (add, edit, reorder, nest)
- **📊 Metadata**: Full Info dictionary and XMP packet editing
- **🔒 Security**: AES-256 encryption, password protection, and permissions
- **📚 Organization**: Merge, split, reorder, extract, and replace pages
- **💾 Export/Import**: Convert to/from Word, images (PNG/JPEG), text, and PDF/A
- **♿ Accessibility**: PDF tagging, reading order, and alt text support
- **⚡ Optimization**: Linearization, compression, and image downsampling

## 🚀 Quick Start

### Prerequisites

**Required:**
- CMake 3.20 or higher
- Qt 6.5 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
- MuPDF 1.24 or higher
- OpenSSL 3.0 or higher

**Optional:**
- Tesseract 5.3+ (for OCR features)
- Git (for cloning and submodules)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/username/PDFEditor.git
cd PDFEditor

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel

# Install (optional)
sudo cmake --install .
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_GUI` | ON | Build graphical user interface |
| `BUILD_CLI` | ON | Build command-line interface |
| `BUILD_TESTS` | ON | Build unit and integration tests |
| `USE_MUPDF` | ON | Use MuPDF as PDF backend |
| `USE_PDFIUM` | OFF | Use PDFium as PDF backend |

Example with custom options:
```bash
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON -DBUILD_TESTS=OFF
```

## 📖 Usage

### GUI Application

Launch the graphical interface:
```bash
./bin/pdfeditor [file.pdf]
```

### CLI Application

The command-line interface provides access to all features:

#### Document Information
```bash
# Display document info
pdfeditor-cli info document.pdf

# Show page count
pdfeditor-cli pages count document.pdf
```

#### Bookmarks Management
```bash
# List all bookmarks as JSON
pdfeditor-cli bookmarks list document.pdf --json

# Add a bookmark
pdfeditor-cli bookmarks add document.pdf \
  --title "Chapter 3" \
  --page 10 \
  --out updated.pdf

# Remove a bookmark
pdfeditor-cli bookmarks remove document.pdf \
  --id bookmark_123 \
  --out updated.pdf

# Export bookmarks
pdfeditor-cli bookmarks export document.pdf \
  --format opml \
  --out bookmarks.opml
```

#### Metadata Editing
```bash
# Show all metadata
pdfeditor-cli metadata show document.pdf

# Update metadata fields
pdfeditor-cli metadata set document.pdf \
  --Title "New Title" \
  --Author "John Doe" \
  --Subject "Important Document" \
  --out updated.pdf

# Sanitize all metadata (for redaction)
pdfeditor-cli metadata sanitize document.pdf \
  --out sanitized.pdf

# Edit XMP packet
pdfeditor-cli metadata xmp document.pdf \
  --set xmp_packet.xml \
  --out updated.pdf
```

#### Page Operations
```bash
# Extract pages
pdfeditor-cli pages extract document.pdf \
  --range 1-10 \
  --out extracted.pdf

# Merge documents
pdfeditor-cli merge file1.pdf file2.pdf file3.pdf \
  --out merged.pdf

# Split document
pdfeditor-cli split document.pdf \
  --at 5,10,15 \
  --out-dir split_output/
```

#### Security
```bash
# Encrypt with password
pdfeditor-cli encrypt document.pdf \
  --user-password userpass \
  --owner-password ownerpass \
  --out encrypted.pdf

# Remove password
pdfeditor-cli decrypt encrypted.pdf \
  --password userpass \
  --out decrypted.pdf
```

#### OCR
```bash
# Perform OCR on scanned PDF
pdfeditor-cli ocr scanned.pdf \
  --language eng \
  --out searchable.pdf

# OCR specific pages
pdfeditor-cli ocr scanned.pdf \
  --pages 1-5 \
  --language eng+fra \
  --out searchable.pdf
```

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│          GUI (Qt 6)                 │
│      + CLI (Command Line)           │
├─────────────────────────────────────┤
│      Shared Library Layer           │
├─────────────────────────────────────┤
│         Core Library (C++)          │
│  ┌───────────────────────────────┐  │
│  │ Document  │ Renderer          │  │
│  │ Editor    │ Annotations       │  │
│  │ Bookmarks │ Metadata          │  │
│  │ Forms     │ Security          │  │
│  │ OCR       │ Signatures        │  │
│  └───────────────────────────────┘  │
├─────────────────────────────────────┤
│      Third-Party Libraries          │
│  MuPDF │ Tesseract │ OpenSSL │ Qt  │
└─────────────────────────────────────┘
```

## 📚 Documentation

- [Architecture Overview](docs/architecture.md) - System design and component interactions
- [API Documentation](docs/api.md) - Complete API reference
- [Developer Guide](docs/developer_guide.md) - Contributing guidelines and development setup
- [User Manual](docs/user_manual.md) - Comprehensive usage guide

## 🧪 Testing

Run the test suite:
```bash
cd build
ctest --output-on-failure
```

Run specific test categories:
```bash
# Unit tests only
ctest -R unit

# Integration tests only
ctest -R integration

# Verbose output
ctest -V
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details on:
- Code style guidelines
- Pull request process
- Issue reporting
- Development workflow

### Quick Contribution Guide

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📋 Roadmap

### Version 1.0 (Current)
- ✅ Core document viewing and editing
- ✅ Bookmarks and metadata management
- ✅ Basic annotations and forms
- ✅ Security and encryption

### Version 1.1 (Planned)
- [ ] Advanced form field types
- [ ] Enhanced OCR with layout preservation
- [ ] Batch processing operations
- [ ] Plugin system

### Version 2.0 (Future)
- [ ] Cloud storage integration
- [ ] Real-time collaboration
- [ ] AI-powered features (smart redaction, auto-tagging)
- [ ] Mobile companion apps

See [ROADMAP.md](ROADMAP.md) for detailed planning.

## 📄 License

This project is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0) - see the [LICENSE](LICENSE) file for details.

### Third-Party Licenses

- **MuPDF**: AGPL-3.0 (commercial license available from Artifex)
- **Qt**: LGPL-3.0 / Commercial
- **Tesseract**: Apache-2.0
- **OpenSSL**: Apache-2.0

## 🙏 Acknowledgments

- [Artifex Software](https://artifex.com/) for MuPDF
- [Qt Project](https://www.qt.io/) for the excellent GUI framework
- [Tesseract OCR](https://github.com/tesseract-ocr/tesseract) for text recognition
- All our [contributors](https://github.com/username/PDFEditor/graphs/contributors)

## 📞 Contact & Support

- **Issues**: [GitHub Issues](https://github.com/username/PDFEditor/issues)
- **Discussions**: [GitHub Discussions](https://github.com/username/PDFEditor/discussions)
- **Email**: support@pdfeditor.org
- **Website**: https://pdfeditor.org

## ⭐ Star History

If you find this project useful, please consider giving it a star!

[![Star History Chart](https://api.star-history.com/svg?repos=username/PDFEditor&type=Date)](https://star-history.com/#username/PDFEditor&Date)

---

**Made with ❤️ by the PDFEditor community**
