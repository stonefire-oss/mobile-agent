# ROADMAP: Mobile Agent - C++ 移植版

## Milestones

- ✅ **v1.0 C++ 移植** — Phase 1-3 (shipped 2026-03-03)
- ✅ **v1.1 API Key 配置** — Phase 5 (shipped 2026-03-03)
- 🚧 **v1.2 清理 agent 模块** — Phase 6 (in progress)

## Phases

### Phase 6: 清理 agent 模块

**Goal:** 清理 agent 模块中不需要的代码和文件

**Depends on:** v1.1 complete

**Requirements:** CLEAN-01 to CLEAN-07

**Plans:** 1 plan

**Plan list:**
- [ ] 06-01-PLAN.md — 清理未使用的目录和文件

**Success Criteria** (what must be TRUE):
  1. 移除未使用的源文件和头文件
  2. 简化 CMakeLists.txt 配置
  3. 移除不需要的依赖
  4. 修复所有编译警告
  5. 代码风格统一

---

### Phase 5: API Key 配置 (已完成)

**Goal:** 提供配置文件方式设置 LLM API Key

**Depends on:** v1.0 complete

**Requirements:** CONFIG-01

**Success Criteria** (what must be TRUE):
  1. 配置文件格式支持 JSON
  2. 支持配置 API Key 和 Base URL
  3. C++ Agent 启动时读取配置
  4. 配置路径可从 Java 层传入

---

## Progress

| Phase | Milestone | Plans Complete | Status |
|-------|-----------|----------------|--------|
| 1. Build System & Agent Core | v1.0 | 3/3 | Complete |
| 2. JNI Bridge | v1.0 | 1/1 | Complete |
| 3. API Integration | v1.0 | 1/1 | Complete |
| 5. API Key 配置 | v1.1 | 1/1 | Complete |
| 6. 清理 agent 模块 | v1.2 | 0/1 | Not started |

---

*Generated: 2026-03-03*
