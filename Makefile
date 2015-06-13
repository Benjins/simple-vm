compiler: src/*.cpp
	$(CXX) -std=c++11 -DCOMPILER -o svm src/*.cpp -ldl

machine: src/*.cpp
	$(CXX) -std=c++11 -o svb src/*.cpp -ldl

docs: header/*.h src/*.cpp *.dox
	doxygen .doxy-config

test: src/*.cpp
	$(CXX) -std=c++11 -DTESTING -g -fprofile-arcs -ftest-coverage -o simplevm-test src/*.cpp -ldl
