@echo off
chcp 65001 >nul
echo === Test: Survive (hello.bug) ===
echo Expected: exit code 0, output contains "Hi"
echo.
bugnote.exe examples\hello.bug > test_out.txt 2> test_err.txt
set CODE=%errorlevel%
type test_out.txt
echo.
echo Exit code: %CODE%
if %CODE%==0 (echo [PASS] Program survived.) else (echo [FAIL] Program crashed.)
del test_out.txt test_err.txt 2>nul