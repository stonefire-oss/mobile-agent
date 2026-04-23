# Web DOM 与 Native XML 元素提取方案设计（基于当前实现）

> 范围说明：本文只描述仓库当前代码中的**既有设计与执行链路**，用于架构对齐与评审输入。
> 非目标：不提出改造方案、不包含优化路线图。

## 1. 目标与上下文

项目当前在 UI 元素定位上存在两条并行链路：

1. **WebView/H5 链路**：基于 Web DOM 快照与 Web Action 执行。
2. **Native Android 链路**：基于 Activity View 层级 XML 与 Gesture 执行。

两条链路最终都会进入统一 observation 结构（`uiTree`、`screenElements`、`snapshotId` 等），作为动作工具输入。

## 2. Web DOM 设计（当前实现）

### 2.1 流程分层

当前 Web 侧可映射为以下阶段：

1. **Live DOM**：在当前前台 WebView 上执行 JS，读取页面 DOM 信息。
2. **DOM 提取**：脚本在页面内递归采样 DOM tree，提取 tag/text/ref/selector/bounds 等字段。
3. **扁平化索引**：从树结构中收集可执行元素列表（`screenElements`）。
4. **简化结构**：输出 canonical `uiTree + screenElements + quality + raw`。
5. **动作执行**：消费 `snapshotId + ref/selector` 执行 click/input/scroll/eval。
6. **执行反馈**：返回执行结果与关键定位信息（ref、selector、normalized 坐标等）。

### 2.2 关键组件

- **快照采集**：`RealWebDomSnapshotProvider`
  - 选择前台 WebView；
  - 注入 `WebDomScriptFactory.buildSnapshotScript()`；
  - 解析结果后写入 `ViewObservationSnapshotRegistry`。
- **DOM 脚本构造**：`WebDomScriptFactory`
  - 包含 `MAX_DEPTH/MAX_NODES/MAX_SERIALIZED_CHARS` 约束；
  - 为每个节点注入 `data-agent-ref`；
  - 提供 click/input/scroll 对应脚本。
- **canonical 适配**：`WebDomObservationAdapter`
  - DOM tree → `uiTree`；
  - 可执行候选 → `screenElements`；
  - 产出 `quality`（节点数、是否截断、最大深度）。
- **动作执行**：`RealWebActionExecutor`
  - 校验 observation 是否携带且命中最新 `snapshotId`；
  - 仅在 `source=web_dom && interactionDomain=web` 时执行；
  - 支持 `click/input/scroll_to_bottom/eval_js`。

### 2.3 Web 侧输入输出要点

- 输入（观测阶段）：`targetHint`（可选）。
- 输出（观测阶段）：`snapshotId`、`pageUrl`、`pageTitle`、`uiTree`、`screenElements`、`quality`、`raw`、`webDom`。
- 输入（执行阶段）：`action` + `observation.snapshotId` + `ref/selector/...`。
- 输出（执行阶段）：`success/error` + action 结果字段（如 `value`、`before/after`、`normalizedX/Y`）。

## 3. Native XML 设计（当前实现）

### 3.1 流程分层

当前 Native 侧可映射为以下阶段：

1. **Live UI**：获取稳定前台 Activity（处理悬浮容器切换）。
2. **XML 提取**：遍历 DecorView，导出 XML-like hierarchy。
3. **扁平化索引**：解析 XML 收集可执行/可读元素到 `screenElements`。
4. **简化结构**：输出 canonical `uiTree + screenElements + quality + raw`。
5. **动作执行**：消费 `snapshotId + referencedBounds` 执行 tap/swipe。
6. **执行反馈**：返回注入点、命中视图、dispatch 统计等执行结果。

### 3.2 关键组件

- **层级采集**：`InProcessViewHierarchyDumper`
  - 等待稳定前台页面；
  - 遍历可见节点，导出 `<hierarchy><node ...>`；
  - 节点数量受 `MAX_NODES` 限制。
- **canonical 适配**：`NativeXmlObservationAdapter`
  - XML 解析为 `Document`；
  - 转换为 `uiTree`；
  - 递归收集 `screenElements`；
  - 产出 `quality`（`nativeNodeCount`）。
- **动作执行**：`InProcessGestureExecutor`
  - 校验 `observation.snapshotId` 是否为最新 snapshot；
  - tap 优先使用 `observation.referencedBounds` 解析目标点；
  - swipe 要求提供滚动容器 `referencedBounds`。

### 3.3 Native 侧输入输出要点

- 输入（观测阶段）：`targetHint`（可选）。
- 输出（观测阶段）：`snapshotId`、`activityClassName`、`uiTree`、`screenElements`、`quality`、`raw`、`nativeViewXml`。
- 输入（执行阶段）：`action` + `observation.snapshotId` + `referencedBounds/...`。
- 输出（执行阶段）：`success/error` + 注入/分发/目标解析相关字段。

## 4. 两条链路的统一接口（当前实现）

### 4.1 统一观测模型

当前实现通过 `UnifiedViewObservationFacade` 将不同 source（`native_xml`、`web_dom`、`screen_snapshot` 等）收敛为统一结构，核心字段包括：

- `pageSummary`
- `uiTree`
- `screenElements`
- `quality`
- `raw`

### 4.2 Observation-Bound 执行约束

当前执行链路采用 observation-bound 模式：

- 先观测，再执行；
- 执行请求必须引用 `observation.snapshotId`；
- 运行时校验 snapshot 是否为当前最新且域匹配；
- 校验不通过返回 stale/missing 相关错误。

### 4.3 Web 与 Native 的执行分域

- Web DOM 元素动作：`android_web_action_tool`。
- Native 界面动作：`android_gesture_tool`。

两类动作共享 observation 引用机制，但执行器与参数细节按 domain 分离。

## 5. 属性列表（UI 定位输入）在当前实现中的形态

在当前代码里，“属性列表”主要对应 canonical `screenElements` 与 `uiTree` 中节点属性。典型字段来源如下：

- **Web**：`ref`、`tagName`、`selector`、`text`、`ariaLabel`、`bounds`、`clickable`、`inputable`。
- **Native**：`className`、`text`、`bounds`、`clickable`（以及 `index` 等结构字段）。

执行阶段一般以：

- Web：`snapshotId + ref/selector`
- Native：`snapshotId + referencedBounds`

作为主定位输入。

## 6. 时序总览（当前实现）

1. Agent 调用 `android_view_context_tool` 指定 source（`web_dom`/`native_xml`/...）。
2. Source Provider 采集并构建 observation。
3. 系统注册 snapshot 并返回 `snapshotId + canonical fields`。
4. Agent 基于返回元素定位目标，构造动作请求。
5. Agent 调用 `android_web_action_tool` 或 `android_gesture_tool`。
6. 执行器校验 snapshot 并执行动作，返回结果。

---

以上即当前项目内 Web DOM 与 Native XML 元素提取/执行的既有方案设计说明。
