# Building MuPDF on Windows

This guide explains how to build MuPDF from source on Windows for use with PDFEditor.

## Why Build from Source?

MuPDF is not available through vcpkg or other Windows package managers, so we need to build it from source. The good news is that we've automated this process!

## Prerequisites

Before building MuPDF, ensure you have:

1. **Visual Studio 2019 or 2022** with C++ development tools
2. **Git** for cloning the MuPDF repository
3. **nmake** (included with Visual Studio)

## Automated Build (Recommended)

We provide a PowerShell script that automates the entire build process:

```powershell
# From the PDFEditor project root
.\scripts\build_mupdf_windows.ps1
```

This script will:
1. Clone the MuPDF repository (1.24.x branch)
2. Build MuPDF for x64 Release configuration
3. Install headers and libraries to `mupdf-install` directory
4. Clean up temporary files

### Script Options

```powershell
# Custom install directory
.\scripts\build_mupdf_windows.ps1 -InstallDir "C:\Libraries\mupdf"

# Different branch
.\scripts\build_mupdf_windows.ps1 -Branch "1.23.x"

# Debug build
.\scripts\build_mupdf_windows.ps1 -BuildConfig "Debug"
```

## Manual Build

If you prefer to build manually:

### Step 1: Clone MuPDF

```bash
git clone --recursive https://git.ghostscript.com/mupdf.git
cd mupdf
git checkout 1.24.x
```

### Step 2: Open Developer Command Prompt

Launch "Developer Command Prompt for VS 2022" (or your VS version) from the Start menu.

### Step 3: Build MuPDF

```cmd
cd path\to\mupdf
nmake /f platform/win32/Makefile HAVE_X11=no HAVE_GLUT=no
```

This will:
- Build `libmupdf.lib` - Main MuPDF library
- Build `libthirdparty.lib` - Third-party dependencies (zlib, jpeg, freetype, etc.)

The libraries will be in: `platform\win32\x64\Release\`

### Step 4: Install Headers and Libraries

Create an installation directory structure:

```
mupdf-install/
├── include/
│   └── mupdf/
│       ├── fitz.h
│       └── ... (all headers)
└── lib/
    ├── libmupdf.lib
    └── libthirdparty.lib
```

Copy files:
```cmd
xcopy /E /I include\mupdf C:\path\to\mupdf-install\include\mupdf
copy platform\win32\x64\Release\libmupdf.lib C:\path\to\mupdf-install\lib\
copy platform\win32\x64\Release\libthirdparty.lib C:\path\to\mupdf-install\lib\
```

## Using MuPDF with CMake

After building MuPDF, configure PDFEditor with:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
    -DMuPDF_ROOT="C:\path\to\mupdf-install" `
    -DUSE_MUPDF=ON `
    ... (other options)
```

The `FindMuPDF.cmake` module will automatically find the libraries in the specified directory.

## Troubleshooting

### nmake not found

**Problem:** `nmake` is not recognized as a command.

**Solution:** Make sure you're running from a Visual Studio Developer Command Prompt, not a regular PowerShell or CMD window.

To verify:
```cmd
where nmake
```

### Build fails with linker errors

**Problem:** Missing dependencies during build.

**Solution:** MuPDF builds its dependencies automatically. Ensure you cloned with `--recursive` to get all submodules:
```bash
git submodule update --init --recursive
```

### Libraries not found by CMake

**Problem:** CMake can't find MuPDF even though it's built.

**Solution:** Check that:
1. Headers are in `<MuPDF_ROOT>/include/mupdf/`
2. Libraries are in `<MuPDF_ROOT>/lib/`
3. You're passing `-DMuPDF_ROOT=<path>` to CMake
4. The path doesn't contain spaces or special characters

For verbose output:
```powershell
cmake -B build --debug-find -DMuPDF_ROOT="..." ...
```

### Runtime errors about missing DLLs

**Problem:** Application fails to start with DLL errors.

**Solution:** MuPDF builds static libraries by default, so DLLs shouldn't be needed. If you see this error:
1. Check that you're linking against the correct libraries
2. Ensure all Windows system libraries are linked (see `FindMuPDF.cmake`)
3. Verify the build configuration (Debug/Release) matches

## CI/CD Integration

The GitHub Actions workflows automatically build MuPDF on Windows runners:

```yaml
- name: Build MuPDF from source
  shell: pwsh
  run: |
    .\scripts\build_mupdf_windows.ps1 -InstallDir "$env:GITHUB_WORKSPACE\mupdf-install"

- name: Configure CMake
  run: |
    cmake -B build -DMuPDF_ROOT="$env:GITHUB_WORKSPACE\mupdf-install" ...
```

## Build Time

Building MuPDF from source takes approximately:
- **First build:** 5-10 minutes
- **Subsequent builds:** 2-5 minutes (if cleaning temp files)

The build is cached in CI/CD to speed up subsequent runs.

## Alternative: Prebuilt Binaries

If building from source is problematic, you can download prebuilt MuPDF binaries:

1. Visit [MuPDF Downloads](https://mupdf.com/releases/)
2. Download the Windows build
3. Extract to a directory
4. Point CMake to it with `-DMuPDF_ROOT=<path>`

**Note:** Prebuilt binaries may be different versions or configurations.

## Summary

MuPDF is essential for PDF rendering in PDFEditor. While it requires building from source on Windows, we've made this as painless as possible with automated scripts and clear documentation. The one-time setup cost is worth it for the robust PDF rendering capabilities!

For issues or questions, see:
- [MuPDF Documentation](https://mupdf.readthedocs.io/)
- [PDFEditor GitHub Issues](https://github.com/username/PDFEditor/issues)
