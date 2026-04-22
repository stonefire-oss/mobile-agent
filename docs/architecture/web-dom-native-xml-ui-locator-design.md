# Web DOM 与 Native XML 元素提取双轨设计方案（调研 + 落地）

## 1. 背景与目标

当前项目已形成两条页面观测与执行链路：

1. **WebView/H5 链路**：通过 JS 注入采集 DOM，再由 `android_web_action_tool` 或本地注入完成动作。
2. **Native Android 链路**：通过前台 Activity 的 View 层级导出 XML，再由 `android_gesture_tool` 基于 observation 执行动作。

两条链路最终都会产出结构化元素数据（`uiTree` + `screenElements`），并通过 `snapshotId` 与执行阶段绑定，形成 observation-bound 闭环。

本方案目标：

- 对你提出的两种设计做并行调研与拆解；
- 给出统一的“属性列表（UI locator）输入模型”；
- 给出一套可分阶段落地的架构升级方案，降低误点、提高可解释性与跨页面稳定性。

---

## 2. 现状调研结论

### 2.1 Web DOM 五阶段流水线（现状）

从实现看，Web 侧已经具备“采集-映射-执行-校验”的闭环：

1. **Live DOM 采集**：`RealWebDomSnapshotProvider` 通过 `WebViewJsBridge` 执行脚本，读取当前 WebView 的 DOM 快照。  
2. **DOM 提取与裁剪**：`WebDomScriptFactory.buildSnapshotScript()` 在浏览器上下文中做深度/节点数/序列化长度限制（`MAX_DEPTH/MAX_NODES/MAX_SERIALIZED_CHARS`）。  
3. **扁平化与 canonical 映射**：`WebDomObservationAdapter` 将 DOM tree 映射到 canonical `uiTree`，并收集 `screenElements`。  
4. **动作执行**：`RealWebActionExecutor` 使用 `ref/selector/snapshotId` 执行 `click/input/scroll/eval_js`，并做快照新鲜度校验。  
5. **执行后回传**：返回 action 结果和定位字段（如 `ref`、`selector`、归一化坐标），用于后续推理和追踪。

**优点**

- 有明确的采集限制与防爆量机制；
- DOM 节点具备 `ref`（`data-agent-ref`）可追踪性强；
- 行为执行前会校验 `snapshotId`，能阻断“旧页面点新页面”问题。

**问题**

- 当前“简化 HTML”更偏规则裁剪，缺少任务语义导向（如“只保留可交互 + 与 targetHint 相关”）；
- `clickable/inputable/text` 规则较粗，可能引入文本噪声节点；
- 执行后主要回传结果，缺少“视觉一致性确认（pre/post diff）”的标准化评分。

### 2.2 Native XML 元素提取链路（现状）

Native 侧链路是“视图树采集 -> canonical 映射 -> 手势执行”：

1. **前台稳定 Activity 获取**：`InProcessViewHierarchyDumper` 会等待稳定前台页面并处理浮层容器切换。  
2. **View 树导出 XML-like 结构**：按可见性和节点数上限（`MAX_NODES`）导出 `<hierarchy><node ...>`。  
3. **canonical 转换**：`NativeXmlObservationAdapter` 解析 XML 到 `uiTree/screenElements`。  
4. **动作执行**：`InProcessGestureExecutor` 基于 `observation.snapshotId`、`referencedBounds` 执行 tap/swipe。  
5. **执行校验**：运行时校验 snapshot 新鲜度 + activity 一致性，避免跨页面注入。

**优点**

- 与 Android 原生事件模型一致，执行可靠；
- 通过 `referencedBounds` 将“语义目标”落到几何坐标，工程可控；
- 与 observation 协议结合紧密，日志可追溯。

**问题**

- XML 节点属性目前偏少（例如未系统化输出 `enabled/focusable/checkable/scrollable` 等）；
- `isActionable` 对“有 text 即可入池”的策略会带来候选污染；
- 缺少“跨 source 一致的 locator 优先级”，导致 Web 与 Native 的定位表达仍存在割裂。

---

## 3. 两条链路的共同抽象：统一 UI 属性列表

建议将 Web DOM 与 Native XML 最终统一为同一份 `LocatorCandidate` 列表（即你说的“属性列表”），作为 LLM 与执行器的唯一主输入：

```json
{
  "snapshotId": "obs_xxx",
  "interactionDomain": "web|native",
  "candidates": [
    {
      "nodeId": "web:node-17 | native:42",
      "source": "web_dom|native_xml",
      "role": "button|input|link|list_item|text",
      "text": "提交",
      "contentDesc": "",
      "resourceId": "",
      "className": "android.widget.Button",
      "tagName": "button",
      "ref": "node-17",
      "selector": "button#submit",
      "bounds": "[l,t][r,b] | {x,y,width,height}",
      "clickable": true,
      "inputable": false,
      "enabled": true,
      "visible": true,
      "score": 0.92,
      "scoreBreakdown": {
        "actionability": 0.4,
        "semantic": 0.3,
        "geometry": 0.2,
        "freshness": 0.1
      }
    }
  ]
}
```

核心点：

