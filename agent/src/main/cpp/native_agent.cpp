#include <jni.h>
#include <string>
#include <memory>
#include <curl/curl.h>
#include "icraw/core/logger.hpp"
#include "icraw/mobile_agent.hpp"
#include "icraw/config.hpp"
#include "icraw/android_tools.hpp"
#include <nlohmann/json.hpp>

// ICRAW_ANDROID is already defined by CMake, no need to redefine

// Global MobileAgent instance
static std::unique_ptr<icraw::MobileAgent> g_agent;

// Global JNI environment for callback invocations
static JavaVM* g_jvm = nullptr;
static jobject g_callback_object = nullptr;

extern "C" {

/**
 * JNI OnLoad - Called when the native library is loaded
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // Store JavaVM reference for callback invocations
    g_jvm = vm;

    // Initialize curl globally (required before using curl_easy_init)
    CURLcode curl_res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_res != CURLE_OK) {
        // Cannot use logger yet, but continue anyway
    }

    // Initialize the logger (Android uses logcat, directory is ignored)
    icraw::Logger::get_instance().init("", "debug");

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
 * Returns: 0 on success, -1 on failure
 */
JNIEXPORT jint JNICALL Java_com_hh_agent_library_NativeAgent_nativeInitialize(
        JNIEnv* env,
        jclass /* clazz */,
        jstring configJsonStr) {
    // Get config JSON from Java
    const char* config_json = nullptr;
    if (configJsonStr) {
        config_json = env->GetStringUTFChars(configJsonStr, nullptr);
    }

    icraw::Logger::get_instance().logger()->info("nativeInitialize: Starting initialization");

    // Create MobileAgent with config
    try {
        icraw::IcrawConfig config;

        // Parse JSON config if provided
        if (config_json && strlen(config_json) > 0) {
            try {
                auto json = nlohmann::json::parse(config_json);
                if (json.contains("provider")) {
                    auto& provider = json["provider"];
                    if (provider.contains("apiKey")) {
                        config.provider.api_key = provider["apiKey"].get<std::string>();
                    }
                    if (provider.contains("baseUrl")) {
                        config.provider.base_url = provider["baseUrl"].get<std::string>();
                    }
                }
                if (json.contains("agent") && json["agent"].contains("model")) {
                    config.agent.model = json["agent"]["model"].get<std::string>();
                }
                // Parse workspace path from JSON
                if (json.contains("workspacePath")) {
                    config.workspace_path = json["workspacePath"].get<std::string>();
                }
                // Ensure workspace path is set (load_default sets the default path)
        icraw::IcrawConfig default_config = icraw::IcrawConfig::load_default();
        if (config.workspace_path.empty()) {
            config.workspace_path = default_config.workspace_path;
        }

        icraw::Logger::get_instance().logger()->info("Loaded config from JSON: apiKey set={}, baseUrl={}, model={}, workspace={}",
                    !config.provider.api_key.empty(), config.provider.base_url, config.agent.model, config.workspace_path.string());
            } catch (const std::exception& e) {
                icraw::Logger::get_instance().logger()->warn("Failed to parse config JSON: {}", e.what());
                config = icraw::IcrawConfig::load_default();
            }
        } else {
            icraw::Logger::get_instance().logger()->info("No config JSON provided, using defaults");
            config = icraw::IcrawConfig::load_default();
        }

        icraw::Logger::get_instance().logger()->info("Creating MobileAgent with config: model={}, workspace={}",
            config.agent.model, config.workspace_path.string());

        g_agent = icraw::MobileAgent::create_with_config(config);

        icraw::Logger::get_instance().logger()->info(
            "NativeAgent initialized successfully");
    } catch (const std::exception& e) {
        icraw::Logger::get_instance().logger()->error(
            "Failed to initialize NativeAgent: {}", e.what());
        // Return error to Java instead of silently failing
        if (config_json) {
            env->ReleaseStringUTFChars(configJsonStr, config_json);
        }
        return -1;  // Return error code
    }

    if (config_json) {
        env->ReleaseStringUTFChars(configJsonStr, config_json);
    }
    return 0;  // Success
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

/**
 * Call an Android tool
 * Returns JSON string with result: {"success": true, "result": ...} or {"success": false, "error": "..."}
 */
JNIEXPORT jstring JNICALL Java_com_hh_agent_library_NativeAgent_nativeCallAndroidTool(
        JNIEnv* env,
        jclass /* clazz */,
        jstring toolName,
        jstring argsJson) {
    const char* tool_name = env->GetStringUTFChars(toolName, nullptr);
    const char* args_json = env->GetStringUTFChars(argsJson, nullptr);

    std::string result;

    if (!tool_name || !args_json) {
        nlohmann::json error_result;
        error_result["success"] = false;
        error_result["error"] = "invalid_arguments";
        result = error_result.dump();
    } else {
        try {
            auto args = nlohmann::json::parse(args_json);
            result = icraw::g_android_tools.call_tool(tool_name, args);
        } catch (const std::exception& e) {
            nlohmann::json error_result;
            error_result["success"] = false;
            error_result["error"] = "invalid_args";
            error_result["message"] = e.what();
            result = error_result.dump();
        }
    }

    env->ReleaseStringUTFChars(toolName, tool_name);
    env->ReleaseStringUTFChars(argsJson, args_json);

    return env->NewStringUTF(result.c_str());
}

/**
 * Register Android tool callback from Java
 * This creates a JNI callback that delegates to the Java AndroidToolCallback interface
 */
JNIEXPORT void JNICALL Java_com_hh_agent_library_NativeAgent_nativeRegisterAndroidToolCallback(
        JNIEnv* env,
        jclass /* clazz */,
        jobject callback) {

    // Delete previous global reference if exists
    if (g_callback_object) {
        env->DeleteGlobalRef(g_callback_object);
        g_callback_object = nullptr;
    }

    if (!callback) {
        icraw::Logger::get_instance().logger()->info("AndroidToolCallback unregistered");
        return;
    }

    // Create global reference to keep the callback object alive
    g_callback_object = env->NewGlobalRef(callback);

    // Create a C++ callback that delegates to Java
    class JniCallback : public icraw::AndroidToolCallback {
    public:
        JniCallback(JNIEnv* e, jobject o) : env_(e), callback_(o) {
            // Get the method ID
            jclass cls = env_->GetObjectClass(callback_);
            method_id_ = env_->GetMethodID(cls, "callTool",
                "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        }

        std::string call_tool(const std::string& tool_name, const nlohmann::json& args) override {
            if (!callback_ || !method_id_) {
                nlohmann::json error;
                error["success"] = false;
                error["error"] = "callback_not_available";
                return error.dump();
            }

            jstring j_tool_name = env_->NewStringUTF(tool_name.c_str());
            jstring j_args = env_->NewStringUTF(args.dump().c_str());

            jstring j_result = (jstring)env_->CallObjectMethod(callback_, method_id_, j_tool_name, j_args);

            std::string result;
            if (j_result) {
                const char* result_str = env_->GetStringUTFChars(j_result, nullptr);
                result = result_str;
                env_->ReleaseStringUTFChars(j_result, result_str);
            } else {
                nlohmann::json error;
                error["success"] = false;
                error["error"] = "callback_failed";
                result = error.dump();
            }

            env_->DeleteLocalRef(j_tool_name);
            env_->DeleteLocalRef(j_args);
            if (j_result) env_->DeleteLocalRef(j_result);

            return result;
        }

    private:
        JNIEnv* env_;
        jobject callback_;
        jmethodID method_id_;
    };

    // Register the JNI callback
    icraw::g_android_tools.register_callback(std::make_unique<JniCallback>(env, g_callback_object));

    icraw::Logger::get_instance().logger()->info("AndroidToolCallback registered via JNI");
}

} // extern "C"
