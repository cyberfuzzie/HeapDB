
CXX=clang++
CXXFLAGS=-std=c++11 `llvm-config --cxxflags`
LINK.o=$(LINK.cc)
LDFLAGS=`llvm-config --ldflags --libs jit interpreter nativecodegen`

ALL: calctree

calctree: calctree.o calcnode.o

calctree.o: calctree.cpp
calcnode.o: calcnode.cpp
