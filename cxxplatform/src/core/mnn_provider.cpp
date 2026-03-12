#ifdef ICRAW_USE_MNN

#include "icraw/core/mnn_provider.hpp"
#include "icraw/core/logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

// MNN头文件
#include <MNN/Interpreter.hpp>
#include <MNN/ImageProcess.hpp>
#include <MNN/expr/Executor.hpp>

namespace icraw {

MNNProvider::MNNProvider(const MNNConfig& config)
    : config_(config) {
    ICRAW_LOG_INFO("MNNProvider: Initializing with model: {}", config.model_path);
    
    model_loaded_ = init_session();
    
    if (model_loaded_) {
        ICRAW_LOG_INFO("MNNProvider: Model loaded successfully");
    } else {
        ICRAW_LOG_ERROR("MNNProvider: Failed to load model");
    }
}

MNNProvider::~MNNProvider() {
    if (session_) {
        interpreter_->releaseSession(session_);
        session_ = nullptr;
    }
}

bool MNNProvider::init_session() {
    // 检查模型文件是否存在
    if (!std::filesystem::exists(config_.model_path)) {
        ICRAW_LOG_ERROR("MNNProvider: Model file not found: {}", config_.model_path);
        return false;
    }
    
    // 创建MNN解释器
    interpreter_.reset(MNN::Interpreter::createFromFile(config_.model_path.c_str()));
    if (!interpreter_) {
        ICRAW_LOG_ERROR("MNNProvider: Failed to create MNN interpreter");
        return false;
    }
    
    // 配置调度
    MNN::ScheduleConfig schedule_config;
    
    // 设置后端
    if (config_.backend == "cpu") {
        schedule_config.type = MNN_FORWARD_CPU;
        schedule_config.numThread = config_.thread_num;
    } else if (config_.backend == "opencl") {
        schedule_config.type = MNN_FORWARD_OPENCL;
    } else if (config_.backend == "metal") {
        schedule_config.type = MNN_FORWARD_METAL;
    } else {
        schedule_config.type = MNN_FORWARD_AUTO;
    }
    
    // 创建会话
    session_ = interpreter_->createSession(schedule_config);
    if (!session_) {
        ICRAW_LOG_ERROR("MNNProvider: Failed to create MNN session");
        return false;
    }
    
    // 加载tokenizer（简化版）
    if (!config_.tokenizer_path.empty() && std::filesystem::exists(config_.tokenizer_path)) {
        // TODO: 实现tokenizer加载
        // 这里使用简化实现，实际项目中应该使用sentencepiece或其他tokenizer
        ICRAW_LOG_DEBUG("MNNProvider: Tokenizer path: {}", config_.tokenizer_path);
    }
    
    return true;
}

std::vector<int> MNNProvider::tokenize(const std::string& text) {
    // 简化实现：字符级别tokenization
    // TODO: 实现proper tokenization（sentencepiece/BPE）
    std::vector<int> tokens;
    tokens.reserve(text.size());
    
    for (unsigned char c : text) {
        tokens.push_back(static_cast<int>(c));
    }
    
    return tokens;
}

std::string MNNProvider::detokenize(const std::vector<int>& tokens) {
    // 简化实现：字符级别detokenization
    // TODO: 实现proper detokenization
    std::string text;
    text.reserve(tokens.size());
    
    for (int token : tokens) {
        if (token >= 0 && token < 256) {
            text += static_cast<char>(token);
        }
    }
    
    return text;
}

std::string MNNProvider::run_inference(const std::vector<int>& input_tokens,
                                       int max_new_tokens,
                                       float temperature,
                                       float top_p) {
    // 获取输入tensor
    MNN::Tensor* input_tensor = interpreter_->getSessionInput(session_, nullptr);
    if (!input_tensor) {
        ICRAW_LOG_ERROR("MNNProvider: Failed to get input tensor");
        return "";
    }
    
    // 准备输入数据
    // TODO: 根据实际模型调整输入格式
    std::vector<int> all_tokens = input_tokens;
    std::string generated_text;
    
    // 自回归生成
    for (int i = 0; i < max_new_tokens; ++i) {
        // 运行推理
        interpreter_->runSession(session_);
        
        // 获取输出
        MNN::Tensor* output_tensor = interpreter_->getSessionOutput(session_, nullptr);
        if (!output_tensor) {
            break;
        }
        
        // TODO: 实现采样策略（temperature, top_p等）
        // 简化实现：随机选择
        int next_token = 0;  // TODO: 实际采样逻辑
        
        // 检查是否结束
        if (next_token == 0 /* EOS token */) {
            break;
        }
        
        all_tokens.push_back(next_token);
    }
    
    // Detokenize
    generated_text = detokenize(all_tokens);
    
    return generated_text;
}

ChatCompletionResponse MNNProvider::chat_completion(
    const ChatCompletionRequest& request) {
    
    ChatCompletionResponse response;
    
    if (!model_loaded_) {
        response.finish_reason = "error";
        response.content = "MNN model not loaded";
        return response;
    }
    
    // 构建输入prompt
    std::string prompt;
    for (const auto& msg : request.messages) {
        if (msg.role == "user") {
            prompt += "User: " + msg.text() + "\n";
        } else if (msg.role == "assistant") {
            prompt += "Assistant: " + msg.text() + "\n";
        }
    }
    prompt += "Assistant:";
    
    // Tokenize
    auto input_tokens = tokenize(prompt);
    
    // 运行推理
    int max_new_tokens = std::min(request.max_tokens, max_tokens_);
    std::string generated = run_inference(
        input_tokens, 
        max_new_tokens,
        static_cast<float>(request.temperature),
        0.9f  // top_p
    );
    
    response.content = generated;
    response.finish_reason = "stop";
    
    return response;
}

void MNNProvider::chat_completion_stream(
    const ChatCompletionRequest& request,
    std::function<void(const ChatCompletionResponse&)> callback) {
    
    if (!model_loaded_) {
        ChatCompletionResponse error_response;
        error_response.finish_reason = "error";
        error_response.content = "MNN model not loaded";
        callback(error_response);
        return;
    }
    
    // 构建输入prompt
    std::string prompt;
    for (const auto& msg : request.messages) {
        if (msg.role == "user") {
            prompt += "User: " + msg.text() + "\n";
        } else if (msg.role == "assistant") {
            prompt += "Assistant: " + msg.text() + "\n";
        }
    }
    prompt += "Assistant:";
    
    // Tokenize
    auto input_tokens = tokenize(prompt);
    std::vector<int> all_tokens = input_tokens;
    
    // 流式生成
    int max_new_tokens = std::min(request.max_tokens, max_tokens_);
    
    for (int i = 0; i < max_new_tokens; ++i) {
        // TODO: 实现逐token流式生成
        // 简化实现：一次性返回
        
        ChatCompletionResponse chunk;
        
        // 模拟流式输出（每次一个字符）
        if (i == 0) {
            // 第一次返回完整内容
            std::string generated = run_inference(
                input_tokens,
                max_new_tokens,
                static_cast<float>(request.temperature),
                0.9f
            );
            
            chunk.content = generated;
            chunk.finish_reason = "stop";
            callback(chunk);
            break;
        }
    }
    
    // 发送结束标记
    ChatCompletionResponse end_response;
    end_response.is_stream_end = true;
    end_response.finish_reason = "stop";
    callback(end_response);
}

std::vector<std::string> MNNProvider::get_supported_models() const {
    return {
        "qwen-0.5b",
        "qwen-1.5b",
        "qwen-7b",
        "llama-2-7b",
        "gemma-2b"
    };
}

size_t MNNProvider::get_memory_usage() const {
    // TODO: 实现实际内存统计
    return 0;
}

} // namespace icraw

#endif // ICRAW_USE_MNN
