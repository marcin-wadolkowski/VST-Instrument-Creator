cmake_minimum_required(VERSION 3.21)
project(VST3Template VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Wyszukanie pakietów
# Usunięto find_package(vst3sdk), ponieważ vcpkg automatycznie mapuje jego pliki nagłówkowe.
find_package(nlohmann_json CONFIG REQUIRED)

# 2. Definicja projektu wtyczki
add_library(VST3Template SHARED 
    src/plugincore.h 
    src/plugincore.cpp
)

# Wymuszenie rozszerzenia .vst3 zamiast .dll
set_target_properties(VST3Template PROPERTIES 
    PREFIX "" 
    SUFFIX ".vst3"
)

# POPRAWKA: Dodajemy ścieżkę do nagłówków VST3 SDK dostarczanych przez vcpkg
# ${CMAKE_PREFIX_PATH} w trybie manifestu wskazuje na folder vcpkg_installed/x64-windows
target_include_directories(VST3Template PRIVATE 
    src
    "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include/vst3sdk"
    "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include"
)


# 3. Linkowanie celów zarejestrowanych przez vcpkg
# Zamiast przestrzeni nazw vst3sdk:: używamy bezpośrednich nazw bibliotek wygenerowanych przez vcpkg.
target_link_libraries(VST3Template PRIVATE 
    sdk 
    vstgui_support 
    nlohmann_json::nlohmann_json
)
