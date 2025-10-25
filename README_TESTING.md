Quick test instructions (Windows PowerShell)

Prerequisites
- Ninja or another CMake generator
- A working C compiler (MinGW/GCC, MSVC, or Clang)
- CMake 3.15+

From the repository root (PowerShell):

# Configure (create build files with Ninja)
cmake -S . -B build -G Ninja

# Build (use multiple cores)
cmake --build build --parallel

# Run the test suite
.\build\build\tests.exe

Notes
- This project was validated on Windows with MinGW/GCC and Ninja. It should also build on Linux/macOS with a C99-compatible toolchain.
- The test suite lives in `tests/` and is run by the `tests` target. Test output prints a concise PASS/FAIL summary.

If you prefer a CI step to run tests on each push, add a simple GitHub Actions workflow (example in `.github/workflows/ci.yml`).
