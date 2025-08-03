# BDMan — Bulk Download Manager

**BDMan** is a cross-platform C++ download manager that is built using the Curl library.

## 🔧 Features

- ✅ Multi-threaded HTTP downloads (coming soon...)
- ✅ Fallback to single-threaded mode if threads fail (coming soon...)
- ✅ Real-time progress indicators
- ✅ CMake-based build system
- ✅ Cross-platform (Windows & Linux)
- ✅ Lightweight and dependency-minimal

---

## 🖥️ Screenshots (Coming soon...)



---

## 🧱 Requirements

### 🔹 Common

- CMake ≥ 3.16
- C++17 compatible compiler

### 🔹 Windows
- Visual Studio 2019 or newer
- `libcurl` (via `vcpkg` recommended)

### 🔹 Linux
- GCC or Clang
- `libcurl4-openssl-dev` or similar

---

## 📦 Building the Project

### 🔹 Windows (Visual Studio + CMake + vcpkg)

```bash
git clone https://github.com/yourusername/BDMan.git
cd BDMan

# Install libcurl using vcpkg (if not already done)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install curl

# Go back to project root
cd ..
mkdir out && cd out
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
