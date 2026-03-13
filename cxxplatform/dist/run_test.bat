@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo MNN Provider 测试程序
echo ========================================
echo.

if "%~1"=="" (
    echo 用法: run_test.bat ^<模型路径^> [提示词]
    echo.
    echo 示例:
    echo   run_test.bat "D:\AppData\modelscope\Qwen3.5-0.8B-MNN" "Hello"
    echo.
    pause
    exit /b 1
)

set MODEL_PATH=%~1
set PROMPT=%~2
if "%PROMPT%"=="" set PROMPT=Hello, who are you?

echo 模型路径: %MODEL_PATH%
echo 提示词: %PROMPT%
echo.

echo 选择测试程序:
echo   1. mnn_test.exe (基础测试)
echo   2. mnn_tool_test.exe (工具调用测试)
echo   3. mnn_full_tool_test.exe (完整工具测试)
echo.

set /p CHOICE="请输入选择 (1-3): "

if "%CHOICE%"=="1" (
    echo 运行 mnn_test.exe...
    mnn_test.exe "%MODEL_PATH%" "%PROMPT%"
) else if "%CHOICE%"=="2" (
    echo 运行 mnn_tool_test.exe...
    mnn_tool_test.exe "%MODEL_PATH%"
) else if "%CHOICE%"=="3" (
    echo 运行 mnn_full_tool_test.exe...
    mnn_full_tool_test.exe "%MODEL_PATH%"
) else (
    echo 无效选择
    pause
    exit /b 1
)

echo.
pause