INCLUDE_PATHS = 

LIBRARY_PATHS = -LC:/msys64/mingw64/lib

# evil dependencies
DEPENDENCIES = -static-libgcc -static-libstdc++

COMPILER_FLAGS = -std=c++20 -Wall -O3# -Wl,-subsystem,windows

LINKER_FLAGS = -static

#Compile!
all : $(OBJS)
	g++ main.cc $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o Mandelbrot