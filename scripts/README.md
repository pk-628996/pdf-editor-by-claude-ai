# PDFEditor Build Scripts

This directory contains helper scripts for building and packaging PDFEditor.

## Windows Scripts

### `build_mupdf_windows.ps1`

Builds MuPDF from source on Windows.

**Usage:**
```powershell
.\build_mupdf_windows.ps1 [-InstallDir <path>] [-BuildConfig <config>] [-Branch <branch>]
```

**Parameters:**
- `-InstallDir`: Where to install MuPDF (default: `../mupdf-install`)
- `-BuildConfig`: Build configuration - `Release` or `Debug` (default: `Release`)
- `-Branch`: MuPDF git branch to build (default: `1.24.x`)

**Example:**
```powershell
# Default build
.\build_mupdf_windows.ps1

# Custom installation directory
.\build_mupdf_windows.ps1 -InstallDir "C:\Libraries\mupdf"

# Debug build
.\build_mupdf_windows.ps1 -BuildConfig Debug

# Specific version
.\build_mupdf_windows.ps1 -Branch 1.23.x
```

**Requirements:**
- Visual Studio 2019 or later
- Git
- Run from Developer Command Prompt for VS

**What it does:**
1. Clones MuPDF repository (with submodules)
2. Builds using nmake
3. Copies headers to `<InstallDir>/include/mupdf/`
4. Copies libraries to `<InstallDir>/lib/`
5. Cleans up temporary files

**Output:**
```
<InstallDir>/
├── include/
│   └── mupdf/
│       ├── fitz.h
│       └── ... (all MuPDF headers)
└── lib/
    ├── libmupdf.lib
    └── libthirdparty.lib
```

## Linux/macOS Scripts

### `package_linux.sh` (TODO)

Creates Linux packages (DEB, RPM, AppImage).

### `package_macos.sh` (TODO)

Creates macOS DMG installer.

## Development Scripts

### `dev_setup.sh` (TODO)

Sets up development environment with all dependencies.

### `run_tests.sh` (TODO)

Runs the complete test suite with coverage reporting.

## Packaging Scripts

### `package_win.ps1` (TODO)

Creates Windows installer (MSI) using WiX.

**Usage:**
```powershell
.\package_win.ps1 -Version 1.0.0
```

## CI/CD Scripts

These scripts are designed to work in CI/CD environments (GitHub Actions, etc.) and can also be used locally.

**Environment Variables:**
- `GITHUB_WORKSPACE`: Set to workspace root (for CI)
- `BUILD_TYPE`: Release or Debug
- `QT_VERSION`: Qt version to use

## Contributing

When adding new scripts:

1. **Use consistent naming:** `<action>_<platform>.<ext>`
2. **Add error handling:** Scripts should fail gracefully
3. **Document parameters:** Use inline help or README
4. **Test locally:** Verify script works before committing
5. **Update this README:** Add script documentation

## License

These scripts are part of PDFEditor and licensed under AGPL-3.0.
