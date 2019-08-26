@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -WX -W4 -wd4201 -wd4189 -wd4100 -wd4505 -FC -Z7 -DGG_INTERNAL -I"../../lib/SDL2-2.0.10/include" 
set CommonLinkerFlags=-opt:ref -incremental:no user32.lib shell32.lib advapi32.lib gdi32.lib ole32.lib oleaut32.lib setupapi.lib winmm.lib imm32.lib version.lib SDL2main.lib SDL2.lib -libpath:"../../lib/SDL2-2.0.10/lib/x64"

IF NOT EXIST build mkdir build
pushd build

IF NOT EXIST debug mkdir debug
pushd debug

REM Debug build
cl -Od %CommonCompilerFlags% ../../src/main.cpp /link -subsystem:console %CommonLinkerFlags%

popd

popd

