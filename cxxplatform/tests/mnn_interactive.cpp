// MNN 交互式测试程序
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <icraw/core/mnn_provider.hpp>
#include <icraw/config.hpp>

#ifdef ICRAW_USE_MNN

void print_separator() {
    std::cout << "\n----------------------------------------\n";
}

void print_help() {
    std::cout << "\n命令:\n";
    std::cout << "  /help     - 显示帮助\n";
    std::cout << "  /reset    - 重置对话\n";
    std::cout << "  /stream   - 切换流式/非流式输出\n";
    std::cout << "  /tokens   - 设置最大token数\n";
    std::cout << "  /temp     - 设置temperature\n";
    std::cout << "  /quit     - 退出程序\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << R"(
========================================
  MNN 交互式测试程序
  Qwen3.5-0.8B-MNN
========================================
)" << "\n";

    if (argc < 2) {
        std::cout << "用法: " << argv[0] << " <模型路径>\n";
        std::cout << "示例: " << argv[0] << " D:\\AppData\\modelscope\\Qwen3.5-0.8B-MNN\n";
        return 1;
    }

    std::string model_path = argv[1];
    std::cout << "模型路径: " << model_path << "\n\n";

    try {
        // 创建 MNN Provider
        std::cout << "正在加载模型...\n";
        auto start = std::chrono::high_resolution_clock::now();
        
        icraw::MNNConfig config;
        config.model_path = model_path;
        config.thread_num = 4;
        config.backend = "cpu";
        
        icraw::MNNProvider provider(config);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto load_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        if (!provider.is_model_loaded()) {
            std::cerr << "错误: 模型加载失败\n";
            return 1;
        }

        std::cout << "模型加载成功! (" << load_time << " ms)\n";
        std::cout << "多模态: " << (provider.is_multimodal() ? "是" : "否") << "\n";
        std::cout << "最大Token: " << provider.get_max_tokens() << "\n";
        
        print_separator();
        print_help();

        // 配置
        bool use_stream = true;
        int max_tokens = 512;
        double temperature = 0.7;

        // 主循环
        std::string input;
        while (true) {
            std::cout << "\n你: ";
            std::getline(std::cin, input);

            if (input.empty()) continue;

            // 处理命令
            if (input[0] == '/') {
                if (input == "/quit" || input == "/exit") {
                    std::cout << "再见!\n";
                    break;
                } else if (input == "/help") {
                    print_help();
                } else if (input == "/reset") {
                    provider.reset();
                    std::cout << "对话已重置\n";
                } else if (input == "/stream") {
                    use_stream = !use_stream;
                    std::cout << "流式输出: " << (use_stream ? "开启" : "关闭") << "\n";
                } else if (input.rfind("/tokens ", 0) == 0) {
                    max_tokens = std::stoi(input.substr(8));
                    provider.set_max_tokens(max_tokens);
                    std::cout << "最大Token: " << max_tokens << "\n";
                } else if (input.rfind("/temp ", 0) == 0) {
                    temperature = std::stod(input.substr(6));
                    std::cout << "Temperature: " << temperature << "\n";
                } else {
                    std::cout << "未知命令: " << input << "\n";
                    print_help();
                }
                continue;
            }

            // 构建请求
            icraw::ChatCompletionRequest request;
            request.messages.push_back(icraw::Message("user", input));
            request.max_tokens = max_tokens;
            request.temperature = temperature;

            std::cout << "\n助手: ";

            // 计时
            auto inference_start = std::chrono::high_resolution_clock::now();
            int token_count = 0;

            if (use_stream) {
                // 流式输出
                provider.chat_completion_stream(request, [&token_count](const icraw::ChatCompletionResponse& chunk) {
                    if (!chunk.content.empty()) {
                        std::cout << chunk.content << std::flush;
                        token_count++;
                    }
                    if (chunk.is_stream_end) {
                        std::cout << "\n[完成: " << chunk.finish_reason << "]";
                    }
                });
            } else {
                // 非流式输出
                auto response = provider.chat_completion(request);
                std::cout << response.content;
                token_count = response.content.length() / 2; // 粗略估计
                
                if (!response.tool_calls.empty()) {
                    std::cout << "\n\n工具调用:\n";
                    for (const auto& tc : response.tool_calls) {
                        std::cout << "  - " << tc.name << "\n";
                        std::cout << "    参数: " << tc.arguments.dump() << "\n";
                    }
                }
                std::cout << "\n[完成: " << response.finish_reason << "]";
            }

            auto inference_end = std::chrono::high_resolution_clock::now();
            auto inference_time = std::chrono::duration_cast<std::chrono::milliseconds>(inference_end - inference_start).count();

            std::cout << "\n\n--- 统计 ---\n";
            std::cout << "响应时间: " << inference_time << " ms\n";
            if (token_count > 0 && inference_time > 0) {
                double tokens_per_sec = (double)token_count / inference_time * 1000;
                std::cout << "预估速度: " << (int)tokens_per_sec << " tokens/s\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "\n错误: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

#else

int main() {
    std::cerr << "错误: MNN 支持未编译\n";
    std::cerr << "请使用 -DICRAW_USE_MNN=ON 重新编译\n";
    return 1;
}

#endif