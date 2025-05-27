@echo off
setlocal enabledelayedexpansion

set BUILD_TYPE=Release
set BUILD_DIR=build
set INSTALL_PREFIX=C:\Program Files\AnalyzeMFT
set JOBS=%NUMBER_OF_PROCESSORS%
set ENABLE_SIMD=ON
set ENABLE_TESTING=ON
set ENABLE_OPENMP=OFF
set GENERATOR="Visual Studio 16 2019"

:parse_args
if "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto parse_args
)
if "%~1"=="--prefix" (
    set INSTALL_PREFIX=%~2
    shift
    shift
    goto parse_args
)
if "%~1"=="--jobs" (
    set JOBS=%~2
    shift
    shift
    goto parse_args
)
if "%~1"=="--no-simd" (
    set ENABLE_SIMD=OFF
    shift
    goto parse_args
)
if "%~1"=="--no-tests" (
    set ENABLE_TESTING=OFF
    shift
    goto parse_args
)
if "%~1"=="--openmp" (
    set ENABLE_OPENMP=ON
    shift
    goto parse_args
)
if "%~1"=="--generator" (
    set GENERATOR=%~2
    shift
    shift
    goto parse_args
)
if "%~1"=="--help" (
    echo Usage: %0 [OPTIONS]
    echo Options:
    echo   --debug         Build in debug mode
    echo   --build-dir DIR Specify build directory ^(default: build^)
    echo   --prefix DIR    Install prefix ^(default: C:\Program Files\AnalyzeMFT^)
    echo   --jobs N        Number of parallel jobs ^(default: %NUMBER_OF_PROCESSORS%^)
    echo   --no-simd       Disable SIMD optimizations
    echo   --no-tests      Disable testing
    echo   --openmp        Enable OpenMP support
    echo   --generator GEN CMake generator ^(default: "Visual Studio 16 2019"^)
    echo   --help          Show this help
    exit /b 0
)
if not "%~1"=="" (
    echo Unknown option: %~1
    exit /b 1
)

echo Building AnalyzeMFT C++
echo ======================
echo Build type: %BUILD_TYPE%
echo Build directory: %BUILD_DIR%
echo Install prefix: %INSTALL_PREFIX%
echo Jobs: %JOBS%
echo SIMD: %ENABLE_SIMD%
echo Testing: %ENABLE_TESTING%
echo OpenMP: %ENABLE_OPENMP%
echo Generator: %GENERATOR%
echo.

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

cmake .. ^
    -G %GENERATOR% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
    -DENABLE_SIMD=%ENABLE_SIMD% ^
    -DENABLE_TESTING=%ENABLE_TESTING% ^
    -DENABLE_OPENMP=%ENABLE_OPENMP%

if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

cmake --build . --config %BUILD_TYPE% --parallel %JOBS%

if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

if "%ENABLE_TESTING%"=="ON" (
    echo.
    echo Running tests...
    ctest --parallel %JOBS% --output-on-failure --build-config %BUILD_TYPE%
)

echo.
echo Build completed successfully!
echo Executable: %BUILD_DIR%\%BUILD_TYPE%\analyzemft.exe
echo Library: %BUILD_DIR%\%BUILD_TYPE%\analyzemft.lib

if "%BUILD_TYPE%"=="Release" (
    echo.
    echo To install system-wide, run as administrator:
    echo   cmake --install %BUILD_DIR% --config %BUILD_TYPE%
)