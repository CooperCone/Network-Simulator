@echo off
setlocal EnableDelayedExpansion

REM First compile all test files
call vcvars64 >NUL

set baseIncludeDir=-I%RootPath%/include -I%RootPath%/src
set libs=user32.lib
set compilerWarningFlag=-std:c11 -Wv:18 -WL -WX -W4 -wd4201^
 -wd4200 -wd4204 -wd4100 -wd4102 -wd4221 -wd4189 -wd4101 -wd4477^
 -wd4127 -D_CRT_SECURE_NO_WARNINGS
set linkerWarningFlag=-IGNORE:4099,4098
set optFlag=-O2

set testDirs=.

(for %%d in (%testDirs%) do (
    cd %%d

    set includeDir=-I!cd!/include -I!cd!/src

    pushd test

    if not exist ".\bin" mkdir bin

    REM delete all bin files
    pushd bin
    del /Q *.*
    popd bin

    echo Compiling Tests in %%d...

    for %%i in (*) do (
        cl -nologo %compilerWarningFlag% %%i !includeDir!^
 %baseIncludeDir% %optFlag% -Fo"./bin/%%~ni.obj"^
 /link -out:bin/%%~ni.exe %linkerWarningFlag% %libs%^
 | prettyCompilerOutput.exe
    )

    echo ...Done Compiling Tests
    echo.

    popd test
    cd %RootPath%
))

REM check if environment exists
if not exist tools\unitTest\env\ (
    echo Python environment doesn't exist, so setting it up...
    call setupPythonEnv
    echo ...Done
    echo.
)

REM setup environment
call tools\unitTest\env\Scripts\activate.bat

REM then run all tests
py tools\unitTest\unitTest.py %testDirs%
