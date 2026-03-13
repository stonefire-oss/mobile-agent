# MNN Provider 发布包

## 文件说明

| 文件 | 说明 |
|------|------|
| `icraw_demo.exe` | **完整 Agent Demo** (支持远程API和本地MNN) |
| `mnn_interactive.exe` | MNN 交互式测试程序 |
| `mnn_test.exe` | MNN Provider 基础测试程序 |
| `mnn_tool_test.exe` | 工具调用测试程序 |
| `mnn_full_tool_test.exe` | 完整工具调用测试程序 |
| `MNN.dll` | MNN 推理引擎库 |
| `fmt.dll` | 格式化库 |
| `libcurl.dll` | HTTP 客户端库 |
| `spdlog.dll` | 日志库 |
| `sqlite3.dll` | SQLite 数据库库 |
| `zlib1.dll` | 压缩库 |

## 使用方法

### 1. icraw_demo.exe (推荐)

完整的Agent Demo，支持本地MNN模型和远程API两种模式。

**本地MNN模式：**
```bash
# 使用启动脚本
run_demo.bat "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"

# 或直接运行
icraw_demo.exe --mnn-model "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"
```

**远程API模式：**
```bash
icraw_demo.exe --api-key "your-api-key" --model "gpt-4o"
```

**完整参数：**
```
远程API选项:
  --api-key <key>      OpenAI API key (或设置 OPENAI_API_KEY 环境变量)
  --base-url <url>     API base URL (默认: https://api.openai.com/v1)
  --model <model>      模型名称 (默认: gpt-4o)

本地MNN选项:
  --mnn-model <path>   使用本地MNN模型
  --mnn-threads <n>    MNN推理线程数 (默认: 4)
  --mnn-backend <type> MNN后端类型: cpu, opencl (默认: cpu)

其他选项:
  --workspace <path>   工作目录
  --no-stream          禁用流式输出
  --help               显示帮助
```

### 2. mnn_interactive.exe

简化的交互式测试程序，用于测试MNN模型。

```bash
mnn_interactive.exe "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"
```

**内置命令：**
- `/help` - 显示帮助
- `/reset` - 重置对话
- `/stream` - 切换流式/非流式输出
- `/tokens <n>` - 设置最大token数
- `/temp <n>` - 设置temperature
- `/quit` - 退出程序

### 3. 测试程序

```bash
# 基础测试
mnn_test.exe "D:\AppData\modelscope\Qwen3.5-0.8B-MNN" "Hello"

# 工具调用测试
mnn_tool_test.exe "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"

# 完整工具测试
mnn_full_tool_test.exe "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"
```