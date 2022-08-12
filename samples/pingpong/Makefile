
INSTALL_DIR:=../../install

CXX?=g++
CXXFLAGS:=-std=c++11 -Wall -DNDEBUG -O2 -I$(INSTALL_DIR)/include
LINKFLAGS:=-pthread -L$(INSTALL_DIR)/lib -lnet

SRCS:=$(wildcard *.cpp)
TARGETS:=$(SRCS:.cpp=.out)

LIB:=$(INSTALL_DIR)/lib/libnet.a


.PHONY: all clean
all: $(TARGETS)

%.out : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKFLAGS)

clean:
	rm $(TARGETS)