@echo off

mkdir tools
pushd tools
git clone https://github.com/emscripten-core/emsdk.git
pushd emsdk
git pull
emsdk.bat install latest
popd
popd