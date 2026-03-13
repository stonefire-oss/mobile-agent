// 工具调用测试程序
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <icraw/core/mnn_provider.hpp>
#include <icraw/config.hpp>
#include <nlohmann/json.hpp>

#ifdef ICRAW_USE_MNN

void print_usage(const char* program_name) {
    std::cout << "MNN Provider Tool Calling Test\n\n";
    std::cout << "Usage: " << program_name << " <model_path>\n";
    std::cout << "  model_path  - Path to MNN model directory\n\n";
    std::cout << "Example:\n";
    std::cout << "  " << program_name << " ./Qwen2.5-0.5B-Instruct-MNN\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string model_path = argv[1];

    std::cout << "MNN Tool Calling Test\n";
    std::cout << "Model: " << model_path << "\n\n";

    try {
        icraw::MNNConfig config;
        config.model_path = model_path;
        config.thread_num = 4;
        config.backend = "cpu";

        std::cout << "Creating MNN Provider...\n";
        icraw::MNNProvider provider(config);

        if (!provider.is_model_loaded()) {
            std::cerr << "Error: Failed to load model\n";
            return 1;
        }

        std::cout << "Model loaded successfully!\n";
        std::cout << "Provider: " << provider.get_provider_name() << "\n\n";

        // 测试: call_android_tool
        std::cout << "========================================\n";
        std::cout << "Test: call_android_tool\n";
        std::cout << "========================================\n\n";
        
        icraw::ChatCompletionRequest request;
        request.messages.push_back(icraw::Message("user", "Call the list_files tool to list files in the current directory"));
        request.max_tokens = 256;

        std::cout << "User: Call the list_files tool to list files in the current directory\n\n";
        std::cout << "Assistant: ";
        auto response = provider.chat_completion(request);
        
        if (!response.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response.content << "\n";
        }
        std::cout << "Finish reason: " << response.finish_reason << "\n";

        std::cout << "\n========================================\n";
        std::cout << "Test completed!\n";
        std::cout << "========================================\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

#else

int main() {
    std::cerr << "Error: MNN support not compiled in.\n";
    return 1;
}

#endif