# Technology Stack

**Analysis Date:** 2026-03-03

## Languages

**Primary:**
- Java 21 - Android application development (app, lib modules)
- C++ - Native library for NDK integration

**Secondary:**
- XML - Android resource files (layouts, values, configurations)
- Gradle - Build configuration (Groovy-based DSL)

## Runtime

**Environment:**
- Android SDK - API level 24 (minSdk) to 34 (compileSdk)
- Java 21 - Source and target compatibility
- NDK 26.3.11579264 - Native development

**Package Manager:**
- Gradle 8.12.1 - Build automation
- Android Gradle Plugin 8.3.2 - Android-specific build tasks
- Lockfile: `gradle/wrapper/gradle-wrapper.properties`

## Frameworks

**Core:**
- AndroidX AppCompat 1.6.1 - Backward-compatible UI components
- Material Components 1.11.0 - Material Design UI
- AndroidX RecyclerView - Message list display
- AndroidX ConstraintLayout - Layout management

**Markdown Rendering:**
- Markwon Core 4.6.2 - Markdown to Spannable conversion
- Markwon Ext Strikethrough 4.6.2 - Strikethrough support
- Markwon Ext Tables 4.6.2 - Table rendering
- Markwon Ext Tasklist 4.6.2 - Task list (checkbox) support

**HTTP:**
- OkHttp 4.12.0 - HTTP client for Nanobot API communication

**JSON:**
- Gson 2.10.1 - JSON serialization/deserialization

**Native:**
- CMake 3.22.1 - C++ build system
- Android Log library - Native logging

**Testing:**
- JUnit 4.13.2 - Unit testing framework
- OkHttp MockWebServer 4.12.0 - HTTP mocking for tests
- AndroidX Test Runner - Android instrumentation testing
- Espresso 3.5.1 - UI testing framework

## Key Dependencies

**Critical:**
- OkHttp 4.12.0 - HTTP client for Nanobot API calls
- Gson 2.10.1 - JSON serialization for request/response DTOs
- Markwon 4.6.2 - Markdown rendering for assistant messages
- Material 1.11.0 - UI components for chat interface

**Infrastructure:**
- AndroidX AppCompat 1.6.1 - Core Android compatibility
- AndroidX RecyclerView - Efficient message list rendering

## Configuration

**Environment:**
- Gradle properties: `gradle.properties`
  - JVM args: `-Xmx2048m -Dfile.encoding=UTF-8`
  - AndroidX enabled: `android.useAndroidX=true`
  - Non-transitive R class: `android.nonTransitiveRClass=true`
- Local properties: `local.properties` (SDK path)
- Network security config: `app/src/main/res/xml/network_security_config.xml`
  - Allows cleartext to localhost and 10.0.2.2 (Android emulator host)

**Build:**
- Root `build.gradle` - Plugin definitions
- `app/build.gradle` - App module configuration
- `lib/build.gradle` - Library module with NDK/CMake config
- `agent/build.gradle` - Agent library module
- `settings.gradle` - Multi-module project setup

## Platform Requirements

**Development:**
- Android Studio or Gradle-compatible IDE
- Java 21 JDK
- Android SDK API 34
- NDK 26.3.11579264
- CMake 3.22.1

**Production:**
- Android device or emulator (minSdk 24, targetSdk 31)
- Nanobot HTTP service running on localhost:18791
- ADB reverse for emulator: `adb reverse tcp:18791 tcp:18791`

---

*Stack analysis: 2026-03-03*
