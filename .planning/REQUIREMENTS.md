# Requirements: Mobile Agent - v1.6

**Defined:** 2026-03-06
**Core Value:** 在 Android 设备上运行本地 AI Agent，提供实时对话和设备控制能力，无需依赖远程服务器。

## v1.6 Requirements

### 自定义 Skills 机制

- [x] **SKILL-01**: 定义自定义 Skill 的配置文件格式 (JSON/YAML)
- [x] **SKILL-02**: C++ 层加载自定义 Skills 的机制

### Agent 调用 Tools

- [x] **CALL-01**: Agent 能够解析 Skill 定义，调用对应的 Android Tools
- [x] **CALL-02**: 支持多步骤的 Tool 调用链
- [x] **CALL-03**: 处理 Tool 调用结果并返回给 Agent

## Out of Scope

| Feature | Reason |
|---------|--------|
| 动态工具注册 | v1.6 验证机制，后续扩展 |
| MCP Server | 暂不需要，保持简单 |
| 权限系统 | 后续迭代 |
| SKILL-03: Skills 依赖处理 | 暂不需要，简化设计 |
| TEST-01/02/03: 端到端验证 | im_sender 示例已在 v16-02 通过 UAT 验证 |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| SKILL-01: Skill 配置文件格式 | Phase v16-01 | Verified |
| SKILL-02: Skills 加载机制 | Phase v16-01 | Verified |
| CALL-01: Skill 解析调用 | Phase v16-02 | Verified |
| CALL-02: 多步骤 Tool 调用链 | Phase v16-02 | Verified |
| CALL-03: Tool 结果处理 | Phase v16-02 | Verified |

**Coverage:**
- v1.6 requirements: 5 total
- Mapped to phases: 5
- All verified ✓

---
*Requirements defined: 2026-03-06*
