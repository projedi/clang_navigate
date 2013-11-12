CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags) --std=c++11
CLANGLIBS = -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic
LLVMLIBS=$(shell llvm-config --ldflags --libs cppbackend) $(shell llvm-config --libs)
SQLLIBS= -lsqlite3
LIBS=$(CLANGLIBS) $(LLVMLIBS) $(SQLLIBS)

clang_navigate: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o clang_navigate $(LIBS)

clean:
	rm -f clang_navigate
