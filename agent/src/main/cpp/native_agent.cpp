#include <jni.h>
#include <string>
#include <memory>
#include "icraw/core/logger.hpp"
#include "icraw/mobile_agent.hpp"
#include "icraw/config.hpp"

// Define ICRAW_ANDROID for Android build
#define ICRAW_ANDROID

// Global MobileAgent instance
static std::unique_ptr<icraw::MobileAgent> g_agent;

extern "C" {

/**
 * JNI OnLoad - Called when the native library is loaded
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // Initialize the logger
    icraw::Logger::get_instance().init("/sdcard/logs", "debug");

    return JNI_VERSION_1_6;
}

/**
 * Get the native agent version
 */
JNIEXPORT jstring JNICALL Java_com_hh_agent_library_NativeAgent_nativeGetVersion(
        JNIEnv* env,
        jclass /* clazz */) {
    std::string version = "1.0.0-native";
    return env->NewStringUTF(version.c_str());
}

/**
 * Initialize the native agent
 */
JNIEXPORT void JNICALL Java_com_hh_agent_library_NativeAgent_nativeInitialize(
        JNIEnv* env,
        jclass /* clazz */,
        jstring configPath) {
    // Get workspace path from Java
    const char* workspace_path = nullptr;
    if (configPath) {
        workspace_path = env->GetStringUTFChars(configPath, nullptr);
    }

    // Create MobileAgent with default config
    try {
        auto config = icraw::IcrawConfig::load_default();
        g_agent = icraw::MobileAgent::create_with_config(config);

        icraw::Logger::get_instance().logger()->info(
            "NativeAgent initialized successfully with workspace: {}",
            workspace_path ? workspace_path : "(default)");
    } catch (const std::exception& e) {
        icraw::Logger::get_instance().logger()->error(
            "Failed to initialize NativeAgent: {}", e.what());
    }

    if (workspace_path) {
        env->ReleaseStringUTFChars(configPath, workspace_path);
    }
}

/**
 * Send a message to the agent and get a response
 */
JNIEXPORT jstring JNICALL Java_com_hh_agent_library_NativeAgent_nativeSendMessage(
        JNIEnv* env,
        jclass /* clazz */,
        jstring message) {
    const char* msg = env->GetStringUTFChars(message, nullptr);
    std::string response;

    if (!msg) {
        return env->NewStringUTF("");
    }

    icraw::Logger::get_instance().logger()->debug("Received message: {}", msg);

    // Check if agent is initialized
    if (!g_agent) {
        icraw::Logger::get_instance().logger()->warn("Agent not initialized, returning error");
        response = "Error: Agent not initialized. Call nativeInitialize first.";
    } else {
        try {
            // Call MobileAgent::chat()
            response = g_agent->chat(msg);
            icraw::Logger::get_instance().logger()->debug("Agent response: {}", response);
        } catch (const std::exception& e) {
            icraw::Logger::get_instance().logger()->error("Agent chat failed: {}", e.what());
            response = std::string("Error: ") + e.what();
        }
    }

    env->ReleaseStringUTFChars(message, msg);
    return env->NewStringUTF(response.c_str());
}

/**
 * Shutdown the native agent
 */
JNIEXPORT void JNICALL Java_com_hh_agent_library_NativeAgent_nativeShutdown(
        JNIEnv* env,
        jclass /* clazz */) {
    icraw::Logger::get_instance().logger()->info("Shutting down NativeAgent");

    // Clean up MobileAgent instance
    if (g_agent) {
        g_agent->stop();
        g_agent.reset();
        icraw::Logger::get_instance().logger()->info("MobileAgent destroyed");
    }
}

} // extern "C"
