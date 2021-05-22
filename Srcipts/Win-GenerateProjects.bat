@echo off
pushd ..\
call 3dparty\bin\premake\premake5.exe vs2019
popd
Pause