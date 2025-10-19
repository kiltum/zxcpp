CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude
SRCDIR = src
OBJDIR = obj
SOURCES = $(SRCDIR)/emulator.cpp \
          $(SRCDIR)/memory.cpp \
          $(SRCDIR)/port.cpp \
          $(SRCDIR)/z80.cpp \
          $(SRCDIR)/z80_opcodes.cpp \
          $(SRCDIR)/z80_cb_opcodes.cpp \
          $(SRCDIR)/z80_ed_opcodes.cpp \
          $(SRCDIR)/z80_dd_opcodes.cpp \
          $(SRCDIR)/z80_fd_opcodes.cpp \
          $(SRCDIR)/z80_ddcb_opcodes.cpp \
          $(SRCDIR)/z80_fdcb_opcodes.cpp \
          $(SRCDIR)/ula.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = emulator

# SDL3 flags
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null || echo "-I/opt/homebrew/opt/sdl3/include")
SDL_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null || echo "-L/opt/homebrew/opt/sdl3/lib -Wl,-rpath,/opt/homebrew/opt/sdl3/lib -lSDL3")

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(SDL_LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean
