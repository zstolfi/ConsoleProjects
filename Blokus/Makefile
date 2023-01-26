LIBRARY_PATHS = -L C:/msys64/mingw64/lib

# evil dependencies
DEPENDENCIES = -static-libgcc -static-libstdc++

COMPILER_FLAGS = -std=c++23 -Wall -O3

LINKER_FLAGS = -static

#Compile!
all :
	g++ main.cc -o Blokus $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS)

test :
	g++ parseTest.cc -o ParseTest $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS)