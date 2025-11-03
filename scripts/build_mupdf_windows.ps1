# Build MuPDF from source on Windows
# This script clones and builds MuPDF for use with PDFEditor
# 
# Usage:
#   .\build_mupdf_windows.ps1 [-InstallDir <path>] [-BuildConfig <config>] [-Branch <branch>]
#
# Examples:
#   .\build_mupdf_windows.ps1
#   .\build_mupdf_windows.ps1 -InstallDir "C:\Libraries\mupdf"
#   .\build_mupdf_windows.ps1 -BuildConfig Debug -Branch 1.23.x

param(
    [string]$InstallDir = "$PSScriptRoot\..\mupdf-install",
    [string]$BuildConfig = "Release",
    [string]$Branch = "1.24.x",
    [switch]$SkipCleanup,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# Enable verbose output if requested
if ($Verbose) {
    $VerbosePreference = "Continue"
}

function Write-Info {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Green
}

function Write-Error {
    param([string]$Message)
    Write-Host "ERROR: $Message" -ForegroundColor Red
}

function Test-Command {
    param([string]$Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

# Banner
Write-Host ""
Write-Host "============================================" -ForegroundColor Yellow
Write-Host "  MuPDF Builder for Windows" -ForegroundColor Yellow
Write-Host "============================================" -ForegroundColor Yellow
Write-Host ""
Write-Info "Configuration:"
Write-Host "  Install Directory : $InstallDir"
Write-Host "  Build Config      : $BuildConfig"
Write-Host "  Branch            : $Branch"
Write-Host ""

# Check prerequisites
Write-Info "Checking prerequisites..."

if (-not (Test-Command "git")) {
    Write-Error "Git is not installed or not in PATH"
    exit 1
}
Write-Verbose "  ✓ Git found"

if (-not (Test-Command "nmake")) {
    Write-Error "nmake not found. Please run this script from a Visual Studio Developer Command Prompt."
    Write-Host ""
    Write-Host "To fix this:" -ForegroundColor Yellow
    Write-Host "  1. Open 'Developer Command Prompt for VS 2022' from Start Menu"
    Write-Host "  2. Navigate to this directory"
    Write-Host "  3. Run the script again"
    Write-Host ""
    exit 1
}
Write-Verbose "  ✓ nmake found"

if (-not (Test-Command "cl")) {
    Write-Error "MSVC compiler (cl.exe) not found."
    exit 1
}
Write-Verbose "  ✓ MSVC compiler found"

Write-Success "All prerequisites satisfied!"
Write-Host ""

# Create absolute path for install directory
$InstallDir = [System.IO.Path]::GetFullPath($InstallDir)

# Create temp directory
$TempDir = Join-Path $env:TEMP "mupdf-build-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
Write-Info "Creating temporary directory: $TempDir"
New-Item -ItemType Directory -Path $TempDir -Force | Out-Null

try {
    Push-Location $TempDir
    
    # Clone MuPDF
    Write-Host ""
    Write-Info "Cloning MuPDF repository (branch: $Branch)..."
    Write-Host "This may take a few minutes depending on your connection..."
    
    $gitOutput = git clone --recursive --depth 1 --branch $Branch https://git.ghostscript.com/mupdf.git 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to clone MuPDF repository"
        Write-Host $gitOutput
        throw "Git clone failed"
    }
    Write-Success "Repository cloned successfully"
    
    # Enter MuPDF directory
    Set-Location mupdf
    
    # Show MuPDF version info
    if (Test-Path "include\mupdf\fitz\version.h") {
        $versionContent = Get-Content "include\mupdf\fitz\version.h" | Select-String 'FZ_VERSION'
        if ($versionContent) {
            Write-Host ""
            Write-Info "MuPDF Version Information:"
            Write-Host "  $versionContent"
        }
    }
    
    # Build MuPDF
    Write-Host ""
    Write-Info "Building MuPDF..."
    Write-Host "This will take 5-10 minutes. Please be patient..."
    Write-Host ""
    
    $buildStart = Get-Date
    
    # Build command
    $nmakeArgs = @(
        "/f", "platform\win32\Makefile",
        "HAVE_X11=no",
        "HAVE_GLUT=no"
    )
    
    if ($BuildConfig -eq "Debug") {
        $nmakeArgs += "build=debug"
    }
    
    Write-Verbose "Running: nmake $($nmakeArgs -join ' ')"
    
    $buildOutput = & nmake @nmakeArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build MuPDF"
        Write-Host $buildOutput | Select-Object -Last 50
        throw "Build failed"
    }
    
    $buildEnd = Get-Date
    $buildDuration = ($buildEnd - $buildStart).TotalSeconds
    Write-Success "Build completed in $([math]::Round($buildDuration, 1)) seconds"
    
    # Determine library path based on build config
    if ($BuildConfig -eq "Debug") {
        $LibPath = "platform\win32\x64\Debug"
    } else {
        $LibPath = "platform\win32\x64\Release"
    }
    
    # Fallback to Debug if Release not found
    if (-not (Test-Path $LibPath)) {
        Write-Host "Warning: $BuildConfig libraries not found, trying alternate configuration..."
        if ($BuildConfig -eq "Release") {
            $LibPath = "platform\win32\x64\Debug"
        } else {
            $LibPath = "platform\win32\x64\Release"
        }
    }
    
    if (-not (Test-Path $LibPath)) {
        Write-Error "Could not find build output directory"
        Write-Host "Expected: $LibPath"
        throw "Build output not found"
    }
    
    Write-Verbose "Found libraries in: $LibPath"
    
    # Verify required files exist
    $requiredFiles = @(
        "$LibPath\libmupdf.lib",
        "$LibPath\libthirdparty.lib"
    )
    
    foreach ($file in $requiredFiles) {
        if (-not (Test-Path $file)) {
            Write-Error "Required file not found: $file"
            throw "Missing build output"
        }
    }
    
    # Create install directory structure
    Write-Host ""
    Write-Info "Installing MuPDF to: $InstallDir"
    
    if (Test-Path $InstallDir) {
        Write-Verbose "Removing existing installation directory..."
        Remove-Item -Path $InstallDir -Recurse -Force
    }
    
    Write-Verbose "Creating directory structure..."
    New-Item -ItemType Directory -Path "$InstallDir\include" -Force | Out-Null
    New-Item -ItemType Directory -Path "$InstallDir\lib" -Force | Out-Null
    New-Item -ItemType Directory -Path "$InstallDir\bin" -Force | Out-Null
    
    # Copy headers
    Write-Info "Copying headers..."
    Copy-Item -Path "include\mupdf" -Destination "$InstallDir\include\" -Recurse -Force
    Write-Verbose "  Copied include/mupdf/ -> $InstallDir\include\mupdf\"
    
    # Copy libraries
    Write-Info "Copying libraries..."
    Copy-Item -Path "$LibPath\libmupdf.lib" -Destination "$InstallDir\lib\" -Force
    Write-Verbose "  Copied libmupdf.lib"
    
    Copy-Item -Path "$LibPath\libthirdparty.lib" -Destination "$InstallDir\lib\" -Force
    Write-Verbose "  Copied libthirdparty.lib"
    
    # Copy DLLs if they exist (uncommon, but possible)
    if (Test-Path "$LibPath\libmupdf.dll") {
        Copy-Item -Path "$LibPath\libmupdf.dll" -Destination "$InstallDir\bin\" -Force
        Write-Verbose "  Copied libmupdf.dll"
    }
    if (Test-Path "$LibPath\libthirdparty.dll") {
        Copy-Item -Path "$LibPath\libthirdparty.dll" -Destination "$InstallDir\bin\" -Force
        Write-Verbose "  Copied libthirdparty.dll"
    }
    
    # Create a build info file
    $buildInfo = @"
MuPDF Build Information
=======================
Build Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Branch: $Branch
Configuration: $BuildConfig
Build Time: $([math]::Round($buildDuration, 1)) seconds
Install Directory: $InstallDir

Usage with CMake:
  cmake -DMuPDF_ROOT="$InstallDir" -DUSE_MUPDF=ON ...
"@
    
    $buildInfo | Out-File -FilePath "$InstallDir\BUILD_INFO.txt" -Encoding utf8
    Write-Verbose "  Created BUILD_INFO.txt"
    
    Pop-Location
    
    # Summary
    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Success "MuPDF build completed successfully!"
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installation Details:" -ForegroundColor Yellow
    Write-Host "  Location    : $InstallDir"
    Write-Host "  Headers     : $InstallDir\include\mupdf\"
    Write-Host "  Libraries   : $InstallDir\lib\"
    Write-Host "  Build Info  : $InstallDir\BUILD_INFO.txt"
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Yellow
    Write-Host "  To use with CMake, add this option:"
    Write-Host "    -DMuPDF_ROOT=`"$InstallDir`"" -ForegroundColor Cyan
    Write-Host ""
    
    # Verify installation
    $headerCount = (Get-ChildItem "$InstallDir\include\mupdf" -Recurse -File).Count
    $libCount = (Get-ChildItem "$InstallDir\lib" -File -Filter "*.lib").Count
    
    Write-Info "Verification:"
    Write-Host "  Headers : $headerCount files"
    Write-Host "  Libraries : $libCount files"
    Write-Host ""
    
} catch {
    Write-Host ""
    Write-Error "Build failed: $_"
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Yellow
    Write-Host "  1. Ensure you're running from Visual Studio Developer Command Prompt"
    Write-Host "  2. Check that Git can access git.ghostscript.com"
    Write-Host "  3. Verify you have enough disk space (~500MB)"
    Write-Host "  4. Check the error messages above for specific issues"
    Write-Host ""
    exit 1
} finally {
    # Cleanup
    Pop-Location -ErrorAction SilentlyContinue
    
    if (-not $SkipCleanup) {
        Write-Info "Cleaning up temporary files..."
        if (Test-Path $TempDir) {
            try {
                Remove-Item -Path $TempDir -Recurse -Force -ErrorAction Stop
                Write-Verbose "  Removed $TempDir"
            } catch {
                Write-Host "Warning: Could not remove temporary directory: $TempDir" -ForegroundColor Yellow
                Write-Host "You can safely delete it manually." -ForegroundColor Yellow
            }
        }
    } else {
        Write-Host "Temporary files kept at: $TempDir" -ForegroundColor Yellow
    }
}

exit 0
