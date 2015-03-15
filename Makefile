build: src/*.cpp
	$(CXX) -std=c++11 -o simplevm src/*.cpp
	
test: src/*.cpp
	$(CXX) -std=c++11 -DTESTING -o simplevm-test src/*.cpp