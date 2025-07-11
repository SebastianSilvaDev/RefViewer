
mkdir %cd%/build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S %cd% -B %cd%/build