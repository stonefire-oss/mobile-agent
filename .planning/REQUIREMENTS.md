# Requirements: Mobile Agent - v1.5

**Defined:** 2026-03-05
**Core Value:** 在 Android 设备上运行本地 AI Agent，提供实时对话和设备控制能力，无需依赖远程服务器。

## v1.5 Requirements

### 工具管道

- [ ] **PIPE-01**: tools.json 迁移到 inputSchema 格式（MCP 标准）
- [ ] **PIPE-02**: 通用的 call_android_tool 工具（JSON 参数调用任意 Android 功能）
- [ ] **PIPE-03**: 内置工具清单（show_toast，已实现，仅验证框架）

### Skills 编排

- [ ] **SKILL-01**: 定义 Android Skills 加载机制
- [ ] **SKILL-02**: Skill 触发工具调用的模式
- [ ] **SKILL-03**: 示例 Skill（通知用户的工作流）

### 执行模式

- [ ] **MODE-01**: 完全自主调用（无用户确认）
- [ ] **MODE-02**: TODO - 添加需要用户确认的调用模式

## Out of Scope

| Feature | Reason |
|---------|--------|
| 动态工具注册 | v1.5 先做内置清单，后续扩展 |
| MCP Server | 暂不需要，保持简单 |
| 权限系统 | 后续迭代 |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| PIPE-01 | Phase 1 | Pending |
| PIPE-02 | Phase 2 | Pending |
| PIPE-03 | Phase 3 | Pending |
| SKILL-01 | Phase 4 | Pending |
| SKILL-02 | Phase 4 | Pending |
| SKILL-03 | Phase 5 | Pending |
| MODE-01 | Phase 2 | Pending |
| MODE-02 | Future | Pending |

**Coverage:**
- v1.5 requirements: 8 total
- Mapped to phases: 5
- 最小集验证
- Unmapped: 0 ✓

---
*Requirements defined: 2026-03-05*
*Last updated: 2026-03-05 after research*
