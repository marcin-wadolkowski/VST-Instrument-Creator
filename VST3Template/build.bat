@echo off
set VCPKG_ROOT=C:\vcpkg

:: 1. Czyszczenie starych plików
rmdir /s /q build
rmdir /s /q vcpkg_installed

:: 2. Instalacja zależności (zostaną pobrane i przygotowane przez manifest)
"C:\vcpkg\vcpkg.exe" install --triplet=x64-windows --x-manifest-root="%~dp0."

:: 3. Konfiguracja CMake
:: Usunięto zbędny PREFIX_PATH i MANIFEST_MODE, vcpkg.cmake sam zajmie się mapowaniem celów.
"C:\vcpkg\downloads\tools\cmake-4.4.4-windows\cmake-4.4.4-windows-x86_64\bin\cmake.exe" ^
  -S . ^
  -B build ^
  -G "Visual Studio 18 2026" ^
  -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
  -DVCPKG_TARGET_TRIPLET=x64-windows ^
  -DVCPKG_HOST_TRIPLET=x64-windows

:: 4. Kompilacja projektu
"C:\vcpkg\downloads\tools\cmake-4.4.4-windows\cmake-4.4.4-windows-x86_64\bin\cmake.exe" --build build --config Release
