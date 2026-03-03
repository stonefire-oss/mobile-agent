cmake_minimum_required(VERSION 3.22.1)
project(icraw VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Android-specific settings
if(ANDROID)
    # Conan dependencies location
    set(CONAN_DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/armv8/Release/generators")

    # Add Conan generated files to CMake module path
    list(APPEND CMAKE_MODULE_PATH "${CONAN_DEPENDENCIES_DIR}")
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Conan dependencies via find_package (from agent/build/armv8/Release/generators)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Main library sources
set(ICRAW_SOURCES
    src/config.cpp
    src/logger.cpp
    src/core/content_block.cpp
    src/core/memory_manager.cpp
    src/core/skill_loader.cpp
    src/core/prompt_builder.cpp
    src/core/agent_loop.cpp
    src/core/llm_provider.cpp
    src/core/curl_http_client.cpp
    src/core/token_utils.cpp
    src/tools/tool_registry.cpp
    src/mobile_agent.cpp
)

# Headers for IDE navigation
set(ICRAW_HEADERS
    include/icraw/core/logger.hpp
    include/icraw/core/content_block.hpp
    include/icraw/core/memory_manager.hpp
    include/icraw/core/skill_loader.hpp
    include/icraw/core/prompt_builder.hpp
    include/icraw/core/agent_loop.hpp
    include/icraw/core/llm_provider.hpp
    include/icraw/core/http_client.hpp
    include/icraw/core/token_utils.hpp
    include/icraw/tools/tool_registry.hpp
    include/icraw/config.hpp
    include/icraw/mobile_agent.hpp
    include/icraw/types.hpp
)

# Create shared library
add_library(icraw SHARED ${ICRAW_SOURCES} ${ICRAW_HEADERS})

target_include_directories(icraw
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies (using SQLite3 from Conan, not unofficial-sqlite3)
target_link_libraries(icraw PUBLIC
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    CURL::libcurl
    spdlog::spdlog
    ZLIB::ZLIB
)

# Platform-specific settings
if(ANDROID)
    target_compile_definitions(icraw PUBLIC ICRAW_ANDROID)
    target_link_libraries(icraw PUBLIC log)
endif()

# Install (optional for Android)
install(TARGETS icraw
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
