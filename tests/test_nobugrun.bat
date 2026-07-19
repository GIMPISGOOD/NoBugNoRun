@echo off
chcp 65001 >nul
echo === Test: NoBugNoRun (nobug.bug) ===
echo Expected: exit code 1, NoBugNoRun error
echo.
bugnote.exe examples\nobug.bug > test_out.txt 2> test_err.txt
set CODE=%errorlevel%
type test_err.txt
echo.
echo Exit code: %CODE%
if %CODE%==1 (echo [PASS] NoBugNoRun triggered.) else (echo [FAIL] Expected 1, got %CODE%.)
del test_out.txt test_err.txt 2>nul