---
wave: 1
depends_on: []
files_modified:
  - agent/build.gradle
  - agent/src/main/cpp/CMakeLists.txt
  - agent/src/main/cpp/*.cpp
  - agent/src/main/cpp/*.hpp
  - agent/src/main/cpp/icraw/*.hpp
  - agent/src/main/cpp/icraw/core/*.hpp
autonomous: true
---

# Phase 1: Build System & Agent Core - Plan

**Phase:** 01-build-system-agent-core
**Wave:** 1
**Dependencies:** None (first phase)

## Goal

C++ Agent engine can be compiled in Android NDK environment, supporting basic conversation loop and JNI communication. Build system generates `.so` library file.

## Requirements Coverage

| Requirement | Description | Status |
|-------------|-------------|--------|
| AGEN-01 | C++ Agent engine can compile in Android NDK environment | Must Have |
| AGEN-02 | Agent engine supports basic conversation loop (input -> process -> output) | Must Have |
| AGEN-03 | Agent engine can communicate with Java via JNI | Must Have |
| SYS-01 | Gradle build scripts include C++ compilation tasks | Must Have |
| SYS-02 | C++ code uses CMake build | Must Have |
| SYS-03 | Support arm64-v8a architecture | Must Have |

## Tasks

### Task 1: Configure agent/build.gradle with NDK/CMake support

**Description:** Add NDK configuration and externalNativeBuild to agent/build.gradle

**Files:**
- agent/build.gradle

**Actions:**
- Add ndk block with abiFilters for arm64-v8a only
- Add externalNativeBuild block with cmake configuration
- Set ndkVersion to "26.3.11579264"
- Add compileOptions for Java 21

**Verification:**
- agent/build.gradle contains ndk {} block
- agent/build.gradle contains externalNativeBuild {} block

---

### Task 2: Create agent CMakeLists.txt

**Description:** Create CMakeLists.txt in agent/src/main/cpp/ based on cxxplatform CMakeLists.txt

**Files:**
- agent/src/main/cpp/CMakeLists.txt

**Actions:**
- Create directory agent/src/main/cpp/
- Create CMakeLists.txt with cmake_minimum_required(VERSION 3.22.1)
- Configure project with C++17
- Set up find_package for dependencies (nlohmann_json, ZLIB, unofficial-sqlite3, CURL, spdlog)
- Define ICRAW_SOURCES with all source files
- Create shared library icraw
- Add Android-specific definitions (ICRAW_ANDROID)
- Link required libraries including log (Android logcat)

**Verification:**
- CMakeLists.txt exists at agent/src/main/cpp/CMakeLists.txt
- CMakeLists.txt contains all required dependencies
- CMakeLists.txt links to Android log library

---

### Task 3: Copy cxxplatform source files

**Description:** Copy source and header files from cxxplatform to agent module

**Files:**
- agent/src/main/cpp/include/icraw/*.hpp
- agent/src/main/cpp/include/icraw/core/*.hpp
- agent/src/main/cpp/src/*.cpp
- agent/src/main/cpp/src/core/*.cpp

**Actions:**
- Create agent/src/main/cpp/include/icraw/ directory
- Create agent/src/main/cpp/include/icraw/core/ directory
- Create agent/src/main/cpp/src/ directory
- Create agent/src/main/cpp/src/core/ directory
- Copy all header files from cxxplatform/include/icraw/
- Copy all source files from cxxplatform/src/

**Verification:**
- All header files copied to agent/src/main/cpp/include/
- All source files copied to agent/src/main/cpp/src/
- Directory structure matches cxxplatform

---

### Task 4: Create Android log sink for spdlog

**Description:** Create custom spdlog sink that redirects to Android logcat

**Files:**
- agent/src/main/cpp/android_log_sink.hpp

**Actions:**
- Create android_log_sink.hpp with custom sink class
- Inherit from spdlog::sinks::base_sink<std::mutex>
- Implement sink_it_ to use __android_log_print
- Implement flush_ (empty)

**Verification:**
- android_log_sink.hpp exists
- Contains class that extends spdlog::sinks::base_sink
- Uses ANDROID_LOG_DEBUG level

---

### Task 5: Update logger.cpp for Android

**Description:** Modify logger implementation to use Android log sink

**Files:**
- agent/src/main/cpp/src/logger.cpp

**Actions:**
- Update logger initialization to use android_log_sink on Android
- Keep file sink as fallback if needed
- Include android/log.h header

**Verification:**
- logger.cpp uses android_log_sink when ICRAW_ANDROID is defined
- Log output goes to Android logcat

---

### Task 6: Create minimal JNI entry point

**Description:** Create JNI binding layer with basic initialization

**Files:**
- agent/src/main/cpp/native_agent.cpp

**Actions:**
- Create native_agent.cpp with JNI exports
- Implement JNI_OnLoad for library initialization
- Create nativeGetVersion function returning version string
- Create nativeInitialize function for agent initialization

**Verification:**
- native_agent.cpp contains JNIEXPORT functions
- Functions follow Java_com_hh_agent_library_NativeAgent naming convention

---

### Task 7: Create NativeAgent Java class

**Description:** Create Java JNI wrapper class

**Files:**
- agent/src/main/java/com/hh/agent/library/NativeAgent.java

**Actions:**
- Create agent/src/main/java/com/hh/agent/library/ directory
- Create NativeAgent.java with native method declarations
- Add static System.loadLibrary() call

**Verification:**
- NativeAgent.java exists with native methods
- Package matches com.hh.agent.library

---

### Task 8: Test Gradle build

**Description:** Verify the project builds successfully with C++ compilation

**Actions:**
- Run ./gradlew :agent:assembleDebug
- Verify libicraw.so is generated in build/intermediates/cmake/

**Verification:**
- Build succeeds without errors
- .so file generated for at least arm64-v8a

---

## Implementation Notes

### Dependencies
- Use header-only for nlohmann-json and spdlog (no .so needed)
- sqlite3 and curl must be compiled as shared libraries via Conan
- Android log library (log) is required for logcat output

### ABI Support
- Only arm64-v8a ABI required
- ndk.abiFilters must include only arm64-v8a

### Build Output
- Library name: libicraw.so
- Location: agent/build/intermediates/cmake/debug/obj/{abi}/libicraw.so

---

## Verification Criteria

The following must be TRUE for Phase 1 completion:

1. **Gradle build succeeds** - `./gradlew :agent:assembleDebug` completes without errors
2. **C++ .so library generated** - libicraw.so exists in build output
3. **arm64-v8a ABI compiles** - Build succeeds for arm64-v8a
4. **Agent can initialize** - JNI nativeInitialize method callable from Java
5. **CMake integrated** - externalNativeBuild block in agent/build.gradle

---

## Must Haves (Goal-Backward Verification)

| Must Have | Verification Method |
|-----------|---------------------|
| C++ compiles in NDK | Gradle build succeeds |
| CMake used | CMakeLists.txt present in cpp directory |
| arm64-v8a ABI supported | ndk.abiFilters contains arm64-v8a |
| Basic conversation loop | Agent core code present (from cxxplatform) |
| JNI callable | NativeAgent.java with native methods exists |
| Log output to logcat | android_log_sink.hpp implemented |

---

*Plan created for Phase 1: Build System & Agent Core*
