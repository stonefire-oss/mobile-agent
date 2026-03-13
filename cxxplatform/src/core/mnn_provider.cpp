#ifdef ICRAW_USE_MNN

#include "icraw/core/mnn_provider.hpp"
#include "icraw/core/logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <cstring>

// MNN-LLM头文件
#include <llm/llm.hpp>

namespace icraw {

// ========== LlmStreamBuffer实现 ==========

LlmStreamBuffer::LlmStreamBuffer(Callback callback)
    : callback_(std::move(callback)) {
}

std::streamsize LlmStreamBuffer::xsputn(const char* s, std::streamsize n) {
    if (callback_ && n > 0) {
        callback_(s, static_cast<size_t>(n));
    }
    return n;
}

int LlmStreamBuffer::overflow(int c) {
    if (c != EOF && callback_) {
        char ch = static_cast<char>(c);
        callback_(&ch, 1);
    }
    return c;
}

// ========== MNNProvider实现 ==========

MNNProvider::MNNProvider(const MNNConfig& config)
    : config_(config) {
    ICRAW_LOG_INFO("MNNProvider: Initializing with model: {}", config.model_path);
    
    model_loaded_ = init_llm();
    
    if (model_loaded_) {
        ICRAW_LOG_INFO("MNNProvider: Model loaded successfully (multimodal: {})", is_multimodal_);
    } else {
        ICRAW_LOG_ERROR("MNNProvider: Failed to load model");
    }
}

MNNProvider::~MNNProvider() {
    // 显式释放MNN资源
    if (llm_) {
        llm_->reset();  // 清理KV Cache等状态
        llm_.reset();   // 释放LLM对象
    }
    ICRAW_LOG_DEBUG("MNNProvider: Destroyed");
}

bool MNNProvider::init_llm() {
    // 检查模型目录是否存在
    model_dir_ = config_.model_path;
    
    // 如果是文件路径，获取目录
    if (std::filesystem::is_regular_file(model_dir_)) {
        model_dir_ = std::filesystem::path(model_dir_).parent_path().string();
    }
    
    if (!std::filesystem::exists(model_dir_)) {
        ICRAW_LOG_ERROR("MNNProvider: Model directory not found: {}", model_dir_);
        return false;
    }
    
    // 检查必要的文件
    std::string config_file = model_dir_ + "/config.json";
    std::string llm_config_file = model_dir_ + "/llm_config.json";
    
    if (!std::filesystem::exists(config_file)) {
        ICRAW_LOG_ERROR("MNNProvider: config.json not found in: {}", model_dir_);
        return false;
    }
    
    // 检查是否为多模态模型（存在visual.mnn）
    std::string visual_model = model_dir_ + "/visual.mnn";
    is_multimodal_ = std::filesystem::exists(visual_model);
    
    // 创建LLM实例
    config_path_ = config_file;
    
    llm_.reset(MNN::Transformer::Llm::createLLM(config_path_));
    if (!llm_) {
        ICRAW_LOG_ERROR("MNNProvider: Failed to create LLM instance");
        return false;
    }
    
    // 加载模型
    llm_->load();
    
    // 设置运行时配置
    nlohmann::json runtime_config = {
        {"max_new_tokens", max_tokens_},
        {"temperature", 0.7},
        {"topK", 40},
        {"topP", 0.9},
        {"backend_type", config_.backend},
        {"thread_num", config_.thread_num},
        {"use_mmap", config_.use_mmap}
    };
    
    // 应用配置
    llm_->set_config(runtime_config.dump());
    
    ICRAW_LOG_INFO("MNNProvider: LLM loaded with backend={}, threads={}", 
                   config_.backend, config_.thread_num);
    
    return true;
}

std::string MNNProvider::build_qwen_prompt(const std::vector<Message>& messages) {
    // Qwen Chat Template格式
    // <|im_start|>system\n{content}<|im_end|>\n
    // <|im_start|>user\n{content}<|im_end|>\n
    // <|im_start|>assistant\n
    
    std::ostringstream prompt;
    
    for (const auto& msg : messages) {
        if (msg.role == "system") {
            prompt << "<|im_start|>system\n" << msg.text() << "<|im_end|>\n";
        } else if (msg.role == "user") {
            prompt << "<|im_start|>user\n" << msg.text() << "<|im_end|>\n";
        } else if (msg.role == "assistant") {
            prompt << "<|im_start|>assistant\n" << msg.text() << "<|im_end|>\n";
        }
    }
    
    // 添加assistant开始标记
    prompt << "<|im_start|>assistant\n";
    
    return prompt.str();
}

ChatCompletionResponse MNNProvider::chat_completion(
    const ChatCompletionRequest& request) {
    
    ChatCompletionResponse response;
    
    if (!model_loaded_ || !llm_) {
        response.finish_reason = "error";
        response.content = "MNN model not loaded";
        return response;
    }
    
    // 使用MNN的apply_chat_template
    std::string prompt;
    if (!request.messages.empty()) {
        // 获取最后一条用户消息
        for (auto it = request.messages.rbegin(); it != request.messages.rend(); ++it) {
            if (it->role == "user") {
                prompt = llm_->apply_chat_template(it->text());
                break;
            }
        }
        if (prompt.empty()) {
            prompt = llm_->apply_chat_template(request.messages.back().text());
        }
    }
    
    // 更新运行时配置
    nlohmann::json runtime_config = {
        {"max_new_tokens", std::min(request.max_tokens, max_tokens_)},
        {"temperature", request.temperature}
    };
    llm_->set_config(runtime_config.dump());
    
    // 使用stringstream收集输出
    std::ostringstream output_stream;
    llm_->response(prompt, &output_stream, nullptr, request.max_tokens);
    
    std::string raw_output = output_stream.str();
    
    // 解析工具调用
    // Qwen格式: <function=function_name>\n<parameter=param_name>\nvalue\n</parameter>\n</function>
    parse_tool_calls(raw_output, response);
    
    if (response.tool_calls.empty()) {
        // 没有工具调用，直接使用原始输出
        response.content = raw_output;
    } else {
        // 有工具调用，content 为空
        response.content = "";
        response.finish_reason = "tool_calls";
    }
    
    return response;
}

void MNNProvider::parse_tool_calls(const std::string& output, ChatCompletionResponse& response) {
    // 解析 Qwen 工具调用格式
    // 格式: <function=function_name>\n<parameter=param_name>\nvalue\n</parameter>\n</function>
    
    const std::string function_start = "<function=";
    const std::string function_end = "</function>";
    const std::string param_start = "<parameter=";
    const std::string param_end = "</parameter>";
    
    size_t pos = 0;
    while ((pos = output.find(function_start, pos)) != std::string::npos) {
        ToolCall tool_call;
        
        // 提取函数名
        size_t name_start = pos + function_start.length();
        size_t name_end = output.find('>', name_start);
        if (name_end == std::string::npos) break;
        
        tool_call.name = output.substr(name_start, name_end - name_start);
        tool_call.id = "call_" + std::to_string(response.tool_calls.size());
        
        // 提取参数
        nlohmann::json arguments = nlohmann::json::object();
        size_t func_content_start = name_end + 1;
        size_t func_content_end = output.find(function_end, func_content_start);
        
        if (func_content_end != std::string::npos) {
            std::string func_content = output.substr(func_content_start, func_content_end - func_content_start);
            
            // 解析参数
            size_t param_pos = 0;
            while ((param_pos = func_content.find(param_start, param_pos)) != std::string::npos) {
                size_t param_name_start = param_pos + param_start.length();
                size_t param_name_end = func_content.find('>', param_name_start);
                if (param_name_end == std::string::npos) break;
                
                std::string param_name = func_content.substr(param_name_start, param_name_end - param_name_start);
                
                size_t param_value_start = param_name_end + 1;
                size_t param_value_end = func_content.find(param_end, param_value_start);
                if (param_value_end == std::string::npos) break;
                
                std::string param_value = func_content.substr(param_value_start, param_value_end - param_value_start);
                
                // 去除前后空白
                size_t start = param_value.find_first_not_of(" \n\r\t");
                size_t end = param_value.find_last_not_of(" \n\r\t");
                if (start != std::string::npos && end != std::string::npos) {
                    param_value = param_value.substr(start, end - start + 1);
                }
                
                arguments[param_name] = param_value;
                param_pos = param_value_end + param_end.length();
            }
        }
        
        tool_call.arguments = arguments;
        response.tool_calls.push_back(tool_call);
        
        pos = func_content_end + function_end.length();
    }
}

void MNNProvider::chat_completion_stream(
    const ChatCompletionRequest& request,
    std::function<void(const ChatCompletionResponse&)> callback) {
    
    if (!model_loaded_ || !llm_) {
        ChatCompletionResponse error_response;
        error_response.finish_reason = "error";
        error_response.content = "MNN model not loaded";
        callback(error_response);
        return;
    }
    
    // 使用MNN的apply_chat_template
    std::string prompt;
    if (!request.messages.empty()) {
        for (auto it = request.messages.rbegin(); it != request.messages.rend(); ++it) {
            if (it->role == "user") {
                prompt = llm_->apply_chat_template(it->text());
                break;
            }
        }
        if (prompt.empty()) {
            prompt = llm_->apply_chat_template(request.messages.back().text());
        }
    }
    
    // 更新运行时配置
    nlohmann::json runtime_config = {
        {"max_new_tokens", std::min(request.max_tokens, max_tokens_)},
        {"temperature", request.temperature}
    };
    llm_->set_config(runtime_config.dump());
    
    // 累积输出以解析工具调用
    std::string accumulated_output;
    
    // 创建流式缓冲区
    LlmStreamBuffer stream_buffer([&callback, &accumulated_output](const char* str, size_t len) {
        accumulated_output.append(str, len);
        
        ChatCompletionResponse chunk;
        chunk.content = std::string(str, len);
        chunk.finish_reason = "";  // 流式中间结果
        callback(chunk);
    });
    
    std::ostream output_stream(&stream_buffer);
    
    // 执行流式推理
    llm_->response(prompt, &output_stream, nullptr, request.max_tokens);
    
    // 解析工具调用
    ChatCompletionResponse end_response;
    parse_tool_calls(accumulated_output, end_response);
    
    if (!end_response.tool_calls.empty()) {
        // 有工具调用
        end_response.is_stream_end = true;
        end_response.finish_reason = "tool_calls";
        end_response.content = "";
        ICRAW_LOG_DEBUG("MNNProvider: Stream end with {} tool calls", end_response.tool_calls.size());
    } else {
        // 无工具调用
        end_response.is_stream_end = true;
        end_response.finish_reason = "stop";
    }
    
    callback(end_response);
}

std::vector<std::string> MNNProvider::get_supported_models() const {
    return {
        "qwen2.5-0.5b",
        "qwen2.5-1.5b",
        "qwen2.5-3b",
        "qwen2.5-7b",
        "qwen3.5-0.8b",
        "llama-2-7b",
        "gemma-2b"
    };
}

size_t MNNProvider::get_memory_usage() const {
    // TODO: MNN-LLM暂未提供内存统计API
    // 可通过操作系统API获取进程内存
    return 0;
}

void MNNProvider::reset() {
    if (llm_) {
        llm_->reset();
        ICRAW_LOG_DEBUG("MNNProvider: LLM state reset");
    }
}

// ========== 多模态支持 ==========

std::optional<MNNImageInput> MNNProvider::load_image(const std::string& image_path) {
    if (!std::filesystem::exists(image_path)) {
        ICRAW_LOG_ERROR("MNNProvider: Image file not found: {}", image_path);
        return std::nullopt;
    }
    
    // 简化实现：读取文件原始数据
    // 实际应用中需要使用图像解码库（如stb_image）
    std::ifstream file(image_path, std::ios::binary);
    if (!file.is_open()) {
        ICRAW_LOG_ERROR("MNNProvider: Failed to open image file: {}", image_path);
        return std::nullopt;
    }
    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    MNNImageInput result;
    result.data.resize(file_size);
    file.read(reinterpret_cast<char*>(result.data.data()), file_size);
    result.channels = 3;
    
    ICRAW_LOG_INFO("MNNProvider: Loaded image file: {} bytes", result.data.size());
    
    return result;
}

ChatCompletionResponse MNNProvider::chat_completion_with_image(
    const std::string& prompt,
    const MNNImageInput& image) {
    
    ChatCompletionResponse response;
    
    if (!model_loaded_ || !llm_) {
        response.finish_reason = "error";
        response.content = "MNN model not loaded";
        return response;
    }
    
    if (!is_multimodal_) {
        response.finish_reason = "error";
        response.content = "Model does not support multimodal input";
        return response;
    }
    
    // TODO: 实现完整的多模态推理
    // 需要使用MNN的MultimodalPrompt API
    response.finish_reason = "error";
    response.content = "Multimodal inference not yet implemented";
    
    return response;
}

void MNNProvider::chat_completion_with_image_stream(
    const std::string& prompt,
    const MNNImageInput& image,
    std::function<void(const ChatCompletionResponse&)> callback) {
    
    if (!model_loaded_ || !llm_) {
        ChatCompletionResponse error_response;
        error_response.finish_reason = "error";
        error_response.content = "MNN model not loaded";
        callback(error_response);
        return;
    }
    
    if (!is_multimodal_) {
        ChatCompletionResponse error_response;
        error_response.finish_reason = "error";
        error_response.content = "Model does not support multimodal input";
        callback(error_response);
        return;
    }
    
    // TODO: 实现完整的多模态流式推理
    ChatCompletionResponse error_response;
    error_response.finish_reason = "error";
    error_response.content = "Multimodal streaming not yet implemented";
    callback(error_response);
}

} // namespace icraw

#endif // ICRAW_USE_MNN
