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
set EMCC_DEBUG=1
pushd .\bin
echo %cpp_files%
em++ $cpp_files -fno-rtti -fno-exceptions -s WASM=1 -sMAX_WEBGL_VERSION=2  -s GL_ASSERTIONS   -lGL -s ASSERTIONS=2 -sFILESYSTEM=1 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=2 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -g3  -gsource-map  -gseparate-dwarf  -gsplit-dwarf --shell-file $template_html -o index.html
popd
set EMCC_DEBUG=0
exit /b

:getFullPath
set "fullpath=%~f1"
exit /b