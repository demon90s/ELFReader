CC=g++
CXXFLAGS=-g -Wall -std=c++11

TARGET=elfreader
SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SOURCES))

%.o: %.cpp
	@echo -e "[COMPILING] \c"
	$(CC) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@echo -e "[LINGKING] \c"
	$(CC) $(CXXFLAGS) $(OBJS) -o $(TARGET)

clean:
	@rm -fr *.o elfreader core.*

#############################################################
# 使用 gcc -MM *.cpp 创建当前目录下所有CPP文件的依赖关系，然后粘贴在下面
ELFPrinter.o: ELFPrinter.cpp ELFPrinter.h ELFReader.h formattedtable.hpp
ELFReader.o: ELFReader.cpp ELFReader.h
main.o: main.cpp ELFReader.h ELFPrinter.h