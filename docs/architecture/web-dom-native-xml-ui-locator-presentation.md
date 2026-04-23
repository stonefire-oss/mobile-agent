---
marp: true
theme: default
paginate: true
title: Web UI 与 Native UI 元素提取汇聚方案
---

# Web UI 与 Native UI 元素提取汇聚方案

## 基于当前实现的架构说明

- 文档来源：`web-dom-native-xml-ui-locator-design.md`
- 目标：说明 **Web UI** 与 **Native UI** 最终都汇聚为统一的界面元素描述

---

## 1. 背景

当前项目存在两条并行链路：

- WebView/H5：`web_dom` 观测 + `android_web_action_tool` 执行
- Native Android：`native_xml` 观测 + `android_gesture_tool` 执行

两条链路虽采集方式不同，但都输出到统一 observation 结构。

---

## 2. Web UI 当前链路（6阶段）

1. Live DOM（WebView 注入 JS）
2. DOM 提取（tag/text/ref/selector/bounds）
3. 扁平化索引（`screenElements`）
4. 简化结构（`uiTree + quality + raw`）
5. 动作执行（`snapshotId + ref/selector`）
6. 执行反馈（结果 + 定位字段）

**关键组件**：
`RealWebDomSnapshotProvider` / `WebDomScriptFactory` / `WebDomObservationAdapter` / `RealWebActionExecutor`

---

## 3. Native UI 当前链路（6阶段）

1. Live UI（稳定前台 Activity）
2. XML 提取（DecorView 层级导出）
3. 扁平化索引（`screenElements`）
4. 简化结构（`uiTree + quality + raw`）
5. 动作执行（`snapshotId + referencedBounds`）
6. 执行反馈（注入点/命中视图/dispatch）

**关键组件**：
`InProcessViewHierarchyDumper` / `NativeXmlObservationAdapter` / `InProcessGestureExecutor`

---

## 4. 汇聚点：统一界面元素描述

无论 Web 还是 Native，最终都映射到统一字段族：

- `snapshotId`
- `uiTree`
- `screenElements`
- `quality`
- `raw`

这就是当前系统的“界面元素描述”汇聚层。

---

## 5. 汇聚后的输入形态（示意）

### Web 侧常见元素属性
- `ref`, `tagName`, `selector`, `text`, `ariaLabel`, `bounds`, `clickable`, `inputable`

### Native 侧常见元素属性
- `className`, `text`, `bounds`, `clickable`, `index`

### 执行输入
- Web：`snapshotId + ref/selector`
- Native：`snapshotId + referencedBounds`

---

## 6. 统一执行约束（Observation-Bound）

- 先观测，再执行
- 执行请求必须携带 `observation.snapshotId`
- 执行时必须校验 snapshot 新鲜度与 domain/source 匹配
- 不匹配返回 stale/missing 错误

> 作用：把“看到的页面”与“执行的动作”绑定到同一轮上下文。

---

## 7. 端到端时序（当前实现）

1. Agent 调用 `android_view_context_tool`
2. Provider 采集并构建 observation
3. 返回 `snapshotId + canonical fields`
4. Agent 基于元素描述构造动作
5. 调用 `android_web_action_tool` 或 `android_gesture_tool`
6. 执行器校验 snapshot 后执行并返回结果

---

## 8. 核心结论

- **采集分域**：Web DOM 与 Native XML 各自独立采集
- **描述汇聚**：统一收敛到 `uiTree/screenElements/snapshotId`
- **执行分域**：Web 与 Native 执行器分离
- **协议统一**：通过 observation-bound 保证执行一致性

---

## 9. 一页图（可直接放汇报）

```text
Web UI (DOM) ─┐
              ├─> Canonical Observation (uiTree + screenElements + snapshotId) ──> Action Tools
Native UI(XML)┘
```

即：**Web UI 和 Native UI 最终都汇聚到“界面元素描述”再进入执行。**
