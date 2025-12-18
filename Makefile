#OBJS specifies which files to compile as part of the project
OBJS = game.cpp window.cpp logic.cpp

#CC specifies thich compiler in usage
CC = g++

#CXXFLAGS specifies used version of C++
CXXFLAGS = -std=c++17

#COMPILER_FLAGS specifies the additional compilation options
COMPILER_FLAGS = -w #suppresses all warnings

#LINKER_FLAGS specifies the libraries we're linking agains
LINKER_FLAGS = -lSDL2

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = game

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
