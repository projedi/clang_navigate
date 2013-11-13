CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags) --std=c++11 -g
CLANGLIBS = -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lclangTooling
LLVMLIBS=$(shell llvm-config --ldflags --libs cppbackend) $(shell llvm-config --libs)
SQLLIBS= -lsqlite3
LIBS=$(CLANGLIBS) $(LLVMLIBS) $(SQLLIBS)

clang_navigate: main.o sql.o
	$(CXX) main.o sql.o -o clang_navigate $(LIBS)

main.o: main.cpp sql.h
	$(CXX) $(CXXFLAGS) main.cpp -c

sql.o: sql.cpp sql.h
	$(CXX) $(CXXFLAGS) sql.cpp -c

clean:
	rm -f clang_navigate *.o
