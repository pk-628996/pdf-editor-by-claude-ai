# Build MuPDF from source on Windows
# This script clones and builds MuPDF for use with PDFEditor

param(
    [string]$InstallDir = "$PSScriptRoot\..\mupdf-install",
    [string]$BuildConfig = "Release",
    [string]$Branch = "1.24.x"
)

$ErrorActionPreference = "Stop"

Write-Host "Building MuPDF for Windows..." -ForegroundColor Green
Write-Host "Install directory: $InstallDir"
Write-Host "Build configuration: $BuildConfig"
Write-Host "Branch: $Branch"
Write-Host ""

# Check for nmake
$nmake = Get-Command nmake -ErrorAction SilentlyContinue
if (-not $nmake) {
    Write-Error "nmake not found. Please run this script from a Visual Studio Developer Command Prompt."
    exit 1
}

# Create temp directory
$TempDir = Join-Path $env:TEMP "mupdf-build"
if (Test-Path $TempDir) {
    Write-Host "Removing existing temporary directory..."
    Remove-Item -Path $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TempDir | Out-Null

try {
    # Clone MuPDF
    Write-Host "Cloning MuPDF repository..." -ForegroundColor Cyan
    Push-Location $TempDir
    
    git clone --recursive --depth 1 --branch $Branch https://git.ghostscript.com/mupdf.git
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to clone MuPDF repository"
    }
    
    Set-Location mupdf
    
    # Build MuPDF
    Write-Host "Building MuPDF..." -ForegroundColor Cyan
    Write-Host "This may take several minutes..."
    
    # Build for x64 Release
    nmake /f platform/win32/Makefile HAVE_X11=no HAVE_GLUT=no
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to build MuPDF"
    }
    
    # Create install directory
    Write-Host "Installing MuPDF to $InstallDir..." -ForegroundColor Cyan
    
    if (Test-Path $InstallDir) {
        Remove-Item -Path $InstallDir -Recurse -Force
    }
    
    New-Item -ItemType Directory -Path "$InstallDir\include" -Force | Out-Null
    New-Item -ItemType Directory -Path "$InstallDir\lib" -Force | Out-Null
    
    # Copy headers
    Write-Host "Copying headers..."
    Copy-Item -Path "include\mupdf" -Destination "$InstallDir\include\" -Recurse -Force
    
    # Copy libraries
    Write-Host "Copying libraries..."
    $LibPath = "platform\win32\x64\Release"
    if (-not (Test-Path $LibPath)) {
        # Try Debug build
        $LibPath = "platform\win32\x64\Debug"
    }
    
    if (Test-Path $LibPath) {
        Copy-Item -Path "$LibPath\libmupdf.lib" -Destination "$InstallDir\lib\" -Force
        Copy-Item -Path "$LibPath\libthirdparty.lib" -Destination "$InstallDir\lib\" -Force
        
        # Also copy DLLs if they exist
        if (Test-Path "$LibPath\libmupdf.dll") {
            Copy-Item -Path "$LibPath\libmupdf.dll" -Destination "$InstallDir\lib\" -Force
        }
        if (Test-Path "$LibPath\libthirdparty.dll") {
            Copy-Item -Path "$LibPath\libthirdparty.dll" -Destination "$InstallDir\lib\" -Force
        }
    } else {
        throw "Could not find MuPDF build output"
    }
    
    Pop-Location
    
    Write-Host ""
    Write-Host "MuPDF build completed successfully!" -ForegroundColor Green
    Write-Host "Installation directory: $InstallDir"
    Write-Host ""
    Write-Host "To use with CMake, add: -DMuPDF_ROOT=`"$InstallDir`""
    
} catch {
    Write-Error "Build failed: $_"
    exit 1
} finally {
    # Cleanup
    Pop-Location -ErrorAction SilentlyContinue
    if (Test-Path $TempDir) {
        Write-Host "Cleaning up temporary files..."
        Remove-Item -Path $TempDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

exit 0
