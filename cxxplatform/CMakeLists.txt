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

# MNN目录路径（手动指定）
set(MNN_DIR "" CACHE PATH "Path to MNN source directory")

if(ICRAW_USE_MNN)
    message(STATUS "MNN local inference enabled")
    message(STATUS "  OpenCL backend: ${ICRAW_MNN_OPENCL}")
    message(STATUS "  Metal backend: ${ICRAW_MNN_METAL}")
    message(STATUS "  MNN_DIR: ${MNN_DIR}")
    message(STATUS "")
    message(STATUS "  NOTE: MNN must be compiled with -DMNN_BUILD_LLM=true to enable LLM support")
    message(STATUS "        Example: cmake .. -DMNN_BUILD_LLM=true -DMNN_SUPPORT_TRANSFORMER_FUSE=true")
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

# 保存用户指定的MNN_DIR（因为find_package可能会覆盖它）
set(USER_MNN_DIR "${MNN_DIR}")

if(ICRAW_USE_MNN)
    # 尝试查找已安装的MNN
    find_package(MNN CONFIG QUIET)
    
    if(MNN_FOUND)
        set(ICRAW_MNN_AVAILABLE ON)
        message(STATUS "MNN library found via find_package")
    elseif(NOT "${USER_MNN_DIR}" STREQUAL "" AND EXISTS "${USER_MNN_DIR}")
        # 手动指定MNN目录
        message(STATUS "Using MNN from: ${USER_MNN_DIR}")
        
        # 检查MNN头文件
        if(EXISTS "${USER_MNN_DIR}/include/MNN/MNNDefine.h")
            set(MNN_INCLUDE_DIR "${USER_MNN_DIR}/include")
            message(STATUS "MNN headers found at ${MNN_INCLUDE_DIR}")
        else()
            message(FATAL_ERROR "MNN headers not found at ${USER_MNN_DIR}/include")
        endif()
        
        # 检查MNN库文件
        if(EXISTS "${USER_MNN_DIR}/build/Release/MNN.lib")
            set(MNN_LIB_DIR "${USER_MNN_DIR}/build/Release")
        elseif(EXISTS "${USER_MNN_DIR}/build/MNN.lib")
            set(MNN_LIB_DIR "${USER_MNN_DIR}/build")
        else()
            message(FATAL_ERROR "MNN library not found at ${USER_MNN_DIR}/build")
        endif()
        
        # 恢复MNN_DIR为用户指定的值
        set(MNN_DIR "${USER_MNN_DIR}")
        set(ICRAW_MNN_AVAILABLE ON)
        message(STATUS "MNN library found at ${MNN_LIB_DIR}")
    else()
        message(STATUS "MNN not found via find_package and MNN_DIR not set or invalid")
        message(WARNING "MNN library not found. ICRAW_USE_MNN will be disabled.")
        message(STATUS "To enable MNN, set MNN_DIR cmake variable:")
        message(STATUS "  cmake .. -DICRAW_USE_MNN=ON -DMNN_DIR=/path/to/MNN")
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
    
    if(MNN_FOUND)
        # 使用find_package找到MNN
        target_link_libraries(icraw PUBLIC MNN::MNN)
        message(STATUS "Linking with MNN library (via find_package)")
        
        if(TARGET MNN::MNN_LLM)
            target_link_libraries(icraw PUBLIC MNN::MNN_LLM)
            message(STATUS "Linking with MNN-LLM library")
        endif()
    elseif(DEFINED MNN_DIR)
        # 使用手动指定的MNN路径
        # 添加MNN头文件路径
        target_include_directories(icraw PUBLIC
            "${MNN_DIR}/include"
            "${MNN_DIR}/transformers/llm/engine/include"
            "${MNN_DIR}/source"
            "${MNN_DIR}/express"
        )
        
        # 链接MNN库
        target_link_libraries(icraw PUBLIC
            "${MNN_LIB_DIR}/MNN.lib"
        )
        
        # 添加LLM静态库
        if(EXISTS "${MNN_DIR}/build/llm.dir/Release/llm.lib")
            target_link_libraries(icraw PUBLIC
                "${MNN_DIR}/build/llm.dir/Release/llm.lib"
            )
            message(STATUS "Linking with MNN-LLM library")
        endif()
        
        message(STATUS "Linking with MNN library from ${MNN_DIR}")
    endif()
    
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

# MNN Test executable (only when MNN is available)
if(ICRAW_MNN_AVAILABLE)
    add_executable(mnn_test tests/mnn_test.cpp)
    target_link_libraries(mnn_test PRIVATE icraw)
    
    add_executable(mnn_tool_test tests/mnn_tool_test.cpp)
    target_link_libraries(mnn_tool_test PRIVATE icraw)
    
    add_executable(mnn_full_tool_test tests/mnn_full_tool_test.cpp)
    target_link_libraries(mnn_full_tool_test PRIVATE icraw)
    
    add_executable(mnn_interactive tests/mnn_interactive.cpp)
    target_link_libraries(mnn_interactive PRIVATE icraw)
    
    message(STATUS "MNN test executables will be built")
endif()

# Install
install(TARGETS icraw
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/icraw DESTINATION include)
