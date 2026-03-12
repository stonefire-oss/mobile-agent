#include "icraw/mobile_agent.hpp"
#include "icraw/core/memory_manager.hpp"
#include "icraw/core/skill_loader.hpp"
#include "icraw/tools/tool_registry.hpp"
#include "icraw/core/prompt_builder.hpp"
#include "icraw/core/agent_loop.hpp"
#include "icraw/core/llm_provider.hpp"
#include "icraw/core/http_client.hpp"
#include "icraw/core/provider_manager.hpp"
#include "icraw/core/logger.hpp"

namespace icraw {

// MobileAgent implementation
MobileAgent::MobileAgent(const IcrawConfig& config)
    : config_(config) {
    
    // Initialize logger if configured
    if (config_.logging.enabled && !config_.logging.directory.empty()) {
        Logger::get_instance().init(config_.logging.directory, config_.logging.level);
    }
    
    // Ensure workspace exists
    std::filesystem::create_directories(config_.workspace_path);
    
    // Initialize components
    memory_manager_ = std::make_shared<MemoryManager>(config_.workspace_path);
    skill_loader_ = std::make_shared<SkillLoader>();
    tool_registry_ = std::make_shared<ToolRegistry>();
    tool_registry_->set_base_path(config_.workspace_path.string());
    tool_registry_->set_memory_manager(memory_manager_.get());
    tool_registry_->register_builtin_tools();
    
    prompt_builder_ = std::make_shared<PromptBuilder>(
        memory_manager_, skill_loader_, tool_registry_);
    
    // Initialize ProviderManager (默认使用远程API)
    provider_manager_ = std::make_shared<ProviderManager>(config_);
    llm_provider_ = provider_manager_->get_current_provider();
    
    ICRAW_LOG_INFO("MobileAgent initialized with provider: {}", 
                   provider_manager_->get_status().provider_name);
    
    agent_loop_ = std::make_unique<AgentLoop>(
        memory_manager_, skill_loader_, tool_registry_, 
        llm_provider_, config_.agent);
    
    // Load conversation history from database
    load_history_from_memory();
    
    // Build system prompt (passing skills config from user config)
    system_prompt_ = prompt_builder_->build_full(config_.skills);
}

void MobileAgent::load_history_from_memory() {
    // Load recent messages from database into history
    auto memory_entries = memory_manager_->get_recent_messages(
        config_.agent.memory_window, "default");
    
    history_.clear();
    history_.reserve(memory_entries.size());
    
    for (const auto& entry : memory_entries) {
        // Skip tool messages - they require corresponding tool_calls in assistant messages
        // which we don't store in the database. Loading only user/assistant messages
        // preserves the conversation context without breaking API requirements.
        if (entry.role == "tool") {
            continue;
        }
        
        Message msg;
        msg.role = entry.role;
        msg.content.push_back(ContentBlock::make_text(entry.content));
        history_.push_back(std::move(msg));
    }
    
    ICRAW_LOG_DEBUG("Loaded {} messages from memory into history", history_.size());
}

MobileAgent::~MobileAgent() = default;

std::string MobileAgent::chat(const std::string& message) {
    auto new_messages = agent_loop_->process_message(
        message, history_, system_prompt_);
    
    // Add new messages to history
    for (const auto& msg : new_messages) {
        history_.push_back(msg);
    }
    
    // Return the last assistant message text
    for (auto it = new_messages.rbegin(); it != new_messages.rend(); ++it) {
        if (it->role == "assistant") {
            return it->text();
        }
    }
    
    return "";
}

void MobileAgent::chat_stream(const std::string& message, AgentEventCallback callback) {
    ICRAW_LOG_DEBUG("[CHAT_STREAM] Starting process_message_stream");
    auto new_messages = agent_loop_->process_message_stream(
        message, history_, system_prompt_, callback);
    ICRAW_LOG_DEBUG("[CHAT_STREAM] process_message_stream returned, messages={}", new_messages.size());
    
    // Add new messages to history and memory
    for (const auto& msg : new_messages) {
        history_.push_back(msg);
        
        // Save to memory manager
        if (!msg.content.empty()) {
            std::string content;
            for (const auto& block : msg.content) {
                if (!block.text.empty()) {
                    content += block.text + " ";
                }
            }
            if (!content.empty()) {
                nlohmann::json metadata;
                // Store tool calls if present
                if (!msg.content.empty() && msg.content[0].type == "tool_use") {
                    metadata["is_tool_call"] = true;
                }
                memory_manager_->add_message(msg.role, content, "default", metadata);
            }
        }
    }
    
    ICRAW_LOG_DEBUG("[CHAT_STREAM] Starting maybe_consolidate_memory");
    // Trigger memory consolidation if needed
    // Note: This is now non-blocking - consolidation runs asynchronously
    // to avoid delaying the user experience after message_end event
    agent_loop_->maybe_consolidate_memory(new_messages);
    ICRAW_LOG_DEBUG("[CHAT_STREAM] chat_stream completed");
}

void MobileAgent::clear_history() {
    history_.clear();
}

void MobileAgent::stop() {
    agent_loop_->stop();
}

std::unique_ptr<MobileAgent> MobileAgent::create(const std::string& workspace_path) {
    IcrawConfig config = IcrawConfig::load_default();
    
    if (!workspace_path.empty()) {
        config.workspace_path = std::filesystem::path(workspace_path);
    }
    
    return std::make_unique<MobileAgent>(config);
}

std::unique_ptr<MobileAgent> MobileAgent::create_with_config(const IcrawConfig& config) {
    return std::make_unique<MobileAgent>(config);
}

// ========== Provider管理接口实现 ==========

ProviderType MobileAgent::get_current_provider_type() const {
    return provider_manager_->get_current_type();
}

ProviderStatus MobileAgent::get_provider_status() const {
    return provider_manager_->get_status();
}

bool MobileAgent::switch_to_remote_provider() {
    ICRAW_LOG_INFO("MobileAgent: Switching to remote provider");
    
    if (provider_manager_->switch_to_remote()) {
        llm_provider_ = provider_manager_->get_current_provider();
        agent_loop_->set_llm_provider(llm_provider_);
        ICRAW_LOG_INFO("MobileAgent: Switched to remote provider successfully");
        return true;
    }
    return false;
}

#ifdef ICRAW_USE_MNN
bool MobileAgent::switch_to_mnn_provider(const MNNConfig& config) {
    ICRAW_LOG_INFO("MobileAgent: Switching to MNN provider");
    
    if (provider_manager_->switch_to_mnn(config)) {
        llm_provider_ = provider_manager_->get_current_provider();
        agent_loop_->set_llm_provider(llm_provider_);
        ICRAW_LOG_INFO("MobileAgent: Switched to MNN provider successfully");
        return true;
    }
    ICRAW_LOG_ERROR("MobileAgent: Failed to switch to MNN provider");
    return false;
}

void MobileAgent::preload_mnn_model(const MNNConfig& config) {
    ICRAW_LOG_INFO("MobileAgent: Preloading MNN model");
    provider_manager_->preload_mnn(config);
}

bool MobileAgent::is_mnn_model_preloaded() const {
    return provider_manager_->is_mnn_preloaded();
}
#endif

bool MobileAgent::is_mnn_available() {
    return ProviderManager::is_mnn_available();
}

bool MobileAgent::switch_provider(ProviderType type,
                                  const std::optional<MNNConfig>& mnn_config) {
    ICRAW_LOG_INFO("MobileAgent: Switching provider to type {}", static_cast<int>(type));
    
    if (provider_manager_->switch_provider(type, mnn_config)) {
        llm_provider_ = provider_manager_->get_current_provider();
        agent_loop_->set_llm_provider(llm_provider_);
        return true;
    }
    return false;
}

std::shared_ptr<ProviderManager> MobileAgent::get_provider_manager() const {
    return provider_manager_;
}

} // namespace icraw
