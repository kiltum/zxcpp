# Makefile for fuse_test

# Default target
all: run_test

# Compile and run the test
run_test: fuse_test
	cd .. && ./tests/fuse_test --failfast
	rm -f fuse_test

# Compile the test
fuse_test: fuse_test.cpp ../src/z80.cpp ../src/memory.cpp ../src/port.cpp ../src/z80_opcodes.cpp ../src/z80_dd_opcodes.cpp ../src/z80_fd_opcodes.cpp ../src/z80_cb_opcodes.cpp ../src/z80_ed_opcodes.cpp ../src/z80_fdcb_opcodes.cpp
	g++ -std=c++11 -o fuse_test fuse_test.cpp ../src/z80.cpp ../src/memory.cpp ../src/port.cpp ../src/z80_opcodes.cpp ../src/z80_dd_opcodes.cpp ../src/z80_fd_opcodes.cpp ../src/z80_cb_opcodes.cpp ../src/z80_ed_opcodes.cpp ../src/z80_fdcb_opcodes.cpp -I../include

# Clean up any executables
clean:
	rm -f fuse_test

.PHONY: all run_test clean
