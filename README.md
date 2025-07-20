# C Compiler

This is a compiler for a subset of C.

This project will follow this guide:
https://norasandler.com/2017/11/29/Write-a-Compiler.html

## Building
There are no external dependencies.

To build the executable, run
```
make
```
To view the entire compilation process, run
```
make debug
```
To remove all binaries:
```
make clean
```

## Testing
To test this compiler, you will need to clone the test suite:
```
git clone https://github.com/nlsandler/write_a_c_compiler 
cd write_a_c_compiler
./test_compiler.sh path/to/compiler `seq 1 <last-stage-to-test>`
```
