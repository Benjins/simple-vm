ifeq ($(CXX),g++)
build: src/*.cpp
	$(CXX) -std=c++11 -o simplevm_linux src/*.cpp
else
build: src/*.cpp
	$(CXX) -std=c++11 -o simplevm_mac src/*.cpp
endif
	
test: src/*.cpp
	$(CXX) -std=c++11 -DTESTING -o simplevm-test src/*.cpp