- **统一字段，不统一底层采集方式**；
- LLM 只理解 `LocatorCandidate`，执行器再按 `interactionDomain` 路由到 web_action 或 gesture；
- `snapshotId` 仍作为强约束主键。

---

## 4. 目标架构：双轨采集 + 单轨定位 + 分域执行

你给出的五阶段可以升级为统一六阶段（两条链路共享）：

1. **Live UI Capture**  
   - Web：Live DOM。
   - Native：Live View Hierarchy XML。
2. **Structured Extraction**  
   - 提取完整树结构 + 原始关键属性。
3. **Flatten & Index**  
   - 生成扁平候选池，建立 `nodeId/ref/index/path` 索引。
4. **Semantic Simplification**  
   - 面向任务裁剪（targetHint、动作类型、页面区域），不是只做静态字段截断。
5. **Action Execution**  
   - Web：`android_web_action_tool`。
   - Native：`android_gesture_tool`。
6. **Visual/State Feedback**  
   - 统一执行后反馈（成功性、页面变化、目标一致性、风险标签），驱动下一轮决策。

---

## 5. 关键设计细节

### 5.1 候选评分模型（替代“文本即候选”）

对每个 candidate 计算统一分：

- `actionability`：clickable/inputable/enabled/visible 等；
- `semantic`：文本、aria、content-desc、resource-id 与目标语义匹配；
- `geometry`：尺寸、位置、遮挡风险、容器可达性；
- `freshness`：snapshot 与当前前台状态一致性。

建议阈值：

- `score >= 0.75`：可直接执行候选；
- `0.45 ~ 0.75`：需要二次确认（或补观测）；
- `< 0.45`：仅作为语义锚点，不进执行池。

### 5.2 Web 与 Native 的 Locator 优先级

- **Web**：`ref > selector > text+geometry fallback`。
- **Native**：`resource-id/content-desc > bounds+class > xy fallback`。

统一要求：

- 任何 fallback 均必须带 `targetDescriptor`，并在执行后做语义一致性校验；
- 低置信 fallback 必须触发自动重观测。

### 5.3 视觉反馈标准化

新增统一 `postActionFeedback`：

- `domChanged` / `activityChanged` / `urlChanged`；
- `targetStillVisible`；
- `stateDeltaSummary`（如“输入框值变化”“列表滚动位移”）；
- `confidenceAfterAction`。

这部分用于替代当前“只返回执行成功”的粗粒度判断。

### 5.4 协议兼容策略

保持现有字段不破坏：

- 保留 `uiTree/screenElements/raw`；
- 新增 `locatorCandidates` 与 `candidateVersion`；
- 执行工具优先消费 `locatorCandidates`，旧字段作为 fallback。

---

## 6. 分阶段落地计划

### Phase A（1~2 周）：统一数据模型

- 定义 `LocatorCandidate` schema；
- 在 WebDomObservationAdapter / NativeXmlObservationAdapter 同步产出 `locatorCandidates`；
- 增加候选统计日志（数量、置信度分布、过滤原因）。

### Phase B（2~3 周）：执行前校验强化

- Web/Native 执行前统一走 `snapshot + candidateId` 校验；
- fallback 执行增加风险标签与重试策略；
- 在工具返回结构中增加 `preCheck` 与 `postActionFeedback`。

### Phase C（2 周）：语义简化与任务化裁剪

- 以 `targetHint + action` 驱动候选裁剪；
- 引入页面区域优先级（首屏 > 焦点容器 > 其余区域）；
- 对高噪声文本候选做降权。

### Phase D（持续）：评估与回归

- 指标：点击命中率、误点击率、重试次数、每步 token 体积、平均执行时延；
- 建立 Web/Native 双域基准集，进行 nightly 回归。

---

## 7. 风险与应对

1. **候选过多导致 token 上涨**  
   - 通过阶段化裁剪 + TopK 输出 + 摘要优先。
2. **Web 页面动态变更快，ref 失效**  
   - 强制 snapshot 新鲜度校验 + 失效自动重采集。
3. **Native 页面动画/浮层干扰点击**  
   - 执行前检查可见性与遮挡，必要时延迟注入。
4. **双域字段不一致导致维护成本上升**  
   - 以统一 schema 为主，source-specific 字段放在 `extensions`。

---

## 8. 最终建议（可直接执行）

1. **先做统一 `locatorCandidates`，再做算法增强**：优先解决“输入统一”，避免后续策略分叉。  
2. **保留 observation-bound 强约束**：`snapshotId` 继续作为执行前硬校验。  
3. **把“简化 HTML/XML”升级为“任务语义裁剪”**：从“字段少一点”变成“目标更聚焦”。  
4. **把视觉反馈产品化**：执行后必须返回状态变化证据，而不只是 success/fail。  
5. **通过分阶段上线降低风险**：先兼容输出，再灰度切流，再默认切换。

> 结论：
> 你提出的两种设计本质上已经具备可融合基础。建议采用“**双轨采集、单轨定位、分域执行、统一反馈**”的架构，把 Web DOM 与 Native XML 都收敛到同一套候选定位协议，逐步提升自动化稳定性与可解释性。
