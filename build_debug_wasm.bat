@echo off
setlocal enabledelayedexpansion
for /r ".\sources" %%f in (*.cpp) do (
    set "cpp_files=!cpp_files! %%f"
)

for /r ".\sources" %%f in (*.c) do (
    set "c_files=!c_files! %%f"
)

for /r ".\sources" %%f in (template.html) do (
    echo Found: %%f
    set "template_html=%%f"
    goto :found
)

:found

echo Template file path: %template_html%
set current_path=%~dp0
set EMCC_DEBUG=1
pushd .\bin
em++ %cpp_files% %c_files% -fno-rtti -fno-exceptions -s WASM=1 -sMAX_WEBGL_VERSION=2  -s GL_ASSERTIONS  -s ASYNCIFY=1 -s ASYNCIFY_STACK_SIZE=16384 -lGL -s ASSERTIONS=2 -sFILESYSTEM=1 -s SAFE_HEAP=1 -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s STACK_OVERFLOW_CHECK=2 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -g -DDEBUG -gsource-map  -gseparate-dwarf  -gsplit-dwarf -fdebug-compilation-dir='%current_path%' --shell-file %template_html% -o index.html
popd
set EMCC_DEBUG=0
exit /b
