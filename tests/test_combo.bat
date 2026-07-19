@echo off
chcp 65001 >nul
echo === Test: Combo (combo.bug) ===
echo Expected: COMBO x3 message in stderr
echo.
bugnote.exe examples\combo.bug > test_out.txt 2> test_err.txt
set CODE=%errorlevel%
type test_err.txt
echo.
echo Exit code: %CODE%
findstr "COMBO" test_err.txt >nul 2>nul
if %errorlevel%==0 (echo [PASS] Combo detected.) else (echo [FAIL] No combo found.)
del test_out.txt test_err.txt 2>nul