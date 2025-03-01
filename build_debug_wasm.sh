#!/bin/bash

cpp_files=$(find ./sources -name "*.cpp" -exec realpath {} \;)
c_files=$(find ./sources -name "*.c" -exec realpath {} \;)
template_html=$(find ./sources -name "template.html" -exec realpath {} \;)
current_path=$(pwd)
pushd ./bin
EMCC_DEBUG=1 em++ $cpp_files $c_files -fno-rtti -fno-exceptions -s WASM=1 -sMAX_WEBGL_VERSION=2  -s GL_ASSERTIONS   -lGL -s ASSERTIONS=2 -sFILESYSTEM=1 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=2 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0  -g -DDEBUG -gsource-map  -gseparate-dwarf  -gsplit-dwarf -fdebug-compilation-dir="$current_path"  --shell-file $template_html  -o index.html
popd

