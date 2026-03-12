#pragma once

#ifdef ICRAW_USE_MNN

#include "icraw/core/llm_provider.hpp"
#include "icraw/config.hpp"
#include <memory>
#include <string>

// MNN前向声明（避免在头文件中包含MNN头文件）
namespace MNN {
    class Interpreter;
    class Session;
    class Tensor;
}

namespace icraw {

/**
 * MNNProvider - 基于MNN的本地LLM推理Provider
 * 
 * 实现LLMProvider接口，使用MNN引擎进行本地推理
 * 仅在ICRAW_USE_MNN编译时可用
 */
class MNNProvider : public LLMProvider {
public:
    explicit MNNProvider(const MNNConfig& config);
    ~MNNProvider() override;
    
    // 禁止拷贝
    MNNProvider(const MNNProvider&) = delete;
    MNNProvider& operator=(const MNNProvider&) = delete;
    
    // ========== LLMProvider接口实现 ==========
    
    ChatCompletionResponse chat_completion(
        const ChatCompletionRequest& request) override;
    
    void chat_completion_stream(
        const ChatCompletionRequest& request,
        std::function<void(const ChatCompletionResponse&)> callback) override;
    
    std::string get_provider_name() const override { return "MNN-Local"; }
    
    std::vector<std::string> get_supported_models() const override;
    
    // ========== MNN特有方法 ==========
    
    // 检查模型是否已加载
    bool is_model_loaded() const { return model_loaded_; }
    
    // 获取当前内存使用量（字节）
    size_t get_memory_usage() const;
    
    // 获取配置
    const MNNConfig& get_config() const { return config_; }
    
    // 设置最大生成长度
    void set_max_tokens(int max_tokens) { max_tokens_ = max_tokens; }
    
    // 获取当前生成长度限制
    int get_max_tokens() const { return max_tokens_; }

private:
    // 初始化MNN会话
    bool init_session();
    
    // Tokenizer相关（简化实现，后续可扩展）
    std::vector<int> tokenize(const std::string& text);
    std::string detokenize(const std::vector<int>& tokens);
    
    // 执行推理
    std::string run_inference(const std::vector<int>& input_tokens, 
                              int max_new_tokens,
                              float temperature,
                              float top_p);
    
    MNNConfig config_;
    std::unique_ptr<MNN::Interpreter> interpreter_;
    MNN::Session* session_ = nullptr;
    bool model_loaded_ = false;
    int max_tokens_ = 4096;
    
    // Tokenizer（简化版，后续可替换为sentencepiece）
    std::vector<std::string> vocab_;
    std::unordered_map<std::string, int> vocab_map_;
};

} // namespace icraw

#endif // ICRAW_USE_MNN
