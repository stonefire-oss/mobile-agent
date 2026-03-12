#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include "icraw/types.hpp"
#include "icraw/config.hpp"

namespace icraw {

class MemoryManager;
class SkillLoader;
class ToolRegistry;
class PromptBuilder;
class AgentLoop;
class LLMProvider;
class ProviderManager;

// Mobile Agent - Simplified facade for mobile platforms
// Single session, no gateway, no shell commands
class MobileAgent {
public:
    explicit MobileAgent(const IcrawConfig& config);
    ~MobileAgent();

    // Send a message and get response
    std::string chat(const std::string& message);

    // Send a message with streaming callback
    void chat_stream(const std::string& message, AgentEventCallback callback);

    // Get conversation history
    const std::vector<Message>& get_history() const { return history_; }

    // Clear conversation history
    void clear_history();

    // Stop current operation
    void stop();

    // Get underlying components (for advanced usage)
    std::shared_ptr<MemoryManager> get_memory_manager() const { return memory_manager_; }
    std::shared_ptr<ToolRegistry> get_tool_registry() const { return tool_registry_; }
    std::shared_ptr<LLMProvider> get_llm_provider() const { return llm_provider_; }

    // Factory method with default configuration
    static std::unique_ptr<MobileAgent> create(const std::string& workspace_path = "");
    
    // Factory method with custom configuration
    static std::unique_ptr<MobileAgent> create_with_config(const IcrawConfig& config);

    // ========== Provider管理接口（新增） ==========
    
    // 获取当前provider类型
    ProviderType get_current_provider_type() const;
    
    // 获取provider状态
    ProviderStatus get_provider_status() const;
    
    // 切换到远程API
    bool switch_to_remote_provider();
    
    // 切换到MNN本地推理（条件编译可用时）
#ifdef ICRAW_USE_MNN
    bool switch_to_mnn_provider(const MNNConfig& config);
#endif
    
    // 通用切换接口
    bool switch_provider(ProviderType type,
                        const std::optional<MNNConfig>& mnn_config = std::nullopt);
    
    // 检查MNN是否可用（编译期）
    static bool is_mnn_available();
    
    // 预加载MNN模型（后台加载，不阻塞）
#ifdef ICRAW_USE_MNN
    void preload_mnn_model(const MNNConfig& config);
    bool is_mnn_model_preloaded() const;
#endif
    
    // 获取provider管理器（高级用法）
    std::shared_ptr<ProviderManager> get_provider_manager() const;

private:
    // Load conversation history from database
    void load_history_from_memory();
    
    IcrawConfig config_;
    std::shared_ptr<MemoryManager> memory_manager_;
    std::shared_ptr<SkillLoader> skill_loader_;
    std::shared_ptr<ToolRegistry> tool_registry_;
    std::shared_ptr<PromptBuilder> prompt_builder_;
    std::shared_ptr<LLMProvider> llm_provider_;
    std::shared_ptr<ProviderManager> provider_manager_;
    std::unique_ptr<AgentLoop> agent_loop_;

    std::vector<Message> history_;
    std::string system_prompt_;
};

} // namespace icraw
