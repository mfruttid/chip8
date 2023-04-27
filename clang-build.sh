clang++ --std=c++20 \
-ggdb \
-fdiagnostics-show-template-tree \
-Wextra \
-Wall \
-Wpedantic \
-Wconversion \
-Wwrite-strings \
-Warray-bounds \
-Wcast-align \
-Wcast-qual \
-Wdouble-promotion \
-Wnull-dereference \
-Wimplicit-fallthrough \
-Wshift-negative-value \
-Wswitch-default \
-Wswitch-enum \
-Wuninitialized \
-Wfloat-equal \
-Wshadow \
-Wvla \
-Wno-unknown-pragmas \
./src/chip8.cpp \
./tests/test.cpp \
-o bin/chip8.bin

