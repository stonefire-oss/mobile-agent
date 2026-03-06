# Phase v20-03: 代码迁移 - Context

**Gathered:** 2026-03-06
**Status:** Ready for planning

<domain>
## Phase Boundary

将平台相关逻辑从 agent 模块上移到 app 模块。agent 模块保留 C++ 核心 + Android 管道 + JNI 适配作为 AAR 能力，app 模块实现具体的平台相关调用。

</domain>

<decisions>
## Implementation Decisions

### 迁移范围
- **保留在 agent (AAR 能力):**
  - C++ 核心引擎 (JNI 层)
  - MobileAgentApi / NativeMobileAgentApi 接口
  - AndroidToolManager + Tool 实现 (show_toast, display_notification 等)
  - AndroidToolCallback 接口

- **移至 app 模块:**
  - MainPresenter 中的平台调用逻辑
  - NativeMobileAgentApiAdapter 的具体实现
  - Activity/Context 生命周期管理

### 架构方式
- agent 提供 Android Tool 能力作为 AAR
- app 通过 AAR 依赖调用 agent 能力
- 不移动 Tool 实现代码，保持在 agent 中

### 执行顺序
1. 分析 agent 模块当前代码结构
2. 识别需要在 app 中保留的调用逻辑
3. 调整 MainPresenter 与 agent 的交互方式
4. 验证编译通过

</decisions>

<codebase_context>
## Existing Code Insights

### Reusable Assets
- MobileAgentApi 接口 — app 通过此接口与 agent 交互
- NativeMobileAgentApiAdapter — 当前在 app 中作为适配器

### Integration Points
- MainPresenter 调用 mobileAgentApi 方法
- MainActivity 管理 Activity Context

### 代码分布
agent 模块:
- AndroidToolManager.java (依赖 android.content.Context)
- tools/ShowToastTool.java 等 (依赖 Android API)
- NativeMobileAgentApi.java (JNI 绑定)

app 模块:
- MainPresenter.java (业务逻辑)
- MainActivity.java (UI)
- NativeMobileAgentApiAdapter.java (适配器)

</codebase_context>

<specifics>
## Specific Ideas

无特定需求，标准代码迁移操作。

</specifics>

<deferred>
## Deferred Ideas

- AAR 打包配置 — v2.0 里程碑的 out of scope 项
- 验证测试 — v20-04 阶段

</deferred>

---

*Phase: v20-03-code-migration*
*Context gathered: 2026-03-06*
