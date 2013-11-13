CXX = clang++
CXXFLAGS = $(shell llvm-config --cxxflags) --std=c++11 -g
CLANGLIBS = -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lclangTooling
LLVMLIBS = $(shell llvm-config --ldflags --libs cppbackend) $(shell llvm-config --libs)
SQLLIBS = -lsqlite3
LIBS = $(CLANGLIBS) $(LLVMLIBS) $(SQLLIBS)

sources = main.cpp myastvisitor.cpp sql.cpp 
objects = $(sources:.cpp=.o)
depends = $(sources:.cpp=.d)

all: clang_navigate

include $(depends)

clang_navigate: $(objects)
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp $(headers)
	$(CXX) $(CXXFLAGS) $< -c

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MF"$@" -MG -MM -MP -MT"$(<:%.cpp=%.o)" "$<"

clean:
	rm -f clang_navigate $(objects) $(depends)
