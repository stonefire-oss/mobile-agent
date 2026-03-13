@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo icraw CLI Agent Demo (MNN版本)
echo ========================================
echo.

if "%~1"=="" (
    echo 用法: run_demo.bat ^<MNN模型路径^> [工作目录]
    echo.
    echo 示例:
    echo   run_demo.bat "D:\AppData\modelscope\Qwen3.5-0.8B-MNN"
    echo   run_demo.bat "D:\AppData\modelscope\Qwen3.5-0.8B-MNN" "D:\workspace"
    echo.
    echo 命令行参数:
    echo   --mnn-model ^<path^>     使用本地MNN模型
    echo   --mnn-threads ^<n^>      MNN推理线程数 (默认: 4)
    echo   --mnn-backend ^<type^>   MNN后端类型: cpu, opencl (默认: cpu)
    echo   --workspace ^<path^>     工作目录
    echo   --no-stream             禁用流式输出
    echo   --help                  显示帮助
    echo.
    pause
    exit /b 1
)

set MNN_MODEL=%~1
set WORKSPACE=%~2
if "%WORKSPACE%"=="" set WORKSPACE=%~dp0workspace

echo 模型: %MNN_MODEL%
echo 工作目录: %WORKSPACE%
echo.

icraw_demo.exe --mnn-model "%MNN_MODEL%" --workspace "%WORKSPACE%"

pause