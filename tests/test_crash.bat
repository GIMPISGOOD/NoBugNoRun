@echo off
chcp 65001 >nul
echo === Test: Crash (FIXME with 0xDEAD type) ===
echo Expected: exit code 139, satirical crash message
echo.
echo TODO> temp_crash.bug
echo FIXME>> temp_crash.bug
bugnote.exe temp_crash.bug > test_out.txt 2> test_err.txt
set CODE=%errorlevel%
type test_err.txt
echo.
echo Exit code: %CODE%
if %CODE%==139 (echo [PASS] Crashed as expected.) else (echo [FAIL] Expected 139, got %CODE%.)
del temp_crash.bug test_out.txt test_err.txt 2>nul