@echo off
setlocal

set VCPKG_ROOT=%~dp0vcpkg

echo [1/3] Bootstrapping vcpkg...
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat" -disableMetrics
    if errorlevel 1 goto :error
)

echo [2/3] Configuring CMake (Release)...
cmake --preset release
if errorlevel 1 goto :error

echo [3/3] Building...
cmake --build build/release --config Release --target FreeDDecoder
if errorlevel 1 goto :error

echo.
echo ========================================
echo   Build successful!
echo   %~dp0build\release\Release\FreeDDecoder.exe
echo ========================================
goto :end

:error
echo.
echo Build failed.
exit /b 1

:end
endlocal
