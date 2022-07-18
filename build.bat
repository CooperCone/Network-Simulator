@echo off
setlocal

call vcvars64 >NUL

set outputFile=%RootPath%/src/*.c

set includeDir=-I%RootPath%/include
set srcFiles=%outputFile%
set libs=user32.lib
set compilerWarningFlag=-std:c11 -Wv:18 -WL -WX -W4 -wd4201 -wd4200^
 -wd4204 -wd4100 -wd4102 -wd4221 -wd4189 -wd4101 -D_CRT_SECURE_NO_WARNINGS
set linkerWarningFlag=-IGNORE:4099,4098

if not exist ".\build" mkdir build
pushd build

if "%1"=="dbg" (
    set debugFlag=-Z7 -MTd -D_DEBUG -DEBUG
    echo debug
) else (
    set debugFlag=
    echo release
)

cl -nologo %compilerWarningFlag% /TC %srcFiles% %includeDir% %debugFlag%^
 /link -out:simulator.exe %linkerWarningFlag% %libs% | prettyCompilerOutput.exe

popd build
