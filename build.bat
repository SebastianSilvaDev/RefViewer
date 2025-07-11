@echo off
mkdir build
echo "%cd%\build"
cmake --build "%cd%\build" --config Debug --target All_BUILD -j 12