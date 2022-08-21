#!/bin/bash

srcDir=$RootPath/src
srcFiles="$srcDir/*.c $srcDir/devices/*.c $srcDir/layers/*.c $srcDir/util/*.c $srcDir/platform/linux.c"

includeDir=-I$RootPath/include
compilerWarningFlag=-Wno-builtin-declaration-mismatch

if [ ! -d "build" ]
then
    mkdir build
fi

pushd build

if [ $1=="dbg" ]
then
    #debugFlag=-Z7 -MTd -D_DEBUG -DEBUG
    debugFlag=-g
    echo debug
else
    debugFlag=
    echo release
fi

gcc $compilerWarningFlag $srcFiles $includeDir $debugFlag -o simulator

popd
