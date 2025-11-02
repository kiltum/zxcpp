CXX = g++
LIBZIP_CFLAGS := $(shell pkg-config --cflags libzip 2>/dev/null)
LIBZIP_LIBS := $(shell pkg-config --libs libzip 2>/dev/null)
# LLVM_PROFILE_FILE="emu.profraw" ./emulator tests/testdata/Exolon.tzx.zip 
# llvm-profdata merge -output=emu.profdata emu.profraw
# llvm-cov show ./emulator -instr-profile=emu.profdata -format=html > emu.html
CXXFLAGS = -std=c++20 -O3 -march=native -mtune=native -g -fprofile-instr-generate -fcoverage-mapping -fsanitize=address -Wall -Wextra -Iinclude -Ilib/imgui -Ilib/imgui/backends -Ilib/imguifiledialog -Ilib/vgm_decoder/include $(LIBZIP_CFLAGS)
SRCDIR = src
OBJDIR = obj
IMGUI_DIR = lib/imgui
IMGUI_SOURCES = $(IMGUI_DIR)/imgui.cpp \
                $(IMGUI_DIR)/imgui_demo.cpp \
                $(IMGUI_DIR)/imgui_draw.cpp \
                $(IMGUI_DIR)/imgui_tables.cpp \
                $(IMGUI_DIR)/imgui_widgets.cpp \
                $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
                $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp

IMGUIDIALOG_DIR = lib/imguifiledialog
IMGUIDIALOG_SOURCES = $(IMGUIDIALOG_DIR)/ImGuiFileDialog.cpp

# VGM Decoder AY-3-8910 sources
VGM_DECODER_DIR = lib/vgm_decoder
VGM_DECODER_SOURCES = $(VGM_DECODER_DIR)/src/chips/ay-3-8910.cpp

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
          $(SRCDIR)/ula.cpp \
          $(SRCDIR)/kempston.cpp \
          $(SRCDIR)/sound.cpp \
          $(SRCDIR)/tape.cpp \
          $(SRCDIR)/ay8912.cpp \
          $(IMGUI_SOURCES) \
          $(IMGUIDIALOG_SOURCES) \
          $(VGM_DECODER_SOURCES)

OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
OBJECTS := $(OBJECTS:$(IMGUI_DIR)/%.cpp=$(OBJDIR)/%.o)
OBJECTS := $(OBJECTS:$(IMGUI_DIR)/backends/%.cpp=$(OBJDIR)/%.o)
OBJECTS := $(OBJECTS:$(IMGUIDIALOG_DIR)/%.cpp=$(OBJDIR)/%.o)
OBJECTS := $(OBJECTS:$(VGM_DECODER_DIR)/%.cpp=$(OBJDIR)/%.o)

TARGET = emulator

# SDL3 flags
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null || echo "-I/opt/homebrew/opt/sdl3/include")
SDL_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null || echo "-L/opt/homebrew/opt/sdl3/lib -Wl,-rpath,/opt/homebrew/opt/sdl3/lib -lSDL3")

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -g -fprofile-instr-generate -fcoverage-mapping -fsanitize=address -o $@ $(SDL_LIBS) $(LIBZIP_LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(IMGUI_DIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(IMGUI_DIR)/backends/%.cpp | $(OBJDIR)/backends
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(IMGUIDIALOG_DIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(VGM_DECODER_DIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(OBJDIR)/backends:
	mkdir -p $(OBJDIR)/backends

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/backends

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean
