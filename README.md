# AnalyzeMFT C++

**WARNING: ACTIVE DEVELOPMENT - PRE-BETA SOFTWARE** 

This project is currently under **active development** and is in a **pre-beta state**. 

**DO NOT USE IN PRODUCTION ENVIRONMENTS**

---

## Overview

AnalyzeMFT C++ is a high-performance C++ port of the [Python analyzeMFT](https://github.com/rowingdude/analyzeMFT) tool for parsing and analyzing NTFS Master File Table (MFT) files. When complete, it will provide features such as:

1.  Fast parsing of MFT records and attributes
2.  Multiple export formats (CSV, JSON, XML, SQLite, Excel, Timeline)
3.  Hardware-accelerated operations (SIMD, CRC32)
4.  Cross-platform support (Linux, macOS, Windows)
5.  The familiar CLI and library interfaces I wrote into the Python Script

## Current Status

This is a **complete rewrite** from Python to C++ that is currently:
- [ ] Core MFT parsing (in progress)
- [ ] Attribute parsers (implemented but untested)
- [ ] File writers (basic implementation)
- [ ] Command-line interface (basic implementation)
- [ ] Testing framework (not started)
- [ ] Documentation (minimal)
- [ ] Windows compatibility (untested)
- [ ] Memory safety audit (not performed)

## Dependencies

### Fedora / RHEL / CentOS
```bash
# 1. Basic Development tools
sudo dnf group install development-tools
sudo dnf install cmake ninja-build

# 2. Project Specific Required libraries
sudo dnf install openssl-devel sqlite-devel

# 3. Developer/Optional dependencies
# Please include this in your development chain if you want your contributions to be considered
sudo dnf install gtest-devel 

# 4. If you wish to expand the Python API:
sudo dnf install python3-devel pybind11-devel
```

## Ubuntu / Debian
```bash
sudo pacman -S apt install build-essential cmake ninja-build libssl-dev libsqlite3-dev libgtest-dev  python3-dev pybind11-dev
```

## Arch Linux / Manjaro
```bash
sudo pacman -S base-devel cmake ninja openssl sqlite gtest  python pybind11
```

## Windows (vcpkg)
```powershell
# Install vcpkg first: https://github.com/Microsoft/vcpkg
vcpkg install openssl:x64-windows
vcpkg install sqlite3:x64-windows
vcpkg install gtest:x64-windows
vcpkg install pybind11:x64-windows
```


# Building (Experimental - May Not Work)

**These build instructions are theoretical and untested in production**

```bash
git clone https://github.com/yourusername/analyzeMFT-cpp.git
cd analyzeMFT-cpp

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON
cmake --build . --parallel $(nproc)
ctest --parallel $(nproc) --output-on-failure
```

**Or use the Official Project build scripts (also under active development)**

```bash
./build/scripts/build.sh --debug --no-tests  # Linux/macOS

build\scripts\build.bat --debug --no-tests   # Windows
```

# Planned Features
When complete, this tool will support:
Input/Output Formats

- Raw MFT files
- CSV export
- JSON export
- XML export
- SQLite database
- Excel spreadsheets
- Timeline formats (TSK, log2timeline)
- Body files for mactime

Analysis Features

- File system timeline reconstruction
- Deleted file recovery information
- File metadata extraction
- Hash calculation (MD5, SHA256, SHA512, CRC32)
- Attribute analysis
- Directory tree reconstruction

Performance Features

- Multi-threaded processing
- SIMD optimizations (AVX2, SSE4.2)
- Memory-mapped file I/O
- Streaming processing for large MFTs
- Hardware CRC32 acceleration

Contributing
Please do not contribute yet - the codebase is too unstable and changing too rapidly. Wait for the first alpha release.
When ready to accept contributions:

1. Code must compile without warnings
2. All tests must pass
3. Follow the existing code style
4. Document new features
5. Add appropriate error handling

# License
This project maintains compatibility with the original analyzeMFT MIT license.

Original Author: Benjamin Cance
Original Repository: https://github.com/rowingdude/analyzeMFT
Python Version: 3.0.6.6

# Disclaimer
**This software is provided "as is" without warranty of any kind. The authors are not responsible for:**

- **Data loss or corruption**
- **System crashes or instability**
- **Incorrect analysis results**
- **Security vulnerabilities**
- **Any other damages resulting from use**

## Use at your own risk and always work with backup copies of important data.