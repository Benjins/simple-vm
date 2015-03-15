CXX=gcc

hellomake: src/*.cpp
     $(CC) -o simplevm src/*.cpp