@echo off
setlocal

call vcvars64 >NUL

set compilerWarningFlag=-O2 -WL -WX -W4 -wd4201 -wd4204 -wd4100^
 -wd4102 -wd4221 -wd4189 -wd4101 -D_CRT_SECURE_NO_WARNINGS

if not exist ".\build" mkdir build

pushd build

cl -nologo %compilerWarningFlag% ../src/main.c /link^
 -out:prettyCompilerOutput.exe

popd build
