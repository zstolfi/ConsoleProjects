INCLUDE_PATHS = 

LIBRARY_PATHS = -LC:/msys64/mingw64/lib

# evil dependencies
DEPENDENCIES = -static-libgcc -static-libstdc++

COMPILER_FLAGS = -std=c++20 -w -O4

LINKER_FLAGS = -static

#Compile!
all : $(OBJS)
	g++ *.cc $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o 2048