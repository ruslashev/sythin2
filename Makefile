BZIP_SRCS = bzip2-1.0.6/bzlib.c bzip2-1.0.6/crctable.c bzip2-1.0.6/huffman.c \
			bzip2-1.0.6/randtable.c bzip2-1.0.6/compress.c \
			bzip2-1.0.6/decompress.c bzip2-1.0.6/blocksort.c
BZIP_OBJS = $(patsubst bzip2-1.0.6/%.c,.objs/%.o, $(BZIP_SRCS))

IMGUI_OBJS = .objs/imgui.o .objs/imgui_demo.o .objs/imgui_draw.o

SRCDIR = src
OBJS = $(patsubst $(SRCDIR)/%.cc,.objs/%.o, \
	   $(shell find $(SRCDIR) -type f -name '*.cc' ))

CXX = g++
CXXFLAGS = -Wall -Wextra -Wno-deprecated-declarations -Werror -g -std=c++0x
IMGUI_CXXFLAGS = -g -std=c++0x
CC = gcc
CCFLAGS = -w -fpermissive
LDFLAGS = -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system -lGLEW -lGL -llua
EXECNAME = sythin2

all: objdir $(EXECNAME)
	./$(EXECNAME)

.objs/%.o: $(SRCDIR)/%.cc
	@echo "Compiling $<"
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

.objs/%.o: bzip2-1.0.6/%.c
	@echo "Compiling $<"
	@$(CC) -c -o $@ $< $(CCFLAGS)

.objs/%.o: imgui/%.cpp
	@echo "Compiling $<"
	@$(CXX) -c -o $@ $< $(IMGUI_CXXFLAGS)

$(EXECNAME): $(OBJS) $(BZIP_OBJS) $(IMGUI_OBJS)
	@echo "Linking to $@"
	@$(CXX) -o $@ $^ $(LDFLAGS)

valgrind: objdir $(EXECNAME)
	valgrind --leak-check=full ./$(EXECNAME)

callgrind: objdir $(EXECNAME)
	@valgrind --tool=callgrind ./$(EXECNAME)
	@kcachegrind callgrind.out.$!

objdir:
	@mkdir -p .objs

get-deps:
	@mkdir -p imgui
	touch imgui/imconfig.h
	wget https://raw.githubusercontent.com/ocornut/imgui/master/imgui.cpp -O imgui/imgui.cpp
	wget https://raw.githubusercontent.com/ocornut/imgui/master/imgui.h -O imgui/imgui.h
	wget https://raw.githubusercontent.com/ocornut/imgui/master/imgui_internal.h -O imgui/imgui_internal.h
	wget https://raw.githubusercontent.com/ocornut/imgui/master/imgui_demo.cpp -O imgui/imgui_demo.cpp
	wget https://raw.githubusercontent.com/ocornut/imgui/master/imgui_draw.cpp -O imgui/imgui_draw.cpp
	wget https://raw.githubusercontent.com/ocornut/imgui/master/stb_rect_pack.h -O imgui/stb_rect_pack.h
	wget https://raw.githubusercontent.com/ocornut/imgui/master/stb_textedit.h -O imgui/stb_textedit.h
	wget https://raw.githubusercontent.com/ocornut/imgui/master/stb_truetype.h -O imgui/stb_truetype.h
	wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
	tar xzf bzip2-1.0.6.tar.gz
	rm bzip2-1.0.6.tar.gz

clean:
	rm -f $(EXECNAME)
	rm -f $(OBJS)

ftcc:
	g++ file_to_c_source.cc $(BZIP_SRCS) -std=c++0x -o ftcc $(CCFLAGS)

