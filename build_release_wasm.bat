@echo off
setlocal enabledelayedexpansion
for /r ".\sources" %%f in (*.cpp) do (
    call :getFullPath "%%f"
    set "cpp_files=!cpp_files! !fullpath!"
)

for /r ".\sources" %%f in (template.html) do (
    call :getFullPath "%%f"
    set "template_html=!fullpath!"
)

pushd .\bin
echo %cpp_files%
em++ %cpp_files% -fno-rtti -fno-exceptions -s WASM=1 -sMAX_WEBGL_VERSION=2 -s ASSERTIONS=0 -s GL_ASSERTIONS -lGL -sFILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -O2 --shell-file %template_html% -o index.html
popd

exit /b

:getFullPath
set "fullpath=%~f1"
exit /b