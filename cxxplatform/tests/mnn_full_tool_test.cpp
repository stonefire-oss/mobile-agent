// 完整工具调用测试程序
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <icraw/core/mnn_provider.hpp>
#include <icraw/config.hpp>
#include <nlohmann/json.hpp>

#ifdef ICRAW_USE_MNN

void print_separator(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path>\n";
        return 1;
    }

    std::string model_path = argv[1];

    std::cout << "MNN Full Tool Calling Test\n";
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
        std::cout << "Multimodal: " << (provider.is_multimodal() ? "Yes" : "No") << "\n";

        // 测试1: read_file
        print_separator("Test 1: read_file Tool");
        
        icraw::ChatCompletionRequest request1;
        request1.messages.push_back(icraw::Message("user", "Use the read_file tool to read the file 'README.md'"));
        request1.max_tokens = 256;

        std::cout << "User: Use the read_file tool to read the file 'README.md'\n\n";
        std::cout << "Assistant: ";
        auto response1 = provider.chat_completion(request1);
        
        if (!response1.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response1.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response1.content << "\n";
        }
        std::cout << "Finish reason: " << response1.finish_reason << "\n";

        // 测试2: write_file
        print_separator("Test 2: write_file Tool");
        
        icraw::ChatCompletionRequest request2;
        request2.messages.push_back(icraw::Message("user", "Use the write_file tool to create a file 'test.txt' with content 'Hello, World!'"));
        request2.max_tokens = 256;

        std::cout << "User: Use the write_file tool to create a file 'test.txt' with content 'Hello, World!'\n\n";
        std::cout << "Assistant: ";
        auto response2 = provider.chat_completion(request2);
        
        if (!response2.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response2.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response2.content << "\n";
        }
        std::cout << "Finish reason: " << response2.finish_reason << "\n";

        // 测试3: list_files
        print_separator("Test 3: list_files Tool");
        
        icraw::ChatCompletionRequest request3;
        request3.messages.push_back(icraw::Message("user", "Use the list_files tool to list files in the current directory"));
        request3.max_tokens = 256;

        std::cout << "User: Use the list_files tool to list files in the current directory\n\n";
        std::cout << "Assistant: ";
        auto response3 = provider.chat_completion(request3);
        
        if (!response3.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response3.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response3.content << "\n";
        }
        std::cout << "Finish reason: " << response3.finish_reason << "\n";

        // 测试4: grep_files
        print_separator("Test 4: grep_files Tool");
        
        icraw::ChatCompletionRequest request4;
        request4.messages.push_back(icraw::Message("user", "Use the grep_files tool to search for 'include' pattern in all files"));
        request4.max_tokens = 256;

        std::cout << "User: Use the grep_files tool to search for 'include' pattern in all files\n\n";
        std::cout << "Assistant: ";
        auto response4 = provider.chat_completion(request4);
        
        if (!response4.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response4.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response4.content << "\n";
        }
        std::cout << "Finish reason: " << response4.finish_reason << "\n";

        // 测试5: search_memory
        print_separator("Test 5: search_memory Tool");
        
        icraw::ChatCompletionRequest request5;
        request5.messages.push_back(icraw::Message("user", "Use the search_memory tool to search for 'project' in conversation history"));
        request5.max_tokens = 256;

        std::cout << "User: Use the search_memory tool to search for 'project' in conversation history\n\n";
        std::cout << "Assistant: ";
        auto response5 = provider.chat_completion(request5);
        
        if (!response5.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response5.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response5.content << "\n";
        }
        std::cout << "Finish reason: " << response5.finish_reason << "\n";

        // 测试6: save_memory
        print_separator("Test 6: save_memory Tool");
        
        icraw::ChatCompletionRequest request6;
        request6.messages.push_back(icraw::Message("user", "Use the save_memory tool to save 'User prefers dark mode' as a memory"));
        request6.max_tokens = 256;

        std::cout << "User: Use the save_memory tool to save 'User prefers dark mode' as a memory\n\n";
        std::cout << "Assistant: ";
        auto response6 = provider.chat_completion(request6);
        
        if (!response6.tool_calls.empty()) {
            std::cout << "\nTool Calls:\n";
            for (const auto& tc : response6.tool_calls) {
                std::cout << "  - Function: " << tc.name << "\n";
                std::cout << "    Arguments: " << tc.arguments.dump() << "\n";
            }
        } else {
            std::cout << response6.content << "\n";
        }
        std::cout << "Finish reason: " << response6.finish_reason << "\n";

        print_separator("Test Summary");
        std::cout << "All tool calling tests completed.\n";
        std::cout << "Tools tested: read_file, write_file, list_files, grep_files, search_memory, save_memory\n";

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