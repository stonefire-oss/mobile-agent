@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo MNN Provider Performance Test
echo ========================================
echo.
echo Model: Qwen3.5-0.8B-MNN
echo Date: %date% %time%
echo.

set MODEL_PATH=D:\AppData\modelscope\Qwen3.5-0.8B-MNN
set MNN_DEMO=E:\work\AI\MNN\build\Release\llm_demo.exe

echo --- Test 1: Simple Question ---
echo Question: What is 2+2?
echo.
set START=%time%
echo What is 2+2? | %MNN_DEMO% "%MODEL_PATH%\config.json" 2>&1 | findstr /V "config path device supports cost time Prepare tuning"
set END=%time%
echo.
echo Start: %START%
echo End: %END%
echo.

echo --- Test 2: Tool Call ---
echo Question: List files in workspace
echo.
set START2=%time%
echo List files in workspace | %MNN_DEMO% "%MODEL_PATH%\config.json" 2>&1 | findstr /V "config path device supports cost time Prepare tuning"
set END2=%time%
echo.
echo Start: %START2%
echo End: %END2%
echo.

echo --- Test 3: Another Tool Call ---
echo Question: Show a toast message
echo.
set START3=%time%
echo Show a toast message saying Hello | %MNN_DEMO% "%MODEL_PATH%\config.json" 2>&1 | findstr /V "config path device supports cost time Prepare tuning"
set END3=%time%
echo.
echo Start: %START3%
echo End: %END3%
echo.

echo ========================================
echo Test Complete
echo ========================================
pause