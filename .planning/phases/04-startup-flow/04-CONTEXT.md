# Phase 4: 启动流程梳理 - Context

**Gathered:** 2026-03-09
**Status:** Ready for planning

<domain>
## Phase Boundary

梳理启动流程，检查并修复：
1. 内存泄漏风险
2. 主线程阻塞风险

</domain>

<decisions>
## Implementation Decisions

### 1. 内存泄漏检查

#### AgentActivity
- ✅ onDestroy 中正确调用 presenter.detachView() 和 presenter.destroy()
- ⚠️ 需要检查 MessageAdapter 是否有 Context 引用未释放

#### MainPresenter
- ⚠️ executor.shutdown() 是同步的，应该使用 awaitTermination
- ⚠️ Handler 在主线程创建，但如果 View 持有Presenter引用可能导致泄漏

### 2. 主线程阻塞检查

#### AgentActivity.onCreate
- ⚠️ NativeMobileAgentApiAdapter.loadConfigFromAssets() - 同步读取文件
- ⚠️ presenter.loadMessages() - 虽然内部使用 executor，但初始化 createApi() 可能在主线程

### 3. 异步初始化
- 配置加载放到后台线程
- Agent 初始化放到后台线程

</decisions>

<specifics>
## Specific Ideas

- 现有 MVP 架构保持不变
- 异步初始化不改变 UI 交互逻辑

</specifics>

<deferred>
## Deferred Ideas

None

</deferred>

---

*Phase: 04-startup-flow*
*Context gathered: 2026-03-09*
