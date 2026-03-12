#include "icraw/core/provider_manager.hpp"
#include "icraw/core/http_client.hpp"
#include "icraw/core/logger.hpp"
#include <thread>

namespace icraw {

ProviderManager::ProviderManager(const IcrawConfig& config)
    : config_(config) {
    // 默认使用远程API
    current_provider_ = create_remote_provider();
    ICRAW_LOG_INFO("ProviderManager initialized with RemoteAPI provider");
}

std::shared_ptr<LLMProvider> ProviderManager::create_remote_provider() {
    auto provider = std::make_shared<OpenAICompatibleProvider>(
        config_.provider.api_key,
        config_.provider.base_url,
        config_.agent.model);
    
    auto http_client = std::make_unique<CurlHttpClient>();
    provider->set_http_client(std::move(http_client));
    
    return provider;
}

std::shared_ptr<LLMProvider> ProviderManager::get_current_provider() const {
    return current_provider_;
}

ProviderType ProviderManager::get_current_type() const {
    return current_type_;
}

ProviderStatus ProviderManager::get_status() const {
    ProviderStatus status;
    status.current_type = current_type_;
    
    if (current_provider_) {
        status.provider_name = current_provider_->get_provider_name();
        status.is_ready = true;
    } else {
        status.provider_name = "None";
        status.is_ready = false;
        status.error_message = "No provider available";
    }
    
    return status;
}

bool ProviderManager::switch_to_remote() {
    ICRAW_LOG_INFO("Switching to RemoteAPI provider");
    
    current_provider_ = create_remote_provider();
    current_type_ = ProviderType::RemoteAPI;
    
    ICRAW_LOG_INFO("RemoteAPI provider activated");
    return true;
}

#ifdef ICRAW_USE_MNN
bool ProviderManager::switch_to_mnn(const MNNConfig& config) {
    ICRAW_LOG_INFO("Switching to MNN provider, model: {}", config.model_path);
    
    // 如果已预加载且配置相同，直接使用
    if (mnn_preloaded_ && preloaded_mnn_provider_) {
        current_provider_ = preloaded_mnn_provider_;
        current_type_ = ProviderType::MNN;
        mnn_provider_ = preloaded_mnn_provider_;
        ICRAW_LOG_INFO("Using preloaded MNN provider");
        return true;
    }
    
    // 创建新的MNN provider
    try {
        auto new_provider = std::make_shared<MNNProvider>(config);
        if (new_provider->is_model_loaded()) {
            current_provider_ = new_provider;
            current_type_ = ProviderType::MNN;
            mnn_provider_ = new_provider;
            ICRAW_LOG_INFO("MNN provider switched successfully");
            return true;
        } else {
            ICRAW_LOG_ERROR("MNN model failed to load: {}", config.model_path);
            return false;
        }
    } catch (const std::exception& e) {
        ICRAW_LOG_ERROR("MNN provider creation failed: {}", e.what());
        return false;
    }
}

void ProviderManager::preload_mnn(const MNNConfig& config) {
    if (mnn_preloading_ || mnn_preloaded_) {
        ICRAW_LOG_DEBUG("MNN already preloading or preloaded, skipping");
        return;
    }
    
    mnn_preloading_ = true;
    ICRAW_LOG_INFO("Starting MNN model preload in background thread");
    
    // 在后台线程加载
    std::thread([this, config]() {
        try {
            auto provider = std::make_shared<MNNProvider>(config);
            if (provider->is_model_loaded()) {
                preloaded_mnn_provider_ = provider;
                mnn_preloaded_ = true;
                ICRAW_LOG_INFO("MNN model preloaded successfully");
            } else {
                ICRAW_LOG_ERROR("MNN model preload failed");
            }
        } catch (const std::exception& e) {
            ICRAW_LOG_ERROR("MNN preload exception: {}", e.what());
        }
        mnn_preloading_ = false;
    }).detach();
}

bool ProviderManager::is_mnn_preloaded() const {
    return mnn_preloaded_;
}

std::shared_ptr<MNNProvider> ProviderManager::get_preloaded_mnn() const {
    return preloaded_mnn_provider_;
}
#endif

bool ProviderManager::is_mnn_available() {
#ifdef ICRAW_USE_MNN
    return true;
#else
    return false;
#endif
}

bool ProviderManager::switch_provider(ProviderType type,
                                      const std::optional<MNNConfig>& mnn_config) {
    switch (type) {
        case ProviderType::RemoteAPI:
            return switch_to_remote();
            
        case ProviderType::MNN:
#ifdef ICRAW_USE_MNN
            if (mnn_config.has_value()) {
                return switch_to_mnn(mnn_config.value());
            } else if (mnn_preloaded_ && preloaded_mnn_provider_) {
                current_provider_ = preloaded_mnn_provider_;
                current_type_ = ProviderType::MNN;
                mnn_provider_ = preloaded_mnn_provider_;
                ICRAW_LOG_INFO("Using preloaded MNN provider");
                return true;
            }
            ICRAW_LOG_ERROR("MNN switch failed: no config provided and not preloaded");
            return false;
#else
            ICRAW_LOG_ERROR("MNN not available: compiled without ICRAW_USE_MNN");
            return false;
#endif
    }
    
    ICRAW_LOG_ERROR("Unknown provider type: {}", static_cast<int>(type));
    return false;
}

} // namespace icraw
