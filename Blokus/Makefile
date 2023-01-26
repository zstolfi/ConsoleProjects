LIBRARY_PATHS = -L C:/msys64/mingw64/lib

# evil dependencies
DEPENDENCIES = -static-libgcc -static-libstdc++

COMPILER_FLAGS = -std=c++23 -Wall -O3

LINKER_FLAGS = -static

#Compile!
all :
	g++ main.cc $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o Blokus

test :
	g++ parseTest.cc $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o ParseTest