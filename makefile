CXX = g++
CXXFLAGS = -std=c++17 -O2

# Opredelyeam OS
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
else
    DETECTED_OS := $(shell uname -s)
endif


ifeq ($(DETECTED_OS),Windows)
    LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXE_EXT = .exe
else ifeq ($(DETECTED_OS),Darwin)
    LDFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXE_EXT =
else
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXE_EXT =
endif

%$(EXE_EXT): %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
ifeq ($(DETECTED_OS),Windows)
	del /Q *.exe 2>nul
else
	rm -f $(basename $(wildcard *.cpp))
endif

.PHONY: clean