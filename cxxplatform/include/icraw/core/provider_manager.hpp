#pragma once

#include "icraw/config.hpp"
#include "icraw/core/llm_provider.hpp"
#include <memory>
#include <optional>
#include <atomic>

#ifdef ICRAW_USE_MNN
#include "icraw/core/mnn_provider.hpp"
#endif

namespace icraw {

/**
 * ProviderManager - 管理LLM Provider的创建和切换
 * 
 * 支持：
 * - 默认使用远程API（OpenAI兼容）
 * - 运行时动态切换到MNN本地推理
 * - 条件编译控制MNN可用性
 */
class ProviderManager {
public:
    explicit ProviderManager(const IcrawConfig& config);
    ~ProviderManager() = default;
    
    // 禁止拷贝
    ProviderManager(const ProviderManager&) = delete;
    ProviderManager& operator=(const ProviderManager&) = delete;
    
    // ========== 获取Provider ==========
    
    // 获取当前活跃的provider
    std::shared_ptr<LLMProvider> get_current_provider() const;
    
    // 获取当前provider类型
    ProviderType get_current_type() const;
    
    // 获取provider状态
    ProviderStatus get_status() const;
    
    // ========== 切换Provider ==========
    
    // 切换到远程API
    // 返回: 是否切换成功（总是成功）
    bool switch_to_remote();
    
    // 切换到MNN本地推理
    // 返回: 是否切换成功（模型加载失败返回false）
    // 注意: 仅在ICRAW_USE_MNN编译时可用
#ifdef ICRAW_USE_MNN
    bool switch_to_mnn(const MNNConfig& config);
#endif
    
    // 通用切换接口
    // mnn_config: 切换到MNN时需要提供配置
    bool switch_provider(ProviderType type, 
                        const std::optional<MNNConfig>& mnn_config = std::nullopt);
    
    // ========== MNN可用性检查 ==========
    
    // 检查MNN是否可用（编译期 + 运行时）
    static bool is_mnn_available();
    
    // ========== MNN预加载（后台加载，不阻塞） ==========
    
#ifdef ICRAW_USE_MNN
    // 在后台线程预加载MNN模型
    void preload_mnn(const MNNConfig& config);
    
    // 检查MNN是否已预加载完成
    bool is_mnn_preloaded() const;
    
    // 获取预加载的MNN provider（用于检查加载状态）
    std::shared_ptr<MNNProvider> get_preloaded_mnn() const;
#endif

private:
    // 创建远程API provider
    std::shared_ptr<LLMProvider> create_remote_provider();
    
    IcrawConfig config_;
    ProviderType current_type_ = ProviderType::RemoteAPI;
    std::shared_ptr<LLMProvider> current_provider_;
    
#ifdef ICRAW_USE_MNN
    std::shared_ptr<MNNProvider> mnn_provider_;
    std::shared_ptr<MNNProvider> preloaded_mnn_provider_;
    std::atomic<bool> mnn_preloaded_{false};
    std::atomic<bool> mnn_preloading_{false};
#endif
};

} // namespace icraw
