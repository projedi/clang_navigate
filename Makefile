CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
CLANGLIBS = -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic
LLVMLIBS=$(shell llvm-config --ldflags --libs cppabackend) $(shell llvm-config --libs)
LIBS=$(CLANGLIBS) $(LLVMLIBS)

clang_navigate: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o clang_navigate $(LIBS)

clean:
	rm -f clang_navigate