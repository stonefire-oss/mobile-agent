#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <icraw/mobile_agent.hpp>
#include <icraw/core/llm_provider.hpp>
#include <icraw/core/logger.hpp>

#ifdef ICRAW_USE_MNN
#include <icraw/core/mnn_provider.hpp>
#endif

std::atomic<bool> g_running(true);

void signal_handler(int) {
    g_running = false;
    std::cout << "\n\nStopping..." << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << "icraw CLI Agent Demo\n\n";
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "\n远程API选项:\n";
    std::cout << "  --api-key <key>      OpenAI API key (或设置 OPENAI_API_KEY 环境变量)\n";
    std::cout << "  --base-url <url>     API base URL (默认: https://api.openai.com/v1)\n";
    std::cout << "  --model <model>      模型名称 (默认: gpt-4o)\n";
#ifdef ICRAW_USE_MNN
    std::cout << "\n本地MNN选项:\n";
    std::cout << "  --mnn-model <path>   使用本地MNN模型 (例如: D:\\models\\Qwen3.5-0.8B-MNN)\n";
    std::cout << "  --mnn-threads <n>    MNN推理线程数 (默认: 4)\n";
    std::cout << "  --mnn-backend <type> MNN后端类型: cpu, opencl, metal (默认: cpu)\n";
#endif
    std::cout << "\n其他选项:\n";
    std::cout << "  --workspace <path>   工作目录 (默认: ~/.icraw/workspace)\n";
    std::cout << "  --skill <path>       Skills目录 (默认: <workspace>/skills)\n";
    std::cout << "  --log <path>         日志目录 (默认: 禁用)\n";
    std::cout << "  --log-level <level>  日志级别: trace, debug, info, warn, error (默认: info)\n";
    std::cout << "  --no-stream          禁用流式输出\n";
    std::cout << "  --help               显示帮助信息\n";
}

void print_welcome(bool use_mnn, const std::string& model_name) {
    std::cout << "\n";
    std::cout << "==================================================\n";
    std::cout << "            icraw CLI Agent Demo\n";
    if (use_mnn) {
        std::cout << "            [本地MNN模式]\n";
        std::cout << "            Model: " << model_name << "\n";
    } else {
        std::cout << "            [远程API模式]\n";
        std::cout << "            Model: " << model_name << "\n";
    }
    std::cout << "              Type /exit to quit\n";
    std::cout << "==================================================\n\n";
}

int main(int argc, char* argv[]) {
    // Set console to UTF-8 mode on Windows
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::string api_key;
    std::string base_url = "https://api.openai.com/v1";
    std::string model = "gpt-4o";
    std::string workspace_path;
    std::string skill_path;
    std::string log_path;
    std::string log_level = "info";
    bool use_stream = true;
    
    // MNN options
    std::string mnn_model_path;
    int mnn_threads = 4;
    std::string mnn_backend = "cpu";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--api-key" && i + 1 < argc) {
            api_key = argv[++i];
        } else if (arg == "--base-url" && i + 1 < argc) {
            base_url = argv[++i];
        } else if (arg == "--model" && i + 1 < argc) {
            model = argv[++i];
        } else if (arg == "--workspace" && i + 1 < argc) {
            workspace_path = argv[++i];
        } else if (arg == "--skill" && i + 1 < argc) {
            skill_path = argv[++i];
        } else if (arg == "--log" && i + 1 < argc) {
            log_path = argv[++i];
        } else if (arg == "--log-level" && i + 1 < argc) {
            log_level = argv[++i];
        } else if (arg == "--no-stream") {
            use_stream = false;
        }
#ifdef ICRAW_USE_MNN
        else if (arg == "--mnn-model" && i + 1 < argc) {
            mnn_model_path = argv[++i];
        } else if (arg == "--mnn-threads" && i + 1 < argc) {
            mnn_threads = std::stoi(argv[++i]);
        } else if (arg == "--mnn-backend" && i + 1 < argc) {
            mnn_backend = argv[++i];
        }
#endif
        else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Check if using MNN local model
    bool use_mnn = !mnn_model_path.empty();

#ifdef ICRAW_USE_MNN
    if (use_mnn) {
        std::cout << "Using local MNN model: " << mnn_model_path << std::endl;
    }
#else
    if (use_mnn) {
        std::cerr << "Error: MNN support not compiled in.\n";
        std::cerr << "Rebuild with -DICRAW_USE_MNN=ON to use local models.\n";
        return 1;
    }
#endif

    // Check for API key (only needed for remote API)
    if (!use_mnn) {
        if (api_key.empty()) {
            const char* env_key = std::getenv("OPENAI_API_KEY");
            if (env_key) {
                api_key = env_key;
            }
        }

        if (api_key.empty()) {
            std::cerr << "Error: No API key provided.\n";
            std::cerr << "Set OPENAI_API_KEY environment variable or use --api-key option.\n";
            std::cerr << "Or use --mnn-model for local inference.\n";
            return 1;
        }
    }

    // Setup signal handler
    std::signal(SIGINT, signal_handler);

    try {
        // Create configuration
        icraw::IcrawConfig config;
        
        // Set workspace path
        if (workspace_path.empty()) {
            workspace_path = icraw::IcrawConfig::default_workspace_path().string();
        }
        config.workspace_path = workspace_path;
        
        // Set skills path
        if (!skill_path.empty()) {
            config.skills.path = skill_path;
        }
        
        // Initialize logger if log directory is specified
        if (!log_path.empty()) {
            std::cout << "Initializing logger with path: " << log_path << ", level: " << log_level << std::endl;
            icraw::Logger::get_instance().init(log_path, log_level);
            config.logging.enabled = true;
            config.logging.directory = log_path;
            config.logging.level = log_level;
            
            if (icraw::Logger::get_instance().is_initialized()) {
                std::cout << "Logger initialized successfully!" << std::endl;
            } else {
                std::cout << "Logger initialization failed!" << std::endl;
            }
        }
        
        // Set provider config BEFORE creating agent
        if (!use_mnn) {
            // Remote API mode - set API config before creating agent
            config.provider.api_key = api_key;
            config.provider.base_url = base_url;
            config.agent.model = model;
        }
        
        // Create the agent
        icraw::MobileAgent agent(config);
        
        // Switch to MNN if requested
        if (use_mnn) {
#ifdef ICRAW_USE_MNN
            icraw::MNNConfig mnn_config;
            mnn_config.model_path = mnn_model_path;
            mnn_config.thread_num = mnn_threads;
            mnn_config.backend = mnn_backend;
            mnn_config.use_mmap = true;
            
            std::cout << "Loading MNN model: " << mnn_model_path << std::endl;
            if (!agent.switch_to_mnn_provider(mnn_config)) {
                std::cerr << "Error: Failed to switch to MNN provider\n";
                return 1;
            }
            std::cout << "MNN model loaded successfully!" << std::endl;
#else
            (void)mnn_threads;
            (void)mnn_backend;
#endif
        }
        
        print_welcome(use_mnn, use_mnn ? mnn_model_path : model);
        std::cout << "Agent ready! Type your message and press Enter.\n";
        std::cout << "Commands: /exit, /clear, /help\n\n";
        
        // Main chat loop
        while (g_running) {
            std::cout << "You: ";
            std::cout.flush();
            
            std::string input;
            // Check if stdin is still valid before reading
            if (!std::getline(std::cin, input)) {
                // stdin closed or error
                break;
            }
            
            if (!g_running) break;
            
            if (input.empty()) {
                continue;
            }
            
            // Handle commands
            if (input == "/exit" || input == "/quit" || input == "/q") {
                std::cout << "Goodbye!\n";
                break;
            }
            
            if (input == "/clear" || input == "/reset") {
                agent.clear_history();
                std::cout << "Conversation history cleared.\n\n";
                continue;
            }
            
            if (input == "/help") {
                std::cout << "\nCommands:\n";
                std::cout << "  /exit, /quit, /q  - Exit the program\n";
                std::cout << "  /clear, /reset    - Clear conversation history\n";
                std::cout << "  /help             - Show this help message\n\n";
                continue;
            }
            
            // Send message to agent
            std::cout << "\nAssistant: ";
            std::cout.flush();
            
            if (use_stream) {
                // Streaming mode
                agent.chat_stream(input, [&](const icraw::AgentEvent& event) {
                    if (!g_running) return;
                    
                    if (event.type == "text_delta") {
                        if (event.data.contains("delta")) {
                            std::string delta = event.data["delta"].get<std::string>();
                            std::cout << delta;
                            std::cout.flush();
                        }
                    } else if (event.type == "tool_use") {
                        // Show when a tool is being called
                        std::string tool_name = event.data.value("name", "unknown");
                        std::cout << "\n[Calling tool: " << tool_name << "]\n";
                        std::cout.flush();
                    } else if (event.type == "tool_result") {
                        // Show tool result
                        std::string content = event.data.value("content", "");
                        std::cout << "\n[Tool result: " << content << "]\n";
                        std::cout.flush();
                    } else if (event.type == "message_end") {
                        std::string finish_reason = event.data.value("finish_reason", "");
                        std::cout << "\n[End: " << finish_reason << "]\n";
                        std::cout.flush();
                    }
                });
                
                std::cout << "\n\n";
            } else {
                // Non-streaming mode
                std::string response = agent.chat(input);
                std::cout << response << "\n\n";
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
