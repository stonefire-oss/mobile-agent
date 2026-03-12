cmake_minimum_required(VERSION 3.16)
project(icraw VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(ICRAW_BUILD_SHARED "Build shared library" OFF)

# ========== MNN 条件编译选项 ==========
option(ICRAW_USE_MNN "Enable MNN local inference" OFF)
option(ICRAW_MNN_OPENCL "Enable MNN OpenCL backend (Android)" OFF)
option(ICRAW_MNN_METAL "Enable MNN Metal backend (iOS)" OFF)

if(ICRAW_USE_MNN)
    message(STATUS "MNN local inference enabled")
    message(STATUS "  OpenCL backend: ${ICRAW_MNN_OPENCL}")
    message(STATUS "  Metal backend: ${ICRAW_MNN_METAL}")
else()
    message(STATUS "MNN local inference disabled")
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# vcpkg dependencies via find_package
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# ========== MNN 依赖处理 ==========
set(ICRAW_MNN_AVAILABLE OFF)
if(ICRAW_USE_MNN)
    # 尝试查找已安装的MNN
    find_package(MNN CONFIG QUIET)
    
    if(MNN_FOUND)
        set(ICRAW_MNN_AVAILABLE ON)
        message(STATUS "MNN library found")
    else()
        message(STATUS "MNN not found via find_package, will need manual setup")
        message(WARNING "MNN library not found. ICRAW_USE_MNN will be disabled.")
        message(STATUS "To enable MNN, please install MNN or set MNN_DIR cmake variable")
    endif()
endif()

# Main library
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
    src/core/mcp_client.cpp
    src/core/token_utils.cpp
    src/core/provider_manager.cpp
    src/tools/tool_registry.cpp
    src/mobile_agent.cpp
)

# MNN provider（条件编译）
# 只有当MNN库真正找到时才添加源文件
if(ICRAW_MNN_AVAILABLE)
    list(APPEND ICRAW_SOURCES
        src/core/mnn_provider.cpp
    )
endif()

set(ICRAW_HEADERS
    include/icraw/core/logger.hpp
    include/icraw/core/content_block.hpp
    include/icraw/core/memory_manager.hpp
    include/icraw/core/skill_loader.hpp
    include/icraw/core/prompt_builder.hpp
    include/icraw/core/agent_loop.hpp
    include/icraw/core/llm_provider.hpp
    include/icraw/core/http_client.hpp
    include/icraw/core/mcp_client.hpp
    include/icraw/core/token_utils.hpp
    include/icraw/core/provider_manager.hpp
    include/icraw/tools/tool_registry.hpp
    include/icraw/config.hpp
    include/icraw/mobile_agent.hpp
    include/icraw/types.hpp
)

# MNN provider头文件（条件编译）
if(ICRAW_MNN_AVAILABLE)
    list(APPEND ICRAW_HEADERS
        include/icraw/core/mnn_provider.hpp
    )
endif()

if(ICRAW_BUILD_SHARED)
    add_library(icraw SHARED ${ICRAW_SOURCES})
else()
    add_library(icraw STATIC ${ICRAW_SOURCES})
endif()

target_include_directories(icraw
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(icraw PUBLIC 
    nlohmann_json::nlohmann_json 
    unofficial::sqlite3::sqlite3
    CURL::libcurl
    spdlog::spdlog
)

# ========== MNN 链接和编译定义 ==========
if(ICRAW_MNN_AVAILABLE)
    # 添加编译定义
    target_compile_definitions(icraw PUBLIC ICRAW_USE_MNN)
    
    # 链接MNN库
    target_link_libraries(icraw PUBLIC MNN::MNN)
    message(STATUS "Linking with MNN library")
    
    # OpenCL后端（Android）
    if(ICRAW_MNN_OPENCL)
        target_compile_definitions(icraw PUBLIC ICRAW_MNN_OPENCL)
    endif()
    
    # Metal后端（iOS）
    if(ICRAW_MNN_METAL)
        target_compile_definitions(icraw PUBLIC ICRAW_MNN_METAL)
    endif()
elseif(ICRAW_USE_MNN)
    message(WARNING "MNN requested but not found. Building without MNN support.")
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(icraw PRIVATE _WIN32_WINNT=0x0601)
elseif(ANDROID)
    target_compile_definitions(icraw PRIVATE ICRAW_ANDROID)
elseif(IOS)
    target_compile_definitions(icraw PRIVATE ICRAW_IOS)
endif()

# Demo
option(BUILD_DEMO "Build demo application" ON)
if(BUILD_DEMO)
    add_subdirectory(demo)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
install(TARGETS icraw
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/icraw DESTINATION include)
