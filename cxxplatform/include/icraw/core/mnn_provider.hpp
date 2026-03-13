#pragma once

#ifdef ICRAW_USE_MNN

#include "icraw/core/llm_provider.hpp"
#include "icraw/config.hpp"
#include <memory>
#include <string>
#include <functional>
#include <streambuf>
#include <vector>

// MNN-LLM前向声明
namespace MNN {
namespace Transformer {
    class Llm;
    class Tokenizer;
}
}

namespace icraw {

/**
 * 流式输出回调缓冲区
 * 用于将MNN-LLM的流式输出转换为回调函数调用
 */
class LlmStreamBuffer : public std::streambuf {
public:
    using Callback = std::function<void(const char* str, size_t len)>;
    
    explicit LlmStreamBuffer(Callback callback);
    
protected:
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int overflow(int c) override;
    
private:
    Callback callback_;
};

/**
 * 图像输入数据结构
 */
struct MNNImageInput {
    std::vector<uint8_t> data;  // 图像数据（RGB格式）
    int width = 0;
    int height = 0;
    int channels = 3;  // 默认RGB
};

/**
 * MNNProvider - 基于MNN-LLM的本地LLM推理Provider
 * 
 * 实现LLMProvider接口，使用MNN-LLM高级API进行本地推理
 * 仅在ICRAW_USE_MNN编译时可用
 * 
 * 支持功能：
 * - 自动加载tokenizer（Tiktoken/SentencePiece）
 * - 流式生成
 * - 采样策略（temperature, top_p, top_k）
 * - KV Cache优化
 * - 多模态支持（图像输入）
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
    
    // 检查是否为多模态模型
    bool is_multimodal() const { return is_multimodal_; }
    
    // 获取当前内存使用量（字节）
    size_t get_memory_usage() const;
    
    // 获取配置
    const MNNConfig& get_config() const { return config_; }
    
    // 设置最大生成长度
    void set_max_tokens(int max_tokens) { max_tokens_ = max_tokens; }
    
    // 获取当前生成长度限制
    int get_max_tokens() const { return max_tokens_; }
    
    // 重置对话状态（清除KV Cache）
    void reset();
    
    // ========== 多模态支持 ==========
    
    // 带图像的聊天完成（非流式）
    ChatCompletionResponse chat_completion_with_image(
        const std::string& prompt,
        const MNNImageInput& image);
    
    // 带图像的聊天完成（流式）
    void chat_completion_with_image_stream(
        const std::string& prompt,
        const MNNImageInput& image,
        std::function<void(const ChatCompletionResponse&)> callback);
    
    // 从文件加载图像
    static std::optional<MNNImageInput> load_image(const std::string& image_path);

private:
    // 初始化MNN-LLM
    bool init_llm();
    
    // 构建Qwen Chat格式的prompt
    std::string build_qwen_prompt(const std::vector<Message>& messages);
    
    // 构建MNN-LLM配置文件
    bool create_config_files(const std::string& model_dir);
    
    // 解析工具调用（Qwen格式）
    void parse_tool_calls(const std::string& output, ChatCompletionResponse& response);
    
    MNNConfig config_;
    std::unique_ptr<MNN::Transformer::Llm> llm_;
    bool model_loaded_ = false;
    bool is_multimodal_ = false;
    int max_tokens_ = 4096;
    
    // 配置文件路径
    std::string config_path_;
    std::string model_dir_;
};

} // namespace icraw

#endif // ICRAW_USE_MNN
