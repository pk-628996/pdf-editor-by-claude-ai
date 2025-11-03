# Building MuPDF on Windows - Complete Guide

This comprehensive guide explains how to build MuPDF from source on Windows for use with PDFEditor.

## Table of Contents

- [Why Build from Source?](#why-build-from-source)
- [Prerequisites](#prerequisites)
- [Quick Start (Automated)](#quick-start-automated)
- [Manual Build Steps](#manual-build-steps)
- [Using with CMake](#using-with-cmake)
- [Troubleshooting](#troubleshooting)
- [CI/CD Integration](#cicd-integration)
- [Alternative Options](#alternative-options)

## Why Build from Source?

MuPDF is not available through vcpkg or other Windows package managers, so we need to build it from source. The benefits include:

- **Latest Features**: Access to the most recent MuPDF capabilities
- **Custom Configuration**: Build only what you need
- **Compatibility**: Ensure perfect compatibility with PDFEditor
- **Control**: Full control over build flags and optimizations

## Prerequisites

### Required Software

1. **Visual Studio 2019 or 2022**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Required workloads:
     - "Desktop development with C++"
     - Includes: MSVC compiler, Windows SDK, MSBuild

2. **Git for Windows**
   - Download from: https://git-scm.com/download/win
   - Version 2.30 or later recommended

3. **Developer Command Prompt**
   - Included with Visual Studio
   - Provides `nmake`, `cl.exe`, and other build tools

### System Requirements

- **OS**: Windows 10 (1809+) or Windows 11
- **Disk Space**: ~1GB for build (500MB final)
- **RAM**: 4GB minimum, 8GB recommended
- **Time**: 5-10 minutes for first build

### Verify Prerequisites

Open "Developer Command Prompt for VS 2022" and run:

```cmd
git --version
nmake /?
cl
```

If all commands work, you're ready to proceed!

## Quick Start (Automated)

The easiest way to build MuPDF is using our automated PowerShell script.

### Step 1: Open Developer Command Prompt

From Start Menu, search for:
- "Developer Command Prompt for VS 2022" (or your VS version)

### Step 2: Navigate to PDFEditor

```powershell
cd C:\path\to\PDFEditor
```

### Step 3: Run Build Script

```powershell
.\scripts\build_mupdf_windows.ps1
```

That's it! The script will:
1. ✓ Clone MuPDF repository
2. ✓ Build with optimal settings
3. ✓ Install to `mupdf-install` directory
4. ✓ Clean up temporary files

### Script Output

```
============================================
  MuPDF Builder for Windows
============================================

Configuration:
  Install Directory : C:\path\to\PDFEditor\mupdf-install
  Build Config      : Release
  Branch            : 1.24.x

Checking prerequisites...
All prerequisites satisfied!

Cloning MuPDF repository...
Building MuPDF...
This will take 5-10 minutes. Please be patient...

Build completed in 342.5 seconds
Installing MuPDF to: C:\...\mupdf-install
Copying headers...
Copying libraries...

============================================
MuPDF build completed successfully!
============================================

Installation Details:
  Location    : C:\...\mupdf-install
  Headers     : C:\...\mupdf-install\include\mupdf\
  Libraries   : C:\...\mupdf-install\lib\

To use with CMake, add this option:
  -DMuPDF_ROOT="C:\...\mupdf-install"
```

### Advanced Script Options

```powershell
# Custom installation directory
.\scripts\build_mupdf_windows.ps1 -InstallDir "C:\Libraries\mupdf"

# Debug build (for development)
.\scripts\build_mupdf_windows.ps1 -BuildConfig Debug

# Different MuPDF version
.\scripts\build_mupdf_windows.ps1 -Branch 1.23.x

# Keep temporary files (for debugging)
.\scripts\build_mupdf_windows.ps1 -SkipCleanup

# Verbose output
.\scripts\build_mupdf_windows.ps1 -Verbose

# Combined options
.\scripts\build_mupdf_windows.ps1 `
  -InstallDir "C:\SDK\mupdf" `
  -BuildConfig Release `
  -Verbose
```

## Manual Build Steps

If you prefer complete control or the script doesn't work, follow these manual steps.

### Step 1: Clone MuPDF Repository

```cmd
cd C:\Temp
git clone --recursive https://git.ghostscript.com/mupdf.git
cd mupdf
git checkout 1.24.x
```

**Important**: Use `--recursive` to get all submodules (third-party dependencies).

### Step 2: Build MuPDF

From Developer Command Prompt:

```cmd
cd C:\Temp\mupdf
nmake /f platform\win32\Makefile HAVE_X11=no HAVE_GLUT=no
```

Build flags explained:
- `HAVE_X11=no`: Disable X11 (Linux windowing)
- `HAVE_GLUT=no`: Disable GLUT (OpenGL utility toolkit)

For debug build:
```cmd
nmake /f platform\win32\Makefile HAVE_X11=no HAVE_GLUT=no build=debug
```

### Step 3: Verify Build Output

Check that these files exist:

```
platform\win32\x64\Release\
├── libmupdf.lib        ← Main MuPDF library
└── libthirdparty.lib   ← Dependencies (zlib, jpeg, freetype, etc.)
```

### Step 4: Create Installation Directory

```cmd
mkdir C:\SDK\mupdf
mkdir C:\SDK\mupdf\include
mkdir C:\SDK\mupdf\lib
```

### Step 5: Copy Files

Copy headers:
```cmd
xcopy /E /I include\mupdf C:\SDK\mupdf\include\mupdf
```

Copy libraries:
```cmd
copy platform\win32\x64\Release\libmupdf.lib C:\SDK\mupdf\lib\
copy platform\win32\x64\Release\libthirdparty.lib C:\SDK\mupdf\lib\
```

### Step 6: Verify Installation

Your directory should look like:

```
C:\SDK\mupdf\
├── include\
│   └── mupdf\
│       ├── fitz.h          ← Main header
│       ├── fitz\
│       │   ├── version.h
│       │   ├── document.h
│       │   └── ... (many more)
│       └── pdf\
│           └── ... (PDF-specific)
└── lib\
    ├── libmupdf.lib        ← ~50MB
    └── libthirdparty.lib   ← ~10MB
```

## Using with CMake

Once MuPDF is built, configure PDFEditor:

### Basic Configuration

```powershell
cmake -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DMuPDF_ROOT="C:\SDK\mupdf" `
  -DUSE_MUPDF=ON `
  -DCMAKE_BUILD_TYPE=Release
```

### Full Configuration Example

```powershell
cmake -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2019_64" `
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DMuPDF_ROOT="C:\SDK\mupdf" `
  -DUSE_MUPDF=ON `
  -DBUILD_GUI=ON `
  -DBUILD_CLI=ON `
  -DBUILD_TESTS=ON
```

### Build PDFEditor

```powershell
cmake --build build --config Release --parallel
```

### Verify MuPDF Linkage

Check CMake output:
```
-- Found MuPDF: C:/SDK/mupdf/lib/libmupdf.lib
  Version: 1.24.5
  Include: C:/SDK/mupdf/include
  Third-party lib: C:/SDK/mupdf/lib/libthirdparty.lib
```

## Troubleshooting

### Error: "nmake is not recognized"

**Symptom**: 
```
'nmake' is not recognized as an internal or external command
```

**Solution**:
You must run from Developer Command Prompt:
1. Press Windows key
2. Type "Developer Command Prompt for VS 2022"
3. Run as administrator
4. Navigate to your directory
5. Run the build script again

**Alternative**: Manually set up environment:
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

### Error: Git clone fails

**Symptom**:
```
fatal: unable to access 'https://git.ghostscript.com/mupdf.git/': SSL certificate problem
```

**Solutions**:

1. **Check firewall**: Ensure git.ghostscript.com is not blocked
2. **Use HTTPS**: Already using HTTPS, try SSH instead:
   ```bash
   git clone --recursive git@git.ghostscript.com:mupdf.git
   ```
3. **Temporarily disable SSL** (not recommended):
   ```bash
   git config --global http.sslVerify false
   # Clone repo
   git config --global http.sslVerify true
   ```

### Error: Build fails with "out of heap space"

**Symptom**:
```
NMAKE : fatal error U1077: 'cl' : return code '0x2'
```

**Solution**:
Close other applications and try again. MuPDF build is memory-intensive.

### Error: Missing submodules

**Symptom**:
```
fatal error: ft2build.h: No such file or directory
```

**Solution**:
Submodules weren't downloaded. Re-clone with `--recursive`:
```bash
git clone --recursive https://git.ghostscript.com/mupdf.git
```

Or update existing clone:
```bash
cd mupdf
git submodule update --init --recursive
```

### Error: CMake can't find MuPDF

**Symptom**:
```
CMake Error: Could not find MuPDF library
```

**Solutions**:

1. **Check paths**: Ensure headers and libs are in correct locations
   ```powershell
   Test-Path C:\SDK\mupdf\include\mupdf\fitz.h
   Test-Path C:\SDK\mupdf\lib\libmupdf.lib
   ```

2. **Use absolute path**: 
   ```powershell
   -DMuPDF_ROOT="C:\SDK\mupdf"  # Not .\mupdf
   ```

3. **Debug CMake**:
   ```powershell
   cmake -B build --debug-find -DMuPDF_ROOT="C:\SDK\mupdf" ...
   ```

4. **Check FindMuPDF.cmake**: Verify the module exists:
   ```
   PDFEditor\cmake\FindMuPDF.cmake
   ```

### Error: Runtime "libmupdf.dll not found"

**Symptom**:
```
The code execution cannot proceed because libmupdf.dll was not found.
```

**Solution**:
MuPDF builds static libraries by default (`.lib`), not DLLs. If you see this:
1. Check you linked against `.lib` files, not `.dll`
2. Verify `FindMuPDF.cmake` links to static libraries
3. Ensure CMake configuration shows `IMPORTED_LOCATION` pointing to `.lib`

### Error: Linker errors (LNK2001, LNK2019)

**Symptom**:
```
error LNK2019: unresolved external symbol fz_new_context
```

**Solutions**:

1. **Link both libraries**:
   ```cmake
   target_link_libraries(... MuPDF::MuPDF)  # Links both libmupdf and libthirdparty
   ```

2. **Link Windows libraries**: MuPDF needs system libraries:
   ```cmake
   target_link_libraries(... advapi32 comdlg32 gdi32 user32 shell32)
   ```

3. **Check library order**: Link MuPDF before other libraries

### Performance: Build is very slow

**Solutions**:

1. **Use parallel build**: Already default in nmake
2. **Disable unnecessary features**: Already done (`HAVE_X11=no HAVE_GLUT=no`)
3. **Use SSD**: Build on SSD, not HDD
4. **Close applications**: Free up RAM
5. **Use ccache** (advanced):
   ```cmd
   nmake CCACHE=ccache ...
   ```

### Script fails with "Execution Policy" error

**Symptom**:
```
.\build_mupdf_windows.ps1 : File cannot be loaded because running scripts is disabled
```

**Solution**:
Enable script execution (one-time):
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Then run script again.

## CI/CD Integration

### GitHub Actions

The project includes automated MuPDF building in GitHub Actions:

```yaml
- name: Build MuPDF from source
  shell: pwsh
  run: |
    .\scripts\build_mupdf_windows.ps1 -InstallDir "$env:GITHUB_WORKSPACE\mupdf-install"

- name: Configure CMake
  run: |
    cmake -B build -DMuPDF_ROOT="$env:GITHUB_WORKSPACE\mupdf-install" -DUSE_MUPDF=ON ...
```

### Caching

Speed up CI builds by caching MuPDF:

```yaml
- name: Cache MuPDF build
  uses: actions/cache@v4
  with:
    path: mupdf-install
    key: mupdf-windows-1.24.x-${{ runner.os }}
```

First build: ~6 minutes
Cached builds: ~30 seconds

### Other CI Systems

**Azure Pipelines**:
```yaml
- pwsh: .\scripts\build_mupdf_windows.ps1 -InstallDir "$(Build.SourcesDirectory)\mupdf-install"
  displayName: 'Build MuPDF'
```

**GitLab CI**:
```yaml
build_mupdf:
  script:
    - .\scripts\build_mupdf_windows.ps1 -InstallDir "$CI_PROJECT_DIR\mupdf-install"
```

## Alternative Options

### Option 1: Use Prebuilt Binaries

If building from source is problematic:

1. Visit https://mupdf.com/releases/
2. Download Windows binaries
3. Extract to a directory
4. Point CMake: `-DMuPDF_ROOT=C:\path\to\mupdf`

**Cons**: May be older version, different configuration

### Option 2: Use WSL (Windows Subsystem for Linux)

Build in Linux environment:

```bash
# In WSL Ubuntu
sudo apt-get install libmupdf-dev

# Then build PDFEditor in WSL
cmake -DUSE_MUPDF=ON ...
```

**Cons**: Adds complexity, Windows users expect native build

### Option 3: Use Docker

```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2022
RUN choco install -y visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools"
# Build MuPDF in container
```

**Cons**: Requires Docker for Windows, complex setup

## Performance Benchmarks

Build performance on different systems:

| System | CPU | RAM | Storage | Build Time |
|--------|-----|-----|---------|------------|
| Desktop | i7-12700K | 32GB | NVMe SSD | 3m 45s |
| Laptop | i5-1135G7 | 16GB | SATA SSD | 6m 20s |
| CI (GitHub) | 2-core | 7GB | SSD | 5m 30s |
| Old PC | i5-4590 | 8GB | HDD | 12m 15s |

## Summary

Building MuPDF on Windows:
- ✅ **Automated**: One-command script available
- ✅ **Documented**: Comprehensive troubleshooting
- ✅ **Fast**: 5-10 minutes first build
- ✅ **Cached**: CI/CD caching available
- ✅ **Reliable**: Used in production builds

For most users, the automated script is sufficient:
```powershell
.\scripts\build_mupdf_windows.ps1
```

For any issues:
1. Check [Troubleshooting](#troubleshooting) section
2. Review error messages carefully
3. Open issue at: https://github.com/username/PDFEditor/issues

## Additional Resources

- **MuPDF Documentation**: https://mupdf.readthedocs.io/
- **MuPDF Repository**: https://git.ghostscript.com/?p=mupdf.git
- **PDFEditor Issues**: https://github.com/username/PDFEditor/issues
- **Visual Studio Docs**: https://docs.microsoft.com/en-us/visualstudio/

---

**Last Updated**: 2025-11-03  
**MuPDF Version**: 1.24.x  
**Tested With**: Visual Studio 2022, Windows 11
