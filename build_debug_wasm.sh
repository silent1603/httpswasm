#!/bin/bash

cpp_files=$(find ./sources -name "*.cpp" -exec realpath {} \;)
template_html=$(find ./sources -name "template.html" -exec realpath {} \;)
pushd ./bin
em++ $cpp_files -fno-rtti -fno-exceptions -s WASM=1 -sMAX_WEBGL_VERSION=2  -s GL_ASSERTIONS  -lGL -s ASSERTIONS=0 -sFILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -g3  --shell-file $template_html -o index.html
popd

