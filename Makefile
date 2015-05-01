ifeq ($(CXX),g++)
build: src/*.cpp
	$(CXX) -std=c++11 -o simple-vm_linux src/*.cpp
else
build: src/*.cpp
	$(CXX) -std=c++11 -o simple-vm_mac src/*.cpp
endif
	
test: src/*.cpp
	$(CXX) -std=c++11 -g -DTESTING -o simplevm-test src/*.cpp
