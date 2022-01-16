
TARGET:=client.out
INSTALL_DIR:=../../../install

CXX?=g++
CXXFLAGS:=-std=c++11 -Wall -DNDEBUG -O2 -I$(INSTALL_DIR)/include
LINKFLAGS:=-pthread -L$(INSTALL_DIR)/lib -lnet

SRCS:=$(wildcard *.cpp)

LIB:=$(INSTALL_DIR)/lib/libnet.a

$(TARGET): $(SRCS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKFLAGS)
