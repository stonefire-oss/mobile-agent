package com.hh.agent.library;

/**
 * Native agent JNI wrapper class
 * Provides Java interface to the C++ agent engine
 */
public class NativeAgent {

    // Callback instance registered by Java layer
    private static AndroidToolCallback sCallback;

    static {
        System.loadLibrary("icraw");
    }

    /**
     * Get the native agent version
     *
     * @return Version string
     */
    public static native String nativeGetVersion();

    /**
     * Initialize the native agent
     *
     * @param configJson JSON configuration string
     * @return 0 on success, -1 on failure
     */
    public static native int nativeInitialize(String configJson);

    /**
     * Send a message to the agent and get a response
     *
     * @param message The message to send
     * @return The agent's response
     */
    public static native String nativeSendMessage(String message);

    /**
     * Call an Android tool
     *
     * @param toolName The name of the tool to call
     * @param argsJson JSON string containing tool arguments
     * @return JSON string with result
     */
    public static native String nativeCallAndroidTool(String toolName, String argsJson);

    /**
     * Shutdown the native agent
     */
    public static native void nativeShutdown();

    /**
     * Register an Android tool callback
     *
     * @param callback The callback implementation
     */
    public static void registerAndroidToolCallback(AndroidToolCallback callback) {
        sCallback = callback;
        // Call native method to register callback in C++ layer
        nativeRegisterAndroidToolCallback(callback);
    }

    /**
     * Native method to register Android tool callback in C++ layer
     */
    private static native void nativeRegisterAndroidToolCallback(AndroidToolCallback callback);

    /**
     * Get the registered Android tool callback
     *
     * @return The callback or null if not registered
     */
    public static AndroidToolCallback getAndroidToolCallback() {
        return sCallback;
    }

    /**
     * Set tools schema from JSON string
     * This allows Java to pass tools.json content to C++ for tool registration
     *
     * @param schemaJson JSON string containing tools schema
     */
    public static native void nativeSetToolsSchema(String schemaJson);

    // ========== Provider管理接口 ==========

    /**
     * Provider类型常量
     */
    public static final int PROVIDER_REMOTE = 0;
    public static final int PROVIDER_MNN = 2;

    /**
     * 获取当前Provider类型
     *
     * @return Provider类型 (PROVIDER_REMOTE 或 PROVIDER_MNN)
     */
    public static native int nativeGetCurrentProviderType();

    /**
     * 切换到远程API Provider
     *
     * @return true表示成功， false表示失败
     */
    public static native boolean nativeSwitchToRemote();

    /**
     * 检查MNN是否可用（编译期）
     *
     * @return true表示MNN可用， false表示不可用
     */
    public static native boolean nativeIsMNNAvailable();

    /**
     * 获取当前Provider的状态信息
     *
     * @return JSON字符串，包含provider类型、名称、是否就绪等信息
     */
    public static native String nativeGetProviderStatus();

    // ========== 便捷方法 ==========

    /**
     * 切换到远程API Provider
     *
     * @return true表示成功
     */
    public static boolean switchToRemoteProvider() {
        return nativeSwitchToRemote();
    }

    /**
     * 检查MNN是否可用
     *
     * @return true表示MNN已编译且可用
     */
    public static boolean isMNNAvailable() {
        return nativeIsMNNAvailable();
    }

    /**
     * 获取当前Provider类型
     *
     * @return PROVIDER_REMOTE 或 PROVIDER_MNN
     */
    public static int getCurrentProviderType() {
        return nativeGetCurrentProviderType();
    }
}
