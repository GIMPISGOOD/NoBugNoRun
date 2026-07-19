@echo off
chcp 65001 >nul
setlocal

REM ============================================================
REM  BugNote v0.0.2-pre-exploit - MSVC Build Script
REM
REM  WARNING: This build intentionally disables all security
REM  hardening. This is not a bug. This is a feature.
REM
REM  Usage:
REM    build.bat          - Build bugnote.exe
REM    build.bat docs     - Generate "API documentation" (compiler warnings)
REM    build.bat clean    - Remove build artifacts
REM    build.bat run FILE - Build and run a .bug file
REM ============================================================

set SRC=src\main.c src\lexer.c src\memory.c src\engine.c src\cve.c src\audit.c
set OUT=bugnote.exe
REM /W4   = all warnings (the "API documentation")
REM /Od   = no optimization (keep vulnerabilities deterministic)
REM /Zi   = debug info (for windbg reverse engineering)
REM /GS-  = disable stack buffer security check (on purpose)
REM /sdl- = disable additional security checks (on purpose)
set CFLAGS=/nologo /W4 /Od /Zi /GS- /sdl-

if "%1"=="docs" goto :docs
if "%1"=="clean" goto :clean
if "%1"=="run" goto :run
goto :build

:build
echo ============================================================
echo   BugNote v0.0.2-pre-exploit - Building...
echo   "0 features, 14 vulnerabilities."
echo ============================================================
echo.
echo   Compiler flags: %CFLAGS%
echo   Security hardening: DISABLED (intentionally)
echo.

cl %CFLAGS% /Fe:%OUT% %SRC%

if %errorlevel%==0 (
    echo.
    echo   Build successful.
    echo   0 features compiled. 14 vulnerabilities included.
    echo.
    echo   Usage: %OUT% examples\hello.bug
    echo   Usage: %OUT% --safe examples\hello.bug  ^(won't help^)
) else (
    echo.
    echo   Build failed. Even the compiler can't handle this code.
    echo   This is expected. The warnings ARE the documentation.
)
goto :eof

:docs
echo ============================================================
echo   BugNote v0.0.2-pre-exploit - Generating API Documentation
echo   "Compiler warnings = official API documentation"
echo ============================================================
echo.

if not exist docs mkdir docs

cl %CFLAGS% /c %SRC% 2> docs\warnings.txt
del *.obj 2>nul

for %%A in (docs\warnings.txt) do (
    if %%~zA==0 (
        echo   No warnings. This is suspicious. The bugs are hiding.
    ) else (
        echo   API documentation generated: docs\warnings.txt
        echo   Warning count:
        find /c /v "" docs\warnings.txt
    )
)
echo.
echo   "Each warning is a feature. Each feature is a vulnerability."
goto :eof

:clean
echo Cleaning build artifacts...
del %OUT% 2>nul
del *.obj 2>nul
del *.pdb 2>nul
del *.ilk 2>nul
del *.exp 2>nul
del *.lib 2>nul
echo Cleaned. The vulnerabilities remain in the source code.
goto :eof

:run
if "%2"=="" (
    echo Usage: build.bat run ^<file.bug^>
    goto :eof
)
if not exist %OUT% (
    echo Building first...
    call :build
    if not exist %OUT% goto :eof
)
echo.
echo --- Running: %2 ---
echo.
%OUT% %2
echo.
echo --- Exit code: %errorlevel% ---
goto :eof