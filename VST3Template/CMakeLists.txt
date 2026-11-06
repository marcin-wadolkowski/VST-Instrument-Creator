cmake_minimum_required(VERSION 3.21)
project(VST3Template VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point this at the header created by SynthCodeGen. The header is deliberately
# an explicit input: the plug-in must compile against the instrument definition.
set(GENERATED_PARAMETERS_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/GeneratedParameters.h"
    CACHE FILEPATH "Path to generated GeneratedParameters.h")

if(NOT EXISTS "${GENERATED_PARAMETERS_HEADER}")
    message(FATAL_ERROR
        "GeneratedParameters.h not found: ${GENERATED_PARAMETERS_HEADER}\\n"
        "Generate it first with SynthCodeGen <instrument.dat> <output.h>, "
        "or set -DGENERATED_PARAMETERS_HEADER=<path>.")
endif()

get_filename_component(GENERATED_PARAMETERS_DIR
    "${GENERATED_PARAMETERS_HEADER}" DIRECTORY)

add_library(VST3Template SHARED
    src/plugincore.h
    src/plugincore.cpp
)

set_target_properties(VST3Template PROPERTIES
    PREFIX ""
    SUFFIX ".vst3"
)

target_include_directories(VST3Template PRIVATE
    src
    "${GENERATED_PARAMETERS_DIR}"
    "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include/vst3sdk"
    "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include"
)

target_link_libraries(VST3Template PRIVATE
    sdk
    vstgui_support
)

if(MSVC)
    target_compile_options(VST3Template PRIVATE /W4)
else()
    target_compile_options(VST3Template PRIVATE -Wall -Wextra -Wpedantic)
endif()
