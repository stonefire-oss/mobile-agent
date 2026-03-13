// MNN Provider 测试程序
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <icraw/core/mnn_provider.hpp>
#include <icraw/config.hpp>

#ifdef ICRAW_USE_MNN

void print_usage(const char* program_name) {
    std::cout << "MNN Provider Test\n\n";
    std::cout << "Usage: " << program_name << " <model_path> [prompt]\n";
    std::cout << "  model_path  - Path to MNN model directory (containing config.json)\n";
    std::cout << "  prompt      - Test prompt (default: 'Hello, who are you?')\n\n";
    std::cout << "Example:\n";
    std::cout << "  " << program_name << " ./Qwen2.5-0.5B-Instruct-MNN \"What is 2+2?\"\n";
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
    std::string prompt = "Hello, who are you?";
    if (argc >= 3) {
        prompt = argv[2];
    }

    std::cout << "========================================\n";
    std::cout << "MNN Provider Test\n";
    std::cout << "========================================\n\n";
    std::cout << "Model path: " << model_path << "\n";
    std::cout << "Prompt: " << prompt << "\n\n";

    try {
        // 创建 MNN 配置
        icraw::MNNConfig config;
        config.model_path = model_path;
        config.thread_num = 4;
        config.backend = "cpu";
        config.use_mmap = true;

        std::cout << "Creating MNN Provider...\n";
        icraw::MNNProvider provider(config);

        if (!provider.is_model_loaded()) {
            std::cerr << "Error: Failed to load MNN model\n";
            return 1;
        }

        std::cout << "Model loaded successfully!\n";
        std::cout << "Provider name: " << provider.get_provider_name() << "\n\n";

        // 创建请求
        icraw::ChatCompletionRequest request;
        request.messages.push_back(icraw::Message("user", prompt));
        request.max_tokens = 256;
        request.temperature = 0.7;

        std::cout << "========================================\n";
        std::cout << "Testing non-streaming response...\n";
        std::cout << "========================================\n\n";

        auto response = provider.chat_completion(request);
        std::cout << "Response: " << response.content << "\n";
        std::cout << "Finish reason: " << response.finish_reason << "\n\n";

        std::cout << "========================================\n";
        std::cout << "Testing streaming response...\n";
        std::cout << "========================================\n\n";

        std::cout << "Response: ";
        provider.chat_completion_stream(request, [](const icraw::ChatCompletionResponse& chunk) {
            if (!chunk.content.empty()) {
                std::cout << chunk.content << std::flush;
            }
            if (chunk.is_stream_end) {
                std::cout << "\n[Stream ended: " << chunk.finish_reason << "]\n";
            }
        });

        std::cout << "\n========================================\n";
        std::cout << "Test completed successfully!\n";
        std::cout << "========================================\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

#else // ICRAW_USE_MNN

int main(int argc, char* argv[]) {
    std::cerr << "Error: MNN support not compiled in.\n";
    std::cerr << "Rebuild with -DICRAW_USE_MNN=ON\n";
    return 1;
}

#endif // ICRAW_USE_MNN